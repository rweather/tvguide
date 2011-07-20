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

#ifndef _TVBOOKMARK_H
#define _TVBOOKMARK_H

#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qsettings.h>
#include <QtGui/qcolor.h>

class TvProgramme;

class TvBookmark
{
public:
    TvBookmark();
    explicit TvBookmark(const TvBookmark &other);
    ~TvBookmark();

    enum {
        AnyDay,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday,
        MondayToFriday,
        SaturdayAndSunday,
        Last = SaturdayAndSunday,
        Mask = 32
    };

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString channelId() const { return m_channelId; }
    void setChannelId(const QString &id) { m_channelId = id; }

    int dayOfWeek() const { return m_dayOfWeek; }
    void setDayOfWeek(int day);

    int dayOfWeekMask() const { return m_dayOfWeekMask; }
    void setDayOfWeekMask(int mask);

    QString dayOfWeekName() const;
    static QString dayOfWeekLongName(int mask);

    QTime startTime() const { return m_startTime; }
    void setStartTime(const QTime &time) { m_startTime = time; }

    QTime stopTime() const { return m_stopTime; }
    void setStopTime(const QTime &time) { m_stopTime = time; }

    QColor color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool value) { m_enabled = value; }

    enum Match
    {
        NoMatch,
        FullMatch,
        Overrun,        // Runs over the stop time
        Underrun,       // Starts before the start time
        TitleMatch,     // Matches only on the title
        ShouldMatch     // Should match but doesn't (NonMatching option)
    };

    enum MatchOption
    {
        PartialMatches      = 0x0001,
        NonMatching         = 0x0002,
        DefaultOptions      = PartialMatches
    };
    Q_DECLARE_FLAGS(MatchOptions, MatchOption)

    TvBookmark::Match match
        (const TvProgramme *programme, MatchOptions options) const;

    void load(QSettings *settings);
    void save(QSettings *settings);

private:
    QString m_title;
    QString m_channelId;
    int m_dayOfWeek;
    int m_dayOfWeekMask;
    bool m_enabled;
    QTime m_startTime;
    QTime m_stopTime;
    QColor m_color;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TvBookmark::MatchOptions)

#endif
