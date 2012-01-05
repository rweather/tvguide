/*
 * Copyright (C) 2011,2012  Southern Storm Software, Pty Ltd.
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

#ifndef _TVTICK_H
#define _TVTICK_H

#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qsettings.h>
#include <QtCore/qxmlstream.h>

class TvProgramme;

class TvTick
{
public:
    TvTick();
    ~TvTick();

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString channelId() const { return m_channelId; }
    void setChannelId(const QString &id) { m_channelId = id; }

    QDateTime start() const { return m_start; }
    void setStart(const QDateTime &start) { m_start = start; }

    QDateTime timestamp() const { return m_timestamp; }
    void setTimestamp(const QDateTime &timestamp) { m_timestamp = timestamp; }

    bool match(const TvProgramme *programme) const;

    void load(QSettings *settings);
    void save(QSettings *settings);

    void loadXml(QXmlStreamReader *reader);
    void saveXml(QXmlStreamWriter *writer);

private:
    QString m_title;
    QString m_channelId;
    QDateTime m_start;
    QDateTime m_timestamp;
};

#endif
