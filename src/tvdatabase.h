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

#ifndef _TVDATABASE_H
#define _TVDATABASE_H

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>

class TvDatabase : public QObject
{
    Q_OBJECT
public:
    explicit TvDatabase(QObject *parent = 0);
    ~TvDatabase();

    QUrl startUrl() const { return m_startUrl; }
    void setStartUrl(const QUrl &url);

public Q_SLOTS:
    void refresh();

private Q_SLOTS:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
    void startReadyRead();
    void startFinished();
    void startError(QNetworkReply::NetworkError);

private:
    QNetworkAccessManager m_nam;
    QUrl m_startUrl;
};

#endif
