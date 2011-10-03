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
#include "tvbookmark.h"
#include "tvbookmarklist.h"
#include "tvtick.h"
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qpair.h>
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QtCore/qsettings.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>

class TvChannelList : public QObject
{
    Q_OBJECT
public:
    explicit TvChannelList(QObject *parent = 0);
    ~TvChannelList();

    bool hasService() const { return !m_serviceId.isEmpty(); }
    QString serviceName() const { return m_serviceName; }

    QUrl startUrl() const { return m_startUrl; }
    void setStartUrl(const QUrl &url) { m_startUrl = url; }

    TvChannel *channel(const QString &id) const;

    QList<TvChannel *> channels() const { return m_channels.values(); }
    QStringList channelIds() const { return m_channels.keys(); }

    QList<TvChannel *> activeChannels() const { return m_activeChannels; }

    bool haveChannelNumbers() const { return m_haveChannelNumbers; }

    bool busy() const { return m_busy; }
    qreal progress() const { return m_progress; }
    bool useSimpleProgress() const { return m_requestsToDo <= 3; }

    bool largeIcons() const { return m_largeIcons; }
    void setLargeIcons(bool value) { m_largeIcons = value; }

    TvBookmarkList *bookmarkList() { return &m_bookmarkList; }

public Q_SLOTS:
    void refreshChannels(bool forceReload = false);
    void requestChannelDay(TvChannel *channel, const QDate &date, int days = 1, bool trimPrevious = true);
    void abort();
    void reload();
    void reloadService();
    void updateChannels(bool largeIcons);
    void timezoneSettingsChanged();
    void clearCache();
    void reloadBookmarks();

private Q_SLOTS:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
    void throttleTimeout();
    void requestReadyRead();
    void requestFinished();
    void requestError(QNetworkReply::NetworkError);
    void saveBookmarks();
    void saveTicks();

Q_SIGNALS:
    void channelIndexLoaded();
    void channelsChanged();
    void hiddenChannelsChanged();
    void programmesChanged(TvChannel *channel);
    void busyChanged(bool value);
    void progressChanged(qreal value);
    void bookmarksChanged();
    void networkRequest(TvChannel *channel, const QDate &date, bool isIconFetch);
    void channelIconsChanged();

private:
    struct Request
    {
        QList<QUrl> urls;
        int priority;
        bool isIconFetch;
        TvChannel *channel;
        QDate date;

        QUrl url() const { return urls.isEmpty() ? QUrl() : urls.at(0); }
        bool isValid() const { return !urls.isEmpty(); }

        Request() { priority = 0; isIconFetch = false; channel = 0; }
    };

    QMap<QString, TvChannel *> m_channels;
    QList<TvChannel *> m_activeChannels;
    QSet<QString> m_hiddenChannelIds;
    QSet<QString> m_timezoneConvertChannelIds;
    QMap<QString, QString> m_iconFiles;
    QNetworkAccessManager m_nam;
    QString m_serviceId;
    QString m_serviceName;
    QUrl m_startUrl;
    int m_startUrlRefresh;
    QList<Request> m_requests;
    Request m_currentRequest;
    QTimer *m_throttleTimer;
    QSet< QPair<TvChannel *, int> > m_loaded;
    bool m_hasDataFor;
    bool m_throttled;
    bool m_busy;
    bool m_largeIcons;
    bool m_haveChannelNumbers;
    qreal m_progress;
    int m_requestsToDo;
    int m_requestsDone;
    QNetworkReply *m_reply;
    QByteArray m_contents;
    QMap<QUrl, QDateTime> m_lastFetch;
    TvBookmarkList m_bookmarkList;

    void load(QXmlStreamReader *reader, const QUrl &url);
    void loadOzTivoChannelData();
    void loadOzTivoChannelData(QXmlStreamReader *reader, TvChannel *channel);
    void requestData(const Request &req, const QDateTime &lastmod, int refreshAge = -1);
    void trimRequests(int first, int last);
    void nextPending();
    void forceProgressUpdate();
    void loadServiceSettings(QSettings *settings);
    void saveChannelSettings();
    void refreshIcons();
    void setIconData(TvChannel *channel, const QByteArray &data, const QUrl &url);

    friend class TvChannel;
};

#endif
