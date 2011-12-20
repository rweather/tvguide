/*
 * Copyright (C) 2011  Southern Storm Software, Pty Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tvchannellist.h"
#include "tvprogramme.h"
#include <QtCore/qset.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtNetwork/qnetworkdiskcache.h>

//#define DEBUG_NETWORK 1

TvChannelList::TvChannelList(QObject *parent)
    : QObject(parent)
    , m_startUrlRefresh(24)
    , m_hasDataFor(false)
    , m_throttled(false)
    , m_busy(false)
    , m_largeIcons(true)
    , m_haveChannelNumbers(false)
    , m_progress(1.0f)
    , m_requestsToDo(0)
    , m_requestsDone(0)
    , m_reply(0)
    , m_bookmarkList(this)
{
    QString cacheDir = QDir::homePath() +
                       QLatin1String("/.qtvguide/cache");
    QNetworkDiskCache *cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(cacheDir);
    m_nam.setCache(cache);

    connect(&m_nam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&m_nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

    m_throttleTimer = new QTimer(this);
    m_throttleTimer->setSingleShot(true);
    connect(m_throttleTimer, SIGNAL(timeout()),
            this, SLOT(throttleTimeout()));

    connect(&m_bookmarkList, SIGNAL(bookmarksChanged()),
            this, SLOT(reloadBookmarks()));
    connect(&m_bookmarkList, SIGNAL(bookmarksChanged()),
            this, SIGNAL(bookmarksChanged()));
    connect(&m_bookmarkList, SIGNAL(bookmarksChanged()),
            this, SLOT(saveBookmarks()));

    connect(&m_bookmarkList, SIGNAL(ticksChanged()),
            this, SLOT(saveTicks()));

    reloadService();
}

TvChannelList::~TvChannelList()
{
    qDeleteAll(m_channels);
    qDeleteAll(m_groups);
}

TvChannel *TvChannelList::channel(const QString &id) const
{
    return m_channels.value(id, 0);
}

void TvChannelList::setGroups(const QList<TvChannelGroup *> &groups, bool notify)
{
    qDeleteAll(m_groups);
    m_groups = groups;
    saveGroups();
    if (notify)
        emit groupsChanged();
}

static bool sortActiveChannels(TvChannel *c1, TvChannel *c2)
{
    return c1->compare(c2) < 0;
}

void TvChannelList::load(QXmlStreamReader *reader, const QUrl &url)
{
    QString channelId;
    TvChannel *channel;
    TvProgramme *programme;
    bool newChannels = false;
    QSet<TvChannel *> newProgrammes;

    // Will leave the XML stream positioned on </tv>.
    Q_ASSERT(reader->isStartElement());
    Q_ASSERT(reader->name() == QLatin1String("tv"));
    while (!reader->hasError()) {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader->name() == QLatin1String("programme")) {
                channelId = reader->attributes().value
                        (QLatin1String("channel")).toString();
                channel = m_channels.value(channelId, 0);
                if (!channel) {
                    channel = new TvChannel(this);
                    channel->setId(channelId);
                    channel->setName(channelId);
                    m_channels.insert(channelId, channel);
                    newChannels = true;
                    if (m_hiddenChannelIds.contains(channelId))
                        channel->setHidden(true);
                    if (m_timezoneConvertChannelIds.contains(channelId))
                        channel->setConvertTimezone(true);
                }
                programme = new TvProgramme(channel);
                programme->load(reader);
                channel->addProgramme(programme);
                newProgrammes.insert(channel);

                // Update the category and credit sets with new values.
                programme->updateCategorySet(m_categories);
                programme->updateCreditSet(m_credits);
            } else if (reader->name() == QLatin1String("channel")) {
                channelId = reader->attributes().value
                        (QLatin1String("id")).toString();
                channel = m_channels.value(channelId, 0);
                if (channel) {
                    if (channel->load(reader)) {
                        newChannels = true;
                        if (channel->trimProgrammes())
                            newProgrammes.insert(channel);
                    }
                } else {
                    channel = new TvChannel(this);
                    channel->load(reader);
                    m_channels.insert(channelId, channel);
                    newChannels = true;
                    if (m_hiddenChannelIds.contains(channelId))
                        channel->setHidden(true);
                    if (m_timezoneConvertChannelIds.contains(channelId))
                        channel->setConvertTimezone(true);
                }
                if (channel->hasDataFor())
                    m_hasDataFor = true;
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("tv"))
                break;
        }
    }

    // Emit pending signals.
    if (newChannels) {
        // Construct the sorted "active channels" list.  If we have
        // "datafor" declarations in the channel list, then omit
        // any channels that have no information recorded.
        if (m_hasDataFor) {
            QMap<QString, TvChannel *>::ConstIterator it;
            m_activeChannels.clear();
            for (it = m_channels.constBegin();
                    it != m_channels.constEnd(); ++it) {
                TvChannel *channel = it.value();
                if (channel->hasDataFor())
                    m_activeChannels.append(channel);
            }
        } else {
            m_activeChannels = m_channels.values();
        }
        if (m_startUrl.host().endsWith(QLatin1String(".oztivo.net")))
            loadOzTivoChannelData();
        qSort(m_activeChannels.begin(),
              m_activeChannels.end(), sortActiveChannels);
        emit channelsChanged();
    }
    if (!newProgrammes.isEmpty()) {
        QSet<TvChannel *>::ConstIterator it;
        for (it = newProgrammes.constBegin();
                it != newProgrammes.constEnd(); ++it) {
            emit programmesChanged(*it);
        }
    }
    if (url == m_startUrl) {
        refreshIcons();
        emit channelIndexLoaded();
    }
}

void TvChannelList::loadOzTivoChannelData()
{
    QFile file(QLatin1String(":/data/channels_oztivo.xml"));
    if (!file.open(QIODevice::ReadOnly))
        return;
    QXmlStreamReader reader(&file);
    while (!reader.hasError()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("channel")) {
                QString channelId = reader.attributes().value
                        (QLatin1String("id")).toString();
                TvChannel *channel = this->channel(channelId);
                if (channel)
                    loadOzTivoChannelData(&reader, channel);
            }
        }
    }
}

void TvChannelList::loadOzTivoChannelData
    (QXmlStreamReader *reader, TvChannel *channel)
{
    // Will leave the XML stream positioned on </channel>.
    Q_ASSERT(reader->isStartElement());
    Q_ASSERT(reader->name() == QLatin1String("channel"));
    QString commonId = reader->attributes().value(QLatin1String("common-id")).toString();
    channel->setCommonId(commonId);
    while (!reader->hasError()) {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader->name() == QLatin1String("number")) {
                if (reader->attributes().value
                        (QLatin1String("system")) ==
                                QLatin1String("foxtel")) {
                    // Ignore foxtel channel numbers on channels
                    // that already have a free-to-air digital number.
                    if (!channel->channelNumbers().isEmpty())
                        continue;
                }
                QString num = reader->readElementText
                    (QXmlStreamReader::SkipChildElements);
                channel->addChannelNumber(num);
                m_haveChannelNumbers = true;
            } else if (reader->name() == QLatin1String("icon")) {
                QString src = reader->attributes().value
                    (QLatin1String("src")).toString();
                channel->setIconUrl(src);
                channel->setIconFile(m_iconFiles.value(channel->id(), QString()));
                if (!channel->iconFile().isEmpty()) {
                    if (QFile::exists(channel->iconFile())) {
                        channel->setIcon(QIcon(channel->iconFile()));
                    } else {
                        // Icon file no longer exists.
                        channel->setIcon(QIcon());
                        channel->setIconFile(QString());
                        m_iconFiles.remove(channel->id());
                    }
                } else {
                    channel->setIcon(QIcon());
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("channel"))
                break;
        }
    }
}

void TvChannelList::refreshChannels(bool forceReload)
{
    // Add the start URL to the front of the queue to fetch
    // it as soon as the current request completes.
    if (m_startUrl.isValid()) {
        Request req;
        req.urls += m_startUrl;
        req.priority = 0;
        req.isIconFetch = false;
        req.channel = 0;
        req.date = QDate();
        requestData(req, QDateTime(),
                    forceReload ? -1 : m_startUrlRefresh);
    }
}

// Request a particular day's data based on user selections.
void TvChannelList::requestChannelDay(TvChannel *channel, const QDate &date, int days, bool trimPrevious)
{
    Q_ASSERT(channel);

    // No point performing a network request if no data for the day.
    if (!channel->hasDataFor(date))
        return;

    // Trim requests for priority 1 and 2, which are the requests
    // for the current day and the next day.  Since we are about
    // to request a different day for the UI, there's no point
    // retrieving the previous day's data any more.
    if (trimPrevious)
        trimRequests(1, 2);

    // Fetch the day URL and start a request for it.
    QList<QUrl> urls = channel->dayUrls(date);
    if (urls.isEmpty())
        return;
    Request req;
    req.urls = urls;
    req.priority = 1;
    req.isIconFetch = false;
    req.channel = channel;
    req.date = date;
    requestData(req, channel->dayLastModified(date));

    // Add extra days if we want a 7-day outlook.  And add one more
    // day after that to populate "Late Night" timeslots, which are
    // actually "Early Morning" the next day.
    int extraDay = 1;
    while (extraDay <= days) {
        QDate nextDay = date.addDays(extraDay);
        if (channel->hasDataFor(nextDay)) {
            urls = channel->dayUrls(nextDay);
            if (!urls.isEmpty()) {
                req.urls = urls;
                req.priority = 2;
                req.isIconFetch = false;
                req.channel = channel;
                req.date = nextDay;
                requestData(req, channel->dayLastModified(nextDay));
            }
        }
        ++extraDay;
    }
}

void TvChannelList::abort()
{
    m_currentRequest = Request();
    m_requests.clear();
    m_contents.clear();
    m_busy = false;
    m_progress = 1.0f;
    m_requestsToDo = 0;
    m_requestsDone = 0;
    if (m_reply) {
        disconnect(m_reply, SIGNAL(readyRead()), this, SLOT(requestReadyRead()));
        disconnect(m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
        disconnect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                   this, SLOT(requestError(QNetworkReply::NetworkError)));
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;
    }
    emit busyChanged(m_busy);
    emit progressChanged(m_progress);
}

void TvChannelList::reload()
{
    // Clear the "last-fetched" times and force a request to
    // the server to get the channel list.  We'll still use
    // If-Modified-Since to reuse the local disk copy if possible,
    // but we want to know if the cache is up to date on reload.
    m_lastFetch.clear();
    m_loaded.clear();
    refreshChannels(true);
}

void TvChannelList::reloadService()
{
    abort();

    qDeleteAll(m_channels);
    m_channels.clear();
    m_activeChannels.clear();
    m_hiddenChannelIds.clear();
    m_timezoneConvertChannelIds.clear();
    m_iconFiles.clear();
    m_hasDataFor = false;
    m_largeIcons = true;
    m_haveChannelNumbers = false;
    m_bookmarkList.clearBookmarks();
    m_bookmarkList.clearTicks();
    m_serviceId = QString();
    m_serviceName = QString();
    m_startUrl = QUrl();
    m_loaded.clear();

    emit channelsChanged();
    emit bookmarksChanged();

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("Service"));
    m_serviceId = settings.value(QLatin1String("id")).toString();
    m_serviceName = settings.value(QLatin1String("name")).toString();
    QString url = settings.value(QLatin1String("url")).toString();
    if (!url.isEmpty())
        m_startUrl = QUrl(url);
    else
        m_startUrl = QUrl();
    m_startUrlRefresh = settings.value(QLatin1String("refresh"), 24).toInt();
    if (m_startUrlRefresh < 1)
        m_startUrlRefresh = 1;
    settings.endGroup();
    loadServiceSettings(&settings);

    QTimer::singleShot(0, this, SLOT(refreshChannels()));
}

void TvChannelList::updateChannels(bool largeIcons)
{
    QSet<QString> hidden;
    QSet<QString> convert;
    QMap<QString, QString> iconFiles;
    for (int index = 0; index < m_activeChannels.size(); ++index) {
        TvChannel *channel = m_activeChannels.at(index);
        if (channel->isHidden())
            hidden.insert(channel->id());
        QString file = channel->iconFile();
        if (!file.isEmpty())
            iconFiles.insert(channel->id(), file);
        if (channel->convertTimezone())
            convert.insert(channel->id());
    }
    if (m_hiddenChannelIds != hidden ||
            m_timezoneConvertChannelIds != convert ||
            m_iconFiles != iconFiles ||
            m_largeIcons != largeIcons) {
        m_hiddenChannelIds = hidden;
        m_timezoneConvertChannelIds = convert;
        m_iconFiles = iconFiles;
        m_largeIcons = largeIcons;
        saveChannelSettings();
        emit hiddenChannelsChanged();
        refreshIcons();
    }
}

void TvChannelList::timezoneSettingsChanged()
{
    m_loaded.clear();
    refreshChannels(false);
}

void TvChannelList::clearCache()
{
    m_nam.cache()->clear();
    reload();
}

void TvChannelList::reloadBookmarks()
{
    QMap<QString, TvChannel *>::ConstIterator it;
    for (it = m_channels.constBegin();
            it != m_channels.constEnd(); ++it) {
        TvChannel *channel = it.value();
        channel->reloadBookmarks();
    }
}

void TvChannelList::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    // TODO
    Q_UNUSED(reply);
    Q_UNUSED(authenticator);
}

#ifndef QT_NO_OPENSSL

void TvChannelList::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    // TODO
    Q_UNUSED(reply);
    Q_UNUSED(errors);
}

#endif

void TvChannelList::throttleTimeout()
{
    m_throttled = false;
    nextPending();
}

void TvChannelList::requestReadyRead()
{
    char buffer[1024];
    qint64 len;
    while ((len = m_reply->read(buffer, sizeof(buffer))) > 0)
        m_contents.append(buffer, (int)len);
}

void TvChannelList::requestFinished()
{
    if (!m_reply)
        return;
    m_reply->deleteLater();
    m_reply = 0;
    QUrl currentUrl = m_currentRequest.url();
    if (!m_contents.isEmpty()) {
#ifdef DEBUG_NETWORK
        qWarning() << "fetch succeeded:" << currentUrl << "size:" << m_contents.size();
#endif
        m_lastFetch.insert(currentUrl, QDateTime::currentDateTime());
        if (!m_currentRequest.isIconFetch) {
            QXmlStreamReader reader(m_contents);
            while (!reader.hasError()) {
                QXmlStreamReader::TokenType tokenType = reader.readNext();
                if (tokenType == QXmlStreamReader::StartElement) {
                    if (reader.name() == QLatin1String("tv")) {
                        load(&reader, currentUrl);
                        if (m_currentRequest.channel)
                            m_loaded += QPair<TvChannel *, int>(m_currentRequest.channel, m_currentRequest.date.toJulianDay());
                    }
                } else if (tokenType == QXmlStreamReader::EndDocument) {
                    break;
                }
            }
        } else {
            setIconData(m_currentRequest.channel, m_contents, currentUrl);
        }
        m_contents = QByteArray();
    } else {
#ifdef DEBUG_NETWORK
        qWarning() << "fetch failed:" << currentUrl;
#endif
    }
    int index = 0;
    while (index < m_requests.size()) {
        // Remove repeated entries for the same URL at other priorities.
        if (m_requests.at(index).urls.contains(currentUrl)) {
            m_requests.removeAt(index);
            --m_requestsToDo;
        } else {
            ++index;
        }
    }
    m_currentRequest = Request();
    ++m_requestsDone;
    nextPending();
    if (!m_currentRequest.isValid() && m_busy && m_requests.isEmpty()) {
        // Turn off the busy flag.
        m_busy = false;
        m_progress = 1.0f;
        m_requestsToDo = 0;
        m_requestsDone = 0;
        emit busyChanged(m_busy);
        emit progressChanged(m_progress);
    }
}

void TvChannelList::requestError(QNetworkReply::NetworkError error)
{
    qWarning() << "TvChannelList: request for url"
               << m_currentRequest.url() << "failed, error =" << int(error);
}

void TvChannelList::requestData
    (const Request &req, const QDateTime &lastmod, int refreshAge)
{
    // Bail out if the url is currently being requested.
    QUrl currentUrl = m_currentRequest.url();
    if (currentUrl.isValid() && req.urls.contains(currentUrl))
        return;

    // Look in the cache to see if the data is fresh enough.
    // Check all URL's in the list in case the same data is
    // fresh under a different URL.  We assume that the data is
    // fresh if the Last-Modified time matches what we expect
    // or if the last time we fetched it within this process
    // was less than an hour ago.
    QUrl url;
    QIODevice *device = 0;
    QDateTime age = QDateTime::currentDateTime().addSecs(-(60 * 60));
    QDateTime refAge = QDateTime::currentDateTime().addSecs(-(60 * 60 * refreshAge));
    QDateTime lastFetch;
    for (int index = 0; index < req.urls.size(); ++index) {
        if (req.isIconFetch)
            break;      // Don't look in the cache for icon fetches.
        url = req.urls.at(index);
        if (lastmod.isValid()) {
            QNetworkCacheMetaData meta = m_nam.cache()->metaData(url);
            if (meta.isValid() && meta.lastModified() == lastmod) {
                device = m_nam.cache()->data(url);
                if (device) {
#ifdef DEBUG_NETWORK
                    qWarning() << "using cache for:" << url
                               << "last modified:" << lastmod.toLocalTime();
#endif
                    break;
                }
            }
        } else if (refreshAge != -1) {
            QNetworkCacheMetaData meta = m_nam.cache()->metaData(url);
            if (meta.isValid() && meta.lastModified() >= refAge) {
                device = m_nam.cache()->data(url);
                if (device) {
#ifdef DEBUG_NETWORK
                    qWarning() << "using cache for:" << url
                               << "last modified:" << meta.lastModified().toLocalTime()
                               << "refresh: every" << refreshAge << "hours";
#endif
                    break;
                }
            }
        }
        lastFetch = m_lastFetch.value(url, QDateTime());
        if (lastFetch.isValid() && lastFetch >= age) {
            device = m_nam.cache()->data(url);
            if (device) {
#ifdef DEBUG_NETWORK
                qWarning() << "using cache for:" << url
                           << "last fetched:" << lastFetch;
#endif
                break;
            }
        }
    }
    QPair<TvChannel *, int> loadedKey(req.channel, req.date.toJulianDay());
    if (device) {
        if (m_loaded.contains(loadedKey)) {
#ifdef DEBUG_NETWORK
            qWarning() << "skipping reparse for:" << url;
#endif
            delete device;
            return;
        }
        QXmlStreamReader *reader = new QXmlStreamReader(device);
        while (!reader->hasError()) {
            QXmlStreamReader::TokenType tokenType = reader->readNext();
            if (tokenType == QXmlStreamReader::StartElement) {
                if (reader->name() == QLatin1String("tv")) {
                    load(reader, url);
                    if (req.channel)
                        m_loaded += loadedKey;
                }
            } else if (tokenType == QXmlStreamReader::EndDocument) {
                break;
            }
        }
        delete reader;
        delete device;
        return;
    }
    m_loaded -= loadedKey;

    // Add the request to the queue, in priority order.
    int index = 0;
    url = req.urls.at(0);
    while (index < m_requests.size()) {
        const Request &r = m_requests.at(index);
        if (r.priority == req.priority && r.urls.contains(url))
            return;     // We have already queued this request.
        if (r.priority > req.priority)
            break;
        ++index;
    }
    m_requests.insert(index, req);
    ++m_requestsToDo;

    // Start the first request if nothing else is active.
    nextPending();
}

void TvChannelList::trimRequests(int first, int last)
{
    int index = 0;
    bool removed = false;
    while (index < m_requests.size()) {
        const Request &r = m_requests.at(index);
        if (r.priority >= first && r.priority <= last) {
            m_requests.removeAt(index);
            --m_requestsToDo;
            removed = true;
        } else {
            ++index;
        }
    }
    if (removed) {
        if (m_requests.isEmpty() && !m_currentRequest.isValid()) {
            m_busy = false;
            m_progress = 1.0f;
            m_requestsToDo = 0;
            m_requestsDone = 0;
            emit busyChanged(m_busy);
            emit progressChanged(m_progress);
        } else {
            forceProgressUpdate();
        }
    }
}

void TvChannelList::nextPending()
{
    // Bail out if already processing a request, there are no
    // pending requests, or we are currently throttled.
    if (m_currentRequest.isValid() || m_requests.isEmpty() || m_throttled) {
        forceProgressUpdate();
        return;
    }

    // Initiate a GET request for the next pending URL.
    Request req = m_requests.takeFirst();
    m_currentRequest = req;
    QUrl currentUrl = m_currentRequest.url();
    QNetworkRequest request;
    request.setUrl(currentUrl);
    request.setRawHeader("User-Agent", "qtvguide/" TVGUIDE_VERSION);
    m_contents = QByteArray();
    m_reply = m_nam.get(request);
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(requestReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(requestError(QNetworkReply::NetworkError)));
    m_lastFetch.remove(currentUrl);
#ifdef DEBUG_NETWORK
    qWarning() << "fetching from network:" << currentUrl;
#endif

    // Start the throttle timer.  According to the OzTivo guidelines,
    // there must be at least 1 second between requests.  Requests
    // must also be performed in serial; never in parallel.
    // http://www.oztivo.net/twiki/bin/view/TVGuide/StaticXMLGuideAPI
    //
    // If a request takes 3 seconds to complete then the next request
    // will start immediately.  But if the request takes 0.5 seconds
    // to complete then there will be a 0.5 second delay before the
    // next GET is sent.  This should give slightly better performance
    // for interactive use and when fetching the data for multiple
    // days or channels, while still technically sending no more than
    // one request per second.
    //
    // For icon fetches we start the next request immediately as they
    // typically won't be going to the XML data server.
    m_throttleTimer->start(req.isIconFetch ? 0 : 1000);
    m_throttled = true;

    // Tell the UI that a network request has been initiated.
    emit networkRequest(req.channel, req.date, req.isIconFetch);

    // Turn on the busy flag and report the progress.
    if (!m_busy) {
        m_busy = true;
        emit busyChanged(true);
    }
    forceProgressUpdate();
}

void TvChannelList::forceProgressUpdate()
{
    if (m_requestsDone < m_requestsToDo)
        m_progress = qreal(m_requestsDone) / qreal(m_requestsToDo);
    else
        m_progress = 1.0f;
    emit progressChanged(m_progress);
}

void TvChannelList::loadServiceSettings(QSettings *settings)
{
    if (m_serviceId.isEmpty())
        return;

    settings->beginGroup(m_serviceId);
    m_largeIcons = settings->value(QLatin1String("largeIcons"), true).toBool();
    m_hiddenChannelIds.clear();
    m_timezoneConvertChannelIds.clear();
    m_iconFiles.clear();
    int size = settings->beginReadArray(QLatin1String("channels"));
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        QString id = settings->value(QLatin1String("id")).toString();
        if (id.isEmpty())
            continue;
        bool hidden = settings->value(QLatin1String("hidden"), false).toBool();
        if (hidden)
            m_hiddenChannelIds.insert(id);
        QString file = settings->value(QLatin1String("icon")).toString();
        if (!file.isEmpty())
            m_iconFiles.insert(id, file);
        bool timezone = settings->value(QLatin1String("convertTimezone"), false).toBool();
        if (timezone)
            m_timezoneConvertChannelIds.insert(id);
    }
    settings->endArray();

    m_bookmarkList.clearBookmarks();
    size = settings->beginReadArray(QLatin1String("bookmarks"));
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        TvBookmark *bookmark = new TvBookmark();
        bookmark->load(settings);
        m_bookmarkList.addBookmark(bookmark, false);
    }
    settings->endArray();

    m_bookmarkList.loadTicks(settings);

    setGroups(TvChannelGroup::loadSettings(this, settings));

    settings->endGroup();
}

void TvChannelList::saveChannelSettings()
{
    if (m_serviceId.isEmpty())
        return;
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(m_serviceId);
    settings.setValue(QLatin1String("largeIcons"), m_largeIcons);
    settings.beginWriteArray(QLatin1String("channels"));
    int aindex = 0;
    for (int index = 0; index < m_activeChannels.size(); ++index) {
        TvChannel *channel = m_activeChannels.at(index);
        if (!channel->isHidden() && channel->iconFile().isEmpty())
            continue;
        settings.setArrayIndex(aindex++);
        settings.setValue(QLatin1String("id"), channel->id());
        settings.setValue(QLatin1String("hidden"), channel->isHidden());
        QString file = channel->iconFile();
        if (file.isEmpty())
            settings.remove(QLatin1String("icon"));
        else
            settings.setValue(QLatin1String("icon"), file);
        settings.setValue(QLatin1String("convertTimezone"), channel->convertTimezone());
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

void TvChannelList::saveGroups()
{
    if (m_serviceId.isEmpty())
        return;
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(m_serviceId);
    TvChannelGroup::saveSettings(m_groups, &settings);
    settings.endGroup();
    settings.sync();
}

void TvChannelList::saveBookmarks()
{
    if (m_serviceId.isEmpty())
        return;
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(m_serviceId);
    settings.beginWriteArray(QLatin1String("bookmarks"));
    QList<TvBookmark *> bookmarks = m_bookmarkList.bookmarks();
    for (int index = 0; index < bookmarks.size(); ++index) {
        TvBookmark *bookmark = bookmarks.at(index);
        settings.setArrayIndex(index);
        bookmark->save(&settings);
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

void TvChannelList::saveTicks()
{
    if (m_serviceId.isEmpty())
        return;
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(m_serviceId);
    m_bookmarkList.saveTicks(&settings);
    settings.endGroup();
    settings.sync();
}

void TvChannelList::refreshIcons()
{
    for (int index = 0; index < m_activeChannels.size(); ++index) {
        TvChannel *channel = m_activeChannels.at(index);
        if (channel->isHidden())
            continue;
        if (!channel->iconFile().isEmpty())
            continue;
        if (channel->iconUrl().isEmpty())
            continue;

        // Queue up a request to fetch the icon for this channel.
        Request req;
        req.urls += QUrl(channel->iconUrl());
        req.priority = 10;
        req.isIconFetch = true;
        req.channel = channel;
        requestData(req, QDateTime());
    }
}

void TvChannelList::setIconData(TvChannel *channel, const QByteArray &data, const QUrl &url)
{
    // Construct the local file pathname to hold the icon data.
    QString path = url.path();
    QString filename = QDir::homePath() +
                       QLatin1String("/.qtvguide");
    QString dirPath = QLatin1String("icons/") + m_serviceId;
    QDir dir(filename);
    if (!dir.mkpath(dirPath))
        return;
    filename += QLatin1Char('/') + dirPath +
                QLatin1Char('/') + channel->id();
    if (path.endsWith(".jpg", Qt::CaseInsensitive))
        filename += QLatin1String(".jpg");
    else if (path.endsWith(".png", Qt::CaseInsensitive))
        filename += QLatin1String(".png");
    else
        return;

    // Save the icon data and then update the channel information.
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    file.write(data);
    file.close();
    channel->setIconFile(filename);
    channel->setIcon(QIcon(filename));
    m_iconFiles.insert(channel->id(), filename);
    saveChannelSettings();
    emit channelIconsChanged();
}
