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

#include "tvbookmark.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qurl.h>
#include <QtGui/qicon.h>

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

    bool isHidden() const { return m_hidden; }
    void setHidden(bool value) { m_hidden = value; }

    bool convertTimezone() const { return m_convertTimezone; }
    void setConvertTimezone(bool value) { m_convertTimezone = value; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    int primaryChannelNumber() const { return m_primaryChannelNumber; }
    QStringList channelNumbers() const { return m_channelNumbers; }
    void addChannelNumber(const QString &number);

    QString iconUrl() const { return m_iconUrl; }
    void setIconUrl(const QString &url) { m_iconUrl = url; }

    QString iconFile() const { return m_iconFile; }
    void setIconFile(const QString &file) { m_iconFile = file; }

    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon &icon) { m_icon = icon; }

    QStringList baseUrls() const { return m_baseUrls; }
    QList<QUrl> dayUrls(const QDate &date) const;
    QDateTime dayLastModified(const QDate &date) const;
    bool hasDataFor(const QDate &date) const;
    bool hasDataFor() const { return !m_dates.isEmpty(); }

    void dataForRange(QDate *first, QDate *last);

    bool load(QXmlStreamReader *reader);

    void addProgramme(TvProgramme *programme);
    bool trimProgrammes();

    enum TimePeriod
    {
        Morning     = 0x0001,
        Afternoon   = 0x0002,
        Night       = 0x0004,
        LateNight   = 0x0008,
        AllPeriods  = 0xFFFF
    };
    Q_DECLARE_FLAGS(TimePeriods, TimePeriod)

    QList<TvProgramme *> programmesForDay
        (const QDate &date, TimePeriods periods,
         TvBookmark::MatchOptions options) const;

    QList<TvProgramme *> bookmarkedProgrammes
        (const QDate &first, const QDate &last,
         TvBookmark::MatchOptions options) const;

    static QDateTime stringToDateTime(const QString &str, TvChannel *channel = 0);
    static QDate stringToDate(const QString &str);

    int compare(const TvChannel *other) const;

private:
    TvChannelList *m_channelList;
    QString m_id;
    QString m_name;
    QString m_iconUrl;
    QString m_iconFile;
    QIcon m_icon;
    QStringList m_baseUrls;
    QList<QDate> m_dates;
    QList<QDateTime> m_modifiedTimes;
    TvProgramme *m_programmes;
    TvProgramme *m_lastInsert;
    QStringList m_channelNumbers;
    int m_primaryChannelNumber;
    bool m_hidden;
    bool m_convertTimezone;

    void addDataFor(const QDate &date, const QDateTime &lastModified);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TvChannel::TimePeriods)

#endif
