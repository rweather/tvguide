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

TvChannelList::TvChannelList(QObject *parent)
    : QObject(parent)
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

    m_startUrl = QUrl(QLatin1String("http://xml.oztivo.net/xmltv/datalist.xml.gz"));

    m_throttleTimer = new QTimer(this);
    m_throttleTimer->setSingleShot(true);
    connect(m_throttleTimer, SIGNAL(timeout()),
            this, SLOT(throttleTimeout()));
}

TvChannelList::~TvChannelList()
{
    qDeleteAll(m_channels);
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

void TvChannelList::refreshChannels()
{
    // Add the start URL to the front of the queue to fetch
    // it as soon as the current request completes.
    if (m_startUrl.isValid())
        prependPending(m_startUrl);
}

// Request a particular day's data based on user selections.
void TvChannelList::requestChannelDay(TvChannel *channel, const QDate &date)
{
    Q_ASSERT(channel);

    // No point performing a network request if no data for the day.
    if (!channel->hasDataFor(date))
        return;

    // Fetch the day URL and start a request for it.  We bump it to
    // the front of the queue since it is the most recent date the user
    // selected in the UI, and hence the one needed most urgently.
    QString url = channel->dayUrl(date);
    if (url.isEmpty())
        return;
    prependPending(QUrl(url));
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
    QString url = channel->dayUrl(date);
    if (url.isEmpty())
        return;
    appendPending(QUrl(url));
}

void TvChannelList::abort()
{
    m_currentRequest = QUrl();
    m_pending.clear();
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
    refreshChannels();
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

void TvChannelList::appendPending(const QUrl &url)
{
    if (m_currentRequest == url)
        return;
    if (!m_pending.contains(url)) {
        m_pending.append(url);
        ++m_requestsToDo;
    }
    nextPending();
}

void TvChannelList::prependPending(const QUrl &url)
{
    if (m_currentRequest == url)
        return;
    if (m_pending.contains(url)) {
        m_pending.removeAll(url);
        --m_requestsToDo;
    }
    m_pending.prepend(url);
    ++m_requestsToDo;
    nextPending();
}

void TvChannelList::nextPending()
{
    // Bail out if already processing a request, there are no
    // pending requests, or we are currently throttled.
    if (m_currentRequest.isValid() || m_pending.isEmpty() || m_throttled) {
        forceProgressUpdate();
        return;
    }

    // Serve requests directly from the disk cache if possible.
    // Anything that is less than 1 hour old is used as-is.
    // Data that is older is re-requested from the server using
    // If-Modified-Since to check for a more up to date copy.
    QUrl url;
    QDateTime age = QDateTime::currentDateTime().addSecs(-(60 * 60));
    for (;;) {
        if (m_pending.isEmpty()) {
            // All pending requests have been served from the cache.
            if (m_busy) {
                m_busy = false;
                m_progress = 1.0f;
                m_requestsToDo = 0;
                m_requestsDone = 0;
                emit busyChanged(m_busy);
                emit progressChanged(m_progress);
            }
            return;
        }
        url = m_pending.takeFirst();
        QDateTime lastFetch = m_lastFetch.value(url, QDateTime());
        if (lastFetch.isValid() && lastFetch >= age) {
            QIODevice *device = m_nam.cache()->data(url);
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
                ++m_requestsDone;
                continue;
            }
        }
        break;
    }

    // Initiate a GET request for the next pending URL.
    m_currentRequest = url;
    QNetworkRequest request;
    request.setUrl(m_currentRequest);
    request.setRawHeader("User-Agent", "qtvguide/0.0.1");
    m_contents = QByteArray();
    m_reply = m_nam.get(request);
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(requestReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(requestError(QNetworkReply::NetworkError)));
    m_lastFetch.remove(url);

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
