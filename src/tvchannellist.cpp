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
#include <QtCore/qdebug.h>
#include <QtNetwork/qnetworkdiskcache.h>

//#define DEBUG_NETWORK 1

TvChannelList::TvChannelList(QObject *parent)
    : QObject(parent)
    , m_startUrlRefresh(24)
    , m_hasDataFor(false)
    , m_throttled(false)
    , m_busy(false)
    , m_progress(1.0f)
    , m_requestsToDo(0)
    , m_requestsDone(0)
    , m_reply(0)
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

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("Service"));
    m_serviceName = settings.value
        (QLatin1String("name"), QLatin1String("OzTivo")).toString();
    QString url = settings.value
        (QLatin1String("url"),
         QLatin1String("http://xml.oztivo.net/xmltv/datalist.xml.gz")).toString();
    if (!url.isEmpty())
        m_startUrl = QUrl(url);
    m_startUrlRefresh = settings.value(QLatin1String("refresh"), 24).toInt();
    if (m_startUrlRefresh < 1)
        m_startUrlRefresh = 1;
    settings.endGroup();
    loadServiceSettings(&settings);
}

TvChannelList::~TvChannelList()
{
    qDeleteAll(m_channels);
    qDeleteAll(m_bookmarks);
}

TvChannel *TvChannelList::channel(const QString &id) const
{
    return m_channels.value(id, 0);
}

static bool sortActiveChannels(TvChannel *c1, TvChannel *c2)
{
    return c1->name().compare(c2->name(), Qt::CaseInsensitive) < 0;
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
            if (reader->name() == QLatin1String("channel")) {
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
                }
                if (channel->hasDataFor())
                    m_hasDataFor = true;
            } else if (reader->name() == QLatin1String("programme")) {
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
                }
                programme = new TvProgramme(channel);
                programme->load(reader);
                channel->addProgramme(programme);
                newProgrammes.insert(channel);
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
    if (url == m_startUrl)
        emit channelIndexLoaded();
}

void TvChannelList::refreshChannels(bool forceReload)
{
    // Add the start URL to the front of the queue to fetch
    // it as soon as the current request completes.
    if (m_startUrl.isValid()) {
        QList<QUrl> urls;
        urls += m_startUrl;
        requestData(urls, QDateTime(), 0,
                    forceReload ? -1 : m_startUrlRefresh);
    }
}

// Request a particular day's data based on user selections.
void TvChannelList::requestChannelDay(TvChannel *channel, const QDate &date, int days)
{
    Q_ASSERT(channel);

    // No point performing a network request if no data for the day.
    if (!channel->hasDataFor(date))
        return;

    // Trim requests for priority 1 and 2, which are the requests
    // for the current day and the next day.  Since we are about
    // to request a different day for the UI, there's no point
    // retrieving the previous day's data any more.
    trimRequests(1, 2);

    // Fetch the day URL and start a request for it.
    QList<QUrl> urls = channel->dayUrls(date);
    if (urls.isEmpty())
        return;
    requestData(urls, channel->dayLastModified(date), 1);

    // Add extra days if we want a 7-day outlook.  And add one more
    // day after that to populate "Late Night" timeslots, which are
    // actually "Early Morning" the next day.
    int extraDay = 1;
    while (extraDay <= days) {
        QDate nextDay = date.addDays(extraDay);
        if (channel->hasDataFor(nextDay)) {
            urls = channel->dayUrls(nextDay);
            if (!urls.isEmpty())
                requestData(urls, channel->dayLastModified(nextDay), 2);
        }
        ++extraDay;
    }
}

// Enqueue a request for a day in the background for bulk
// downloading of channel data.
void TvChannelList::enqueueChannelDay(TvChannel *channel, const QDate &date)
{
    Q_ASSERT(channel);

    // No point performing a network request if no data for the day.
    if (!channel->hasDataFor(date))
        return;

    // Fetch the day URL and start a request for it.  Add the URL
    // to the end of the queue since timeliness is not important.
    QList<QUrl> urls = channel->dayUrls(date);
    if (urls.isEmpty())
        return;
    requestData(urls, channel->dayLastModified(date), 3);
}

void TvChannelList::abort()
{
    m_currentRequest = QUrl();
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
    refreshChannels(true);
}

void TvChannelList::updateHidden()
{
    QSet<QString> hidden;
    for (int index = 0; index < m_activeChannels.size(); ++index) {
        TvChannel *channel = m_activeChannels.at(index);
        if (channel->isHidden())
            hidden.insert(channel->id());
    }
    if (m_hiddenChannelIds != hidden) {
        m_hiddenChannelIds = hidden;
        saveChannelSettings();
        emit hiddenChannelsChanged();
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
    if (!m_contents.isEmpty()) {
#ifdef DEBUG_NETWORK
        qWarning() << "fetch succeeded:" << m_currentRequest << "size:" << m_contents.size();
#endif
        m_lastFetch.insert(m_currentRequest, QDateTime::currentDateTime());
        QXmlStreamReader reader(m_contents);
        while (!reader.hasError()) {
            QXmlStreamReader::TokenType tokenType = reader.readNext();
            if (tokenType == QXmlStreamReader::StartElement) {
                if (reader.name() == QLatin1String("tv"))
                    load(&reader, m_currentRequest);
            } else if (tokenType == QXmlStreamReader::EndDocument) {
                break;
            }
        }
        m_contents = QByteArray();
    } else {
#ifdef DEBUG_NETWORK
        qWarning() << "fetch failed:" << m_currentRequest;
#endif
    }
    int index = 0;
    while (index < m_requests.size()) {
        // Remove repeated entries for the same URL at other priorities.
        if (m_requests.at(index).urls.contains(m_currentRequest)) {
            m_requests.removeAt(index);
            --m_requestsToDo;
        } else {
            ++index;
        }
    }
    m_currentRequest = QUrl();
    ++m_requestsDone;
    nextPending();
    if (!m_currentRequest.isValid() && m_busy) {
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
               << m_currentRequest << "failed, error =" << int(error);
}

void TvChannelList::requestData
    (const QList<QUrl> &urls, const QDateTime &lastmod,
     int priority, int refreshAge)
{
    // Bail out if the url is currently being requested.
    if (m_currentRequest.isValid() && urls.contains(m_currentRequest))
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
    for (int index = 0; index < urls.size(); ++index) {
        url = urls.at(index);
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
    if (device) {
        QXmlStreamReader *reader = new QXmlStreamReader(device);
        while (!reader->hasError()) {
            QXmlStreamReader::TokenType tokenType = reader->readNext();
            if (tokenType == QXmlStreamReader::StartElement) {
                if (reader->name() == QLatin1String("tv"))
                    load(reader, url);
            } else if (tokenType == QXmlStreamReader::EndDocument) {
                break;
            }
        }
        delete reader;
        delete device;
        return;
    }

    // Add the request to the queue, in priority order.
    int index = 0;
    url = urls.at(0);
    while (index < m_requests.size()) {
        const Request &r = m_requests.at(index);
        if (r.priority == priority && r.urls.contains(url))
            return;     // We have already queued this request.
        if (r.priority > priority)
            break;
        ++index;
    }
    Request req;
    req.urls = urls;
    req.priority = priority;
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
    m_currentRequest = req.urls.at(0);
    QNetworkRequest request;
    request.setUrl(m_currentRequest);
    request.setRawHeader("User-Agent", "qtvguide/0.0.1");
    m_contents = QByteArray();
    m_reply = m_nam.get(request);
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(requestReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(requestError(QNetworkReply::NetworkError)));
    m_lastFetch.remove(m_currentRequest);
#ifdef DEBUG_NETWORK
    qWarning() << "fetching from network:" << m_currentRequest;
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
    m_throttleTimer->start(1000);
    m_throttled = true;

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
    settings->beginGroup(m_serviceName);
    m_hiddenChannelIds.clear();
    int size = settings->beginReadArray(QLatin1String("channels"));
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        QString id = settings->value(QLatin1String("id")).toString();
        if (id.isEmpty())
            continue;
        bool hidden = settings->value(QLatin1String("hidden"), false).toBool();
        if (hidden)
            m_hiddenChannelIds.insert(id);
    }
    settings->endArray();
    settings->endGroup();

    qDeleteAll(m_bookmarks);
    m_bookmarks.clear();
    m_indexedBookmarks.clear();
    settings->beginGroup(QLatin1String("Bookmarks"));
    size = settings->beginReadArray(QLatin1String("bookmarks"));
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        TvBookmark *bookmark = new TvBookmark();
        bookmark->load(settings);
        m_bookmarks.append(bookmark);
        m_indexedBookmarks.insert(bookmark->title().toLower(), bookmark);
    }
    settings->endArray();
    settings->endGroup();
}

void TvChannelList::saveChannelSettings()
{
    if (m_serviceName.isEmpty())
        return;
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(m_serviceName);
    settings.beginWriteArray(QLatin1String("channels"));
    int aindex = 0;
    for (int index = 0; index < m_activeChannels.size(); ++index) {
        TvChannel *channel = m_activeChannels.at(index);
        if (!channel->isHidden())
            continue;
        settings.setArrayIndex(aindex++);
        settings.setValue(QLatin1String("id"), channel->id());
        settings.setValue(QLatin1String("hidden"), channel->isHidden());
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

void TvChannelList::saveBookmarks()
{
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("Bookmarks"));
    settings.beginWriteArray(QLatin1String("bookmarks"));
    for (int index = 0; index < m_bookmarks.size(); ++index) {
        TvBookmark *bookmark = m_bookmarks.at(index);
        settings.setArrayIndex(index);
        bookmark->save(&settings);
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

void TvChannelList::addBookmark(TvBookmark *bookmark)
{
    Q_ASSERT(bookmark);
    m_bookmarks.append(bookmark);
    m_indexedBookmarks.insert(bookmark->title().toLower(), bookmark);
    emit bookmarksChanged();
    saveBookmarks();
}

TvBookmark::Match TvChannelList::matchBookmarks
    (const TvProgramme *programme, TvBookmark **bookmark) const
{
    QMultiMap<QString, TvBookmark *>::ConstIterator it;
    it = m_indexedBookmarks.constFind(programme->title().toLower());
    while (it != m_indexedBookmarks.constEnd()) {
        TvBookmark::Match match = it.value()->match(programme);
        if (match != TvBookmark::NoMatch) {
            *bookmark = it.value();
            return match;
        }
        ++it;
    }
    return TvBookmark::NoMatch;
}

void TvChannelList::replaceBookmarks(const QList<TvBookmark *> &bookmarks)
{
    qDeleteAll(m_bookmarks);
    m_bookmarks = bookmarks;
    m_indexedBookmarks.clear();
    for (int index = 0; index < bookmarks.size(); ++index) {
        TvBookmark *bookmark = bookmarks.at(index);
        m_indexedBookmarks.insert(bookmark->title().toLower(), bookmark);
    }
    emit bookmarksChanged();
    saveBookmarks();
}
