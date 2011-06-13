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

#ifndef _TVCHANNELLIST_H
#define _TVCHANNELLIST_H

#include "tvchannel.h"
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>

class TvChannelList : public QObject
{
    Q_OBJECT
public:
    explicit TvChannelList(QObject *parent = 0);
    ~TvChannelList();

    QUrl startUrl() const { return m_startUrl; }
    void setStartUrl(const QUrl &url) { m_startUrl = url; }

    TvChannel *channel(const QString &id) const;

    QList<TvChannel *> channels() const { return m_channels.values(); }
    QStringList channelIds() const { return m_channels.keys(); }

    bool busy() const { return m_busy; }
    qreal progress() const { return m_progress; }

public Q_SLOTS:
    void refreshChannels();
    void requestChannelDay(TvChannel *channel, const QDate &date);
    void enqueueChannelDay(TvChannel *channel, const QDate &date);
    void abort();
    void reload();

private Q_SLOTS:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
    void throttleTimeout();
    void requestReadyRead();
    void requestFinished();
    void requestError(QNetworkReply::NetworkError);

Q_SIGNALS:
    void channelIndexLoaded();
    void channelsChanged();
    void programmesChanged(TvChannel *channel);
    void busyChanged(bool value);
    void progressChanged(qreal value);

private:
    QMap<QString, TvChannel *> m_channels;
    QNetworkAccessManager m_nam;
    QUrl m_startUrl;
    QList<QUrl> m_pending;
    QUrl m_currentRequest;
    QTimer *m_throttleTimer;
    bool m_throttled;
    bool m_busy;
    qreal m_progress;
    int m_requestsToDo;
    int m_requestsDone;
    QNetworkReply *m_reply;
    QByteArray m_contents;
    QMap<QUrl, QDateTime> m_lastFetch;

    void load(QXmlStreamReader *reader, const QUrl &url);
    void appendPending(const QUrl &url);
    void prependPending(const QUrl &url);
    void nextPending();
    void forceProgressUpdate();
};

#endif
