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

#include "tvbookmark.h"
#include "tvprogramme.h"
#include "tvchannel.h"
#include <QtCore/qdebug.h>

TvBookmark::TvBookmark()
    : m_dayOfWeek(TvBookmark::AnyDay)
    , m_dayOfWeekMask(0xFE)
    , m_enabled(true)
{
}

TvBookmark::TvBookmark(const TvBookmark &other)
    : m_title(other.title())
    , m_channelId(other.channelId())
    , m_dayOfWeek(other.dayOfWeek())
    , m_dayOfWeekMask(other.dayOfWeekMask())
    , m_enabled(other.isEnabled())
    , m_startTime(other.startTime())
    , m_stopTime(other.stopTime())
    , m_color(other.color())
{
}

TvBookmark::~TvBookmark()
{
}

static int const dayOfWeekMasks[] = {
    0xFE, // AnyDay
    0x02, // Monday
    0x04, // Tuesday
    0x08, // Wednesday
    0x10, // Thursday
    0x20, // Friday
    0x40, // Saturday
    0x80, // Sunday
    0x3E, // MondayToFriday
    0xC0, // SaturdayAndSunday
    0x00
};

void TvBookmark::setDayOfWeek(int day)
{
    if (day < 0 || day > Last)
        day = AnyDay;
    m_dayOfWeek = day;
    m_dayOfWeekMask = dayOfWeekMasks[day];
}

void TvBookmark::setDayOfWeekMask(int mask)
{
    int index;
    for (index = 0; dayOfWeekMasks[index] != 0; ++index) {
        if (dayOfWeekMasks[index] == mask) {
            m_dayOfWeek = index;
            m_dayOfWeekMask = mask;
            return;
        }
    }
    m_dayOfWeek = Mask;
    m_dayOfWeekMask = mask;
}

static QString dayOfWeekMaskName(int mask, bool longForm)
{
    QString name;
    int day = 1;
    int endDay;
    while (day <= 7) {
        if (mask & (1 << day)) {
            if (!name.isEmpty()) {
                if (longForm)
                    name += QObject::tr(" and ");
                else
                    name += QLatin1Char(',');
            }
            if (mask & (1 << (day + 1)) && mask & (1 << (day + 2))) {
                // At least three days in a row are combined into
                // Day1-DayN, instead of Day1,Day2,...,DayN.
                endDay = day + 2;
                while (mask & (1 << (endDay + 1)))
                    ++endDay;
                if (longForm) {
                    name += QDate::longDayName(day);
                    name += QObject::tr(" to ");
                    name += QDate::longDayName(endDay);
                } else {
                    name += QDate::shortDayName(day);
                    name += QLatin1Char('-');
                    name += QDate::shortDayName(endDay);
                }
                day = endDay + 1;
            } else {
                if (longForm)
                    name += QDate::longDayName(day);
                else
                    name += QDate::shortDayName(day);
                ++day;
            }
        } else {
            ++day;
        }
    }
    if (name.isEmpty())
        return QObject::tr("No day");
    else
        return name;
}

QString TvBookmark::dayOfWeekName() const
{
    if (m_dayOfWeek == TvBookmark::AnyDay)
        return QObject::tr("Any day");
    else if (m_dayOfWeek == TvBookmark::MondayToFriday)
        return QObject::tr("Mon-Fri");
    else if (m_dayOfWeek == TvBookmark::SaturdayAndSunday)
        return QObject::tr("Sat,Sun");
    else if (m_dayOfWeek == TvBookmark::Mask)
        return dayOfWeekMaskName(m_dayOfWeekMask, false);
    else
        return QDate::longDayName(m_dayOfWeek);
}

QString TvBookmark::dayOfWeekLongName(int mask)
{
    if (mask == 0xFE)
        return QObject::tr("Any day");
    else
        return dayOfWeekMaskName(mask, true);
}

TvBookmark::Match TvBookmark::match
    (const TvProgramme *programme, MatchOptions options) const
{
    TvBookmark::Match result = FullMatch;
    bool should = false;

    if (!m_enabled)
        return NoMatch;

    if (m_title.compare(programme->title(), Qt::CaseInsensitive) != 0) {
        if (!(options & NonMatching))
            return NoMatch;
        if (!m_channelId.isEmpty() &&
                m_channelId != programme->channel()->id())
            return NoMatch;
        should = true;
        result = ShouldMatch;
    } else {
        if (!m_channelId.isEmpty() &&
                m_channelId != programme->channel()->id())
            result = TitleMatch;
    }

    // Check that start and stop times are within the expected range.
    QTime start = programme->start().time();
    QTime stop = programme->stop().time();
    int dayOfWeekMask = m_dayOfWeekMask;
    if (m_startTime < m_stopTime) {
        if (start < m_startTime) {
            if (stop > m_startTime)
                result = Underrun;
            else
                result = TitleMatch;
        } else if (start >= m_stopTime) {
            result = TitleMatch;
        } else if (stop < m_startTime || stop > m_stopTime) {
            result = Overrun;
        }
    } else {
        if (start >= m_stopTime && start < m_startTime) {
            if (stop > m_startTime || stop <= m_stopTime)
                result = Underrun;
            else if (stop >= m_stopTime && stop < start)
                result = Underrun;
            else
                result = TitleMatch;
        } else if (start < m_stopTime) {
            // Adjust the expected weekday - start time is in tomorrow.
            // We do this by rotating the day mask left by one position.
            dayOfWeekMask = ((dayOfWeekMask << 1) |
                             (dayOfWeekMask >> 6)) & 0xFE;
            if (stop > m_stopTime)
                result = Overrun;
        } else {
            if (stop > m_stopTime && stop < m_startTime)
                result = Overrun;
        }
    }

    // Validate the weekday.
    int weekday = programme->start().date().dayOfWeek();
    if (!(dayOfWeekMask & (1 << weekday)))
        result = TitleMatch;

    // Disallow partial title matches if not allowed by the option list.
    if (!(options & PartialMatches) && result == TitleMatch)
        result = NoMatch;

    // Deal with non-matching bookmarks that cover the same timeslot.
    if (should && result != ShouldMatch)
        result = NoMatch;

    return result;
}

void TvBookmark::load(QSettings *settings)
{
    m_title = settings->value(QLatin1String("title")).toString();
    m_channelId = settings->value(QLatin1String("channelId")).toString();
    int dayOfWeek = settings->value(QLatin1String("dayOfWeek")).toInt();
    if (dayOfWeek >= 0x0100) {
        setDayOfWeekMask(dayOfWeek & 0xFE);
    } else if (dayOfWeek < AnyDay || dayOfWeek > Last) {
        m_dayOfWeek = AnyDay;
        m_dayOfWeekMask = 0xFE;
    } else {
        setDayOfWeek(dayOfWeek);
    }
    m_enabled = settings->value(QLatin1String("enabled"), true).toBool();
    m_startTime = QTime::fromString(settings->value(QLatin1String("startTime")).toString(), Qt::TextDate);
    m_stopTime = QTime::fromString(settings->value(QLatin1String("stopTime")).toString(), Qt::TextDate);
    m_color = QColor(settings->value(QLatin1String("color")).toString());
}

void TvBookmark::save(QSettings *settings)
{
    settings->setValue(QLatin1String("title"), m_title);
    settings->setValue(QLatin1String("channelId"), m_channelId);
    if (m_dayOfWeek == Mask)
        settings->setValue(QLatin1String("dayOfWeek"), m_dayOfWeekMask | 0x0100);
    else
        settings->setValue(QLatin1String("dayOfWeek"), m_dayOfWeek);
    settings->setValue(QLatin1String("enabled"), m_enabled);
    settings->setValue(QLatin1String("startTime"), m_startTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("stopTime"), m_stopTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("color"), m_color.name());
}
