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
    , m_onair(true)
    , m_anyTime(false)
{
}

TvBookmark::TvBookmark(const TvBookmark &other)
    : m_title(other.title())
    , m_channelId(other.channelId())
    , m_dayOfWeek(other.dayOfWeek())
    , m_dayOfWeekMask(other.dayOfWeekMask())
    , m_onair(other.isOnAir())
    , m_anyTime(other.anyTime())
    , m_startTime(other.startTime())
    , m_stopTime(other.stopTime())
    , m_color(other.color())
    , m_seasons(other.seasonList())
    , m_years(other.yearList())
{
}

TvBookmark::~TvBookmark()
{
    for (int index = 0; index < m_matchingProgrammes.size(); ++index) {
        TvProgramme *programme = m_matchingProgrammes.at(index);
        programme->clearBookmarkMatch();
    }
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

static QString seasonsToString(const QList< QPair<int, int> > &seasons)
{
    QString result;
    for (int index = 0; index < seasons.size(); ++index) {
        int first = seasons.at(index).first;
        int last = seasons.at(index).second;
        if (index != 0)
            result += QLatin1Char(',');
        if (first == last) {
            result += QString::number(first);
        } else if (last == 0x7fffffff) {
            result += QString::number(first);
            result += QLatin1Char('+');
        } else {
            result += QString::number(first);
            result += QLatin1Char('-');
            result += QString::number(last);
        }
    }
    return result;
}

QString TvBookmark::seasons() const
{
    return seasonsToString(m_seasons);
}

void TvBookmark::setSeasons(const QString &seasons)
{
    m_seasons = parseSeasons(seasons, 0);
}

static const int ST_End     = -1;
static const int ST_Error   = -2;
static const int ST_Dash    = -3;
static const int ST_Plus    = -4;
static const int ST_Comma   = -5;

static int seasonToken(const QString &seasons, int *index)
{
    while (*index < seasons.length()) {
        int ch = seasons.at((*index)++).unicode();
        if (ch >= '0' && ch <= '9') {
            int number = ch - '0';
            while (*index < seasons.length()) {
                ch = seasons.at(*index).unicode();
                if (ch >= '0' && ch <= '9') {
                    number = number * 10 + ch - '0';
                    ++(*index);
                } else {
                    break;
                }
            }
            return number;
        } else if (ch == '-') {
            return ST_Dash;
        } else if (ch == '+') {
            return ST_Plus;
        } else if (ch == ',') {
            return ST_Comma;
        } else if (ch != ' ' && ch != '\t') {
            return ST_Error;
        }
    }
    return ST_End;
}

QList< QPair<int, int> > TvBookmark::parseSeasons(const QString &seasons, bool *ok)
{
    QList< QPair<int, int> > list;
    int index = 0;
    int token = seasonToken(seasons, &index);
    while (token != ST_End) {
        if (token > 0) {
            int first = token;
            token = seasonToken(seasons, &index);
            if (token == ST_Dash) {
                token = seasonToken(seasons, &index);
                if (token >= 0 && token >= first) {
                    list.append(QPair<int, int>(first, token));
                    token = seasonToken(seasons, &index);
                } else {
                    token = ST_Error;
                    break;
                }
            } else if (token == ST_Plus) {
                list.append(QPair<int, int>(first, 0x7fffffff));
                token = seasonToken(seasons, &index);
            } else {
                list.append(QPair<int, int>(first, first));
            }
        } else if (token != ST_Comma) {
            break;
        } else {
            token = seasonToken(seasons, &index);
        }
    }
    if (token != ST_End)
        list.clear();
    if (ok)
        *ok = (token == ST_End);
    return list;
}

QString TvBookmark::years() const
{
    return seasonsToString(m_years);
}

void TvBookmark::setYears(const QString &years)
{
    m_years = parseSeasons(years, 0);
}

TvBookmark::Match TvBookmark::match
    (const TvProgramme *programme, MatchOptions options) const
{
    TvBookmark::Match result = FullMatch;
    bool should = false;

    if (m_title.compare(programme->title(), Qt::CaseInsensitive) != 0) {
        if (!(options & NonMatching))
            return NoMatch;
        if (!m_channelId.isEmpty() &&
                !programme->channel()->isSameChannel(m_channelId))
            return NoMatch;
        should = true;
        result = ShouldMatch;
    } else {
        if (!m_channelId.isEmpty() &&
                !programme->channel()->isSameChannel(m_channelId))
            result = TitleMatch;
    }

    // Check that start and stop times are within the expected range.
    QTime start = programme->start().time();
    QTime stop = programme->stop().time();
    int dayOfWeekMask = m_dayOfWeekMask;
    if (m_anyTime) {
        // If we are matching at any time of day, then don't show
        // failed matches.  Otherwise everything will show as failed!
        if (result == ShouldMatch)
            return NoMatch;
    } else if (m_startTime < m_stopTime) {
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
    if (should && (result == Underrun || result == Overrun))
        result = ShouldMatch;
    if (should && result != ShouldMatch)
        result = NoMatch;

    // Off-air bookmarks don't show failed matches.
    if (!m_onair && result == ShouldMatch)
        result = NoMatch;

    // Match the season number.
    if (!m_seasons.isEmpty() && result != ShouldMatch &&
            result != NoMatch) {
        int season = programme->season();
        int index;
        if (season) {
            for (index = 0; index < m_seasons.size(); ++index) {
                if (season >= m_seasons.at(index).first &&
                        season <= m_seasons.at(index).second)
                    break;
            }
            if (index >= m_seasons.size())
                result = NoMatch;
        } else {
            // If the programme does not have a season, then match
            // it against a bookmark with N+ as one of the ranges.
            // Usually the programme does not have a season number
            // because it is a new episode in the most recent season
            // and the upstream XMLTV database doesn't have a season
            // and episode number for it yet.
            for (index = 0; index < m_seasons.size(); ++index) {
                if (m_seasons.at(index).second == 0x7fffffff)
                    break;
            }
            if (index >= m_seasons.size())
                result = NoMatch;
        }
    }

    // Match the year number.
    if (!m_years.isEmpty() && result != ShouldMatch &&
            result != NoMatch) {
        int year = programme->year();
        int index;
        if (year) {
            for (index = 0; index < m_years.size(); ++index) {
                if (year >= m_years.at(index).first &&
                        year <= m_years.at(index).second)
                    break;
            }
            if (index >= m_years.size())
                result = NoMatch;
        } else {
            // If the programme does not have a year, then match
            // it against a bookmark with N+ as one of the ranges.
            for (index = 0; index < m_years.size(); ++index) {
                if (m_years.at(index).second == 0x7fffffff)
                    break;
            }
            if (index >= m_years.size())
                result = NoMatch;
        }
    }

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
    m_anyTime = settings->value(QLatin1String("anyTime"), false).toBool();
    m_startTime = QTime::fromString(settings->value(QLatin1String("startTime")).toString(), Qt::TextDate);
    m_stopTime = QTime::fromString(settings->value(QLatin1String("stopTime")).toString(), Qt::TextDate);
    m_color = QColor(settings->value(QLatin1String("color")).toString());
    setSeasons(settings->value(QLatin1String("seasons")).toString());
    setYears(settings->value(QLatin1String("years")).toString());
    m_onair = settings->value(QLatin1String("onair"), true).toBool();
}

void TvBookmark::save(QSettings *settings)
{
    settings->setValue(QLatin1String("title"), m_title);
    settings->setValue(QLatin1String("channelId"), m_channelId);
    if (m_dayOfWeek == Mask)
        settings->setValue(QLatin1String("dayOfWeek"), m_dayOfWeekMask | 0x0100);
    else
        settings->setValue(QLatin1String("dayOfWeek"), m_dayOfWeek);
    settings->remove(QLatin1String("enabled")); // Obsolete
    settings->setValue(QLatin1String("anyTime"), m_anyTime);
    settings->setValue(QLatin1String("startTime"), m_startTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("stopTime"), m_stopTime.toString(Qt::TextDate));
    settings->setValue(QLatin1String("color"), m_color.name());
    settings->setValue(QLatin1String("seasons"), seasons());
    settings->setValue(QLatin1String("years"), years());
    settings->setValue(QLatin1String("onair"), m_onair);
}

void TvBookmark::loadXml(QXmlStreamReader *reader)
{
    Q_UNUSED(reader);
}

void TvBookmark::saveXml(QXmlStreamWriter *writer)
{
    writer->writeStartElement(QLatin1String("bookmark"));
    writer->writeTextElement(QLatin1String("title"), m_title);
    if (!m_channelId.isEmpty())
        writer->writeTextElement(QLatin1String("channel-id"), m_channelId);
    writer->writeTextElement(QLatin1String("days"), QString::number(dayOfWeekMask()));
    writer->writeTextElement(QLatin1String("start-time"), m_startTime.toString(QLatin1String("HH:mm")));
    writer->writeTextElement(QLatin1String("stop-time"), m_stopTime.toString(QLatin1String("HH:mm")));
    if (m_anyTime)
        writer->writeEmptyElement(QLatin1String("any-time"));
    if (!m_onair)
        writer->writeEmptyElement(QLatin1String("off-air"));
    writer->writeTextElement(QLatin1String("color"), m_color.name());
    if (!m_seasons.isEmpty())
        writer->writeTextElement(QLatin1String("seasons"), seasons());
    if (!m_years.isEmpty())
        writer->writeTextElement(QLatin1String("years"), years());
    writer->writeEndElement();
}

void TvBookmark::addProgramme(TvProgramme *programme)
{
    for (int index = 0; index < m_matchingProgrammes.size(); ++index) {
        TvProgramme *prog = m_matchingProgrammes.at(index);
        prog->markDirty();
    }
    m_matchingProgrammes.append(programme);
}

void TvBookmark::removeProgramme(TvProgramme *programme)
{
    m_matchingProgrammes.removeOne(programme);
    for (int index = 0; index < m_matchingProgrammes.size(); ++index) {
        TvProgramme *prog = m_matchingProgrammes.at(index);
        prog->markDirty();
    }
}
