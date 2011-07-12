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
    , m_enabled(true)
{
}

TvBookmark::TvBookmark(const TvBookmark &other)
    : m_title(other.title())
    , m_channelId(other.channelId())
    , m_dayOfWeek(other.dayOfWeek())
    , m_enabled(other.isEnabled())
    , m_startTime(other.startTime())
    , m_stopTime(other.stopTime())
    , m_color(other.color())
{
}

TvBookmark::~TvBookmark()
{
}

TvBookmark::Match TvBookmark::match(const TvProgramme *programme) const
{
    TvBookmark::Match result = FullMatch;

    if (!m_enabled)
        return NoMatch;

    if (m_title.compare(programme->title(), Qt::CaseInsensitive) != 0)
        return NoMatch;

    if (!m_channelId.isEmpty() &&
            m_channelId != programme->channel()->id())
        result = TitleMatch;

    // Check that start and stop times are within the expected range.
    QTime start = programme->start().time();
    QTime stop = programme->stop().time();
    int dayOfWeek = m_dayOfWeek;
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
            if (stop <= m_stopTime || stop > m_startTime)
                result = Underrun;
            else
                result = TitleMatch;
        } else if (start < m_stopTime) {
            // Adjust the expected weekday - start time is in tomorrow.
            if (dayOfWeek == Sunday)
                dayOfWeek = Monday;
            else if (dayOfWeek >= Monday && dayOfWeek <= Saturday)
                ++dayOfWeek;
            if (stop > m_stopTime)
                result = Overrun;
        } else {
            if (stop > m_stopTime && stop < m_startTime)
                result = Overrun;
        }
    }

    // Validate the weekday.
    int weekday = programme->start().date().dayOfWeek();
    if (dayOfWeek == MondayToFriday) {
        if (weekday == Saturday || weekday == Sunday)
            result = TitleMatch;
    } else if (dayOfWeek != AnyDay) {
        if (weekday != dayOfWeek)
            result = TitleMatch;
    }

    return result;
}

void TvBookmark::load(QSettings *settings)
{
    m_title = settings->value(QLatin1String("title")).toString();
    m_channelId = settings->value(QLatin1String("channelId")).toString();
    m_dayOfWeek = settings->value(QLatin1String("dayOfWeek")).toInt();
    if (m_dayOfWeek < AnyDay || m_dayOfWeek > MondayToFriday)
        m_dayOfWeek = AnyDay;
    m_enabled = settings->value(QLatin1String("enabled"), true).toBool();
    m_startTime = QTime::fromString(settings->value(QLatin1String("startTime")).toString(), Qt::TextDate);
    m_stopTime = QTime::fromString(settings->value(QLatin1String("stopTime")).toString(), Qt::TextDate);
    m_color = QColor(settings->value(QLatin1String("color")).toString());
}

void TvBookmark::save(QSettings *settings)
{
    settings->setValue(QLatin1String("title"), m_title);
    settings->setValue(QLatin1String("channelId"), m_channelId);
    settings->setValue(QLatin1String("dayOfWeek"), m_dayOfWeek);
    settings->setValue(QLatin1String("enabled"), m_enabled);
    settings->setValue(QLatin1String("startTime"), m_startTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("stopTime"), m_stopTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("color"), m_color.name());
}
