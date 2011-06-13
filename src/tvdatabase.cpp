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

#include "tvdatabase.h"
#include <QtNetwork/qnetworkdiskcache.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtCore/qdir.h>
#include <unistd.h>

TvDatabase::TvDatabase(QObject *parent)
    : QObject(parent)
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
}

TvDatabase::~TvDatabase()
{
}

void TvDatabase::setStartUrl(const QUrl &url)
{
    if (m_startUrl != url) {
        m_startUrl = url;
        // TODO: invalidate cached information
    }
}

void TvDatabase::refresh()
{
    QNetworkRequest request;
    request.setUrl(m_startUrl);
    request.setRawHeader("User-Agent", "qtvguide/0.0.1");

    QNetworkReply *reply = m_nam.get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(startReadyRead()));
    connect(reply, SIGNAL(finished()), this, SLOT(startFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(startError(QNetworkReply::NetworkError)));
}

void TvDatabase::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    // TODO
}

#ifndef QT_NO_OPENSSL

void TvDatabase::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    // TODO
}

#endif

void TvDatabase::startReadyRead()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    char buffer[1024];
    qint64 len;
    while ((len = reply->read(buffer, sizeof(buffer))) > 0) {
        //::write(1, buffer, (int)len);
    }
}

void TvDatabase::startFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
}

void TvDatabase::startError(QNetworkReply::NetworkError)
{
    // TODO
}
