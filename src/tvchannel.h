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

#ifndef _TVCHANNEL_H
#define _TVCHANNEL_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qxmlstream.h>

class TvChannelList;
class TvProgramme;

class TvChannel
{
public:
    explicit TvChannel(TvChannelList *channelList);
    ~TvChannel();

    TvChannelList *channelList() const { return m_channelList; }

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QStringList baseUrls() const { return m_baseUrls; }
    QString dayUrl(const QDate &date) const;
    QDateTime dayLastModified(const QDate &date) const;
    bool hasDataFor(const QDate &date) const;

    bool load(QXmlStreamReader *reader);

    void addProgramme(TvProgramme *programme);
    bool trimProgrammes();

    QList<TvProgramme *> programmesForDay(const QDate &date) const;

private:
    TvChannelList *m_channelList;
    QString m_id;
    QString m_name;
    QStringList m_baseUrls;
    QList<QDate> m_dates;
    QList<QDateTime> m_modifiedTimes;
    TvProgramme *m_programmes;

    void addDataFor(const QDate &date, const QDateTime &lastModified);
};

#endif
