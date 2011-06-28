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

#include "tvchannel.h"
#include "tvchannellist.h"
#include "tvprogramme.h"
#include "tvbookmark.h"
#include <QtCore/qdebug.h>

TvChannel::TvChannel(TvChannelList *channelList)
    : m_channelList(channelList)
    , m_programmes(0)
    , m_hidden(false)
{
}

TvChannel::~TvChannel()
{
    TvProgramme *prog = m_programmes;
    TvProgramme *next;
    while (prog != 0) {
        next = prog->m_next;
        delete prog;
        prog = next;
    }
}

QList<QUrl> TvChannel::dayUrls(const QDate &date) const
{
    // http://www.oztivo.net/twiki/bin/view/TVGuide/StaticXMLGuideAPI
    //
    // URL has the form: baseUrl/channelid_YYYY-MM-DD.xml.gz
    //
    // If there is more than one baseUrl, then randomize the list
    // to help spread out the server load.
    QList<QUrl> urls;
    QString url;
    if (m_baseUrls.isEmpty() || m_id.isEmpty())
        return urls;
    for (int index = 0; index < m_baseUrls.size(); ++index) {
        url = m_baseUrls.at(index);
        if (!url.endsWith(QLatin1Char('/')))
            url += QLatin1Char('/');
        url += m_id;
        url += QLatin1Char('_');
        url += date.toString(QLatin1String("yyyy-MM-dd"));
        url += QLatin1String(".xml.gz");
        urls.append(QUrl(url));
    }
    if (urls.size() > 1) {
        for (int index = 1; index < urls.size(); ++index) {
            int other = qrand() % urls.size();
            if (index != other)
                qSwap(urls[index], urls[other]);
        }
    }
    return urls;
}

QDateTime TvChannel::dayLastModified(const QDate &date) const
{
    for (int index = 0; index < m_dates.size(); ++index) {
        if (m_dates.at(index) == date)
            return m_modifiedTimes.at(index);
    }
    return QDateTime();
}

void TvChannel::addDataFor(const QDate &date, const QDateTime &lastModified)
{
    for (int index = 0; index < m_dates.size(); ++index) {
        if (m_dates.at(index) == date) {
            m_modifiedTimes[index] = lastModified;
            return;
        }
    }
    m_dates.append(date);
    m_modifiedTimes.append(lastModified);
}

bool TvChannel::hasDataFor(const QDate &date) const
{
    // If there are no "datafor" declarations, then use +/- 2 weeks.
    if (m_dates.isEmpty()) {
        QDate current = QDate::currentDate();
        QDate start = current.addDays(-14);
        QDate stop = current.addDays(14);
        return date >= start && date <= stop;
    }
    for (int index = 0; index < m_dates.size(); ++index) {
        if (m_dates.at(index) == date)
            return true;
    }
    return false;
}

void TvChannel::dataForRange(QDate *first, QDate *last)
{
    // If there are no "datafor" declarations, then use +/- 2 weeks.
    if (m_dates.isEmpty()) {
        QDate current = QDate::currentDate();
        *first = current.addDays(-14);
        *last = current.addDays(14);
    } else {
        *first = *last = m_dates.at(0);
        for (int index = 1; index < m_dates.size(); ++index) {
            QDate date = m_dates.at(index);
            if (date < *first)
                *first = date;
            if (date > *last)
                *last = date;
        }
    }
}

bool TvChannel::load(QXmlStreamReader *reader)
{
    // Will leave the XML stream positioned on </channel>.
    Q_ASSERT(reader->isStartElement());
    Q_ASSERT(reader->name() == QLatin1String("channel"));
    bool changed = false;
    QStringList baseUrls;
    QList<QDate> dates(m_dates);
    QList<QDateTime> modifiedTimes(m_modifiedTimes);
    m_id = reader->attributes().value(QLatin1String("id")).toString();
    m_dates.clear();
    m_modifiedTimes.clear();
    while (!reader->hasError()) {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader->name() == QLatin1String("display-name")) {
                QString name = reader->readElementText
                    (QXmlStreamReader::SkipChildElements);
                if (m_name != name) {
                    m_name = name;
                    changed = true;
                }
            } else if (reader->name() == QLatin1String("base-url")) {
                baseUrls += reader->readElementText
                    (QXmlStreamReader::SkipChildElements);
            } else if (reader->name() == QLatin1String("datafor")) {
                QString lastmod = reader->attributes().value
                    (QLatin1String("lastmodified")).toString();
                QString date = reader->readElementText
                    (QXmlStreamReader::SkipChildElements);
                addDataFor(QDate::fromString(date, QLatin1String("yyyy-MM-dd")),
                           stringToDateTime(lastmod));
            } else if (reader->name() == QLatin1String("icon")) {
                QString src = reader->attributes().value
                    (QLatin1String("src")).toString();
                if (m_iconUrl != src) {
                    m_iconUrl = src;
                    changed = true;
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("channel"))
                break;
        }
    }
    QString iconFile = m_channelList->m_iconFiles.value(m_id, QString());
    if (m_iconFile != iconFile) {
        m_iconFile = iconFile;
        changed = true;
        if (!iconFile.isEmpty())
            m_icon = QIcon(iconFile);
        else
            m_icon = QIcon();
    }
    if (baseUrls != m_baseUrls) {
        m_baseUrls = baseUrls;
        changed = true;
    }
    if (m_dates != dates || m_modifiedTimes != modifiedTimes)
        changed = true;
    if (m_name.isEmpty()) {
        m_name = m_id;
        changed = true;
    }
    return changed;
}

void TvChannel::addProgramme(TvProgramme *programme)
{
    TvProgramme *prog = m_programmes;
    TvProgramme *prev = 0;
    TvProgramme *next;
    while (prog != 0) {
        next = prog->m_next;
        if (programme->start() >= prog->stop()) {
            // Haven't found the insertion point yet.
            prev = prog;
            prog = next;
        } else if (programme->stop() <= prog->start()) {
            // Insertion point found.
            break;
        } else {
            // Original programme overlaps with new one, so this
            // is probably an update for the same timeslot.
            // Remove the original entry and replace it.
            if (prev)
                prev->m_next = next;
            else
                m_programmes = next;
            delete prog;
            prog = next;
        }
    }
    programme->m_next = prog;
    if (prev)
        prev->m_next = programme;
    else
        m_programmes = programme;
}

// Trim any programmes that do not apply to dates in "datafor" decls.
bool TvChannel::trimProgrammes()
{
    // Get the list of dates that are covered by "datafor" decls.
    // If there are no "datafor" decls, then use +/- 2 weeks.
    QDate start;
    QDate stop;
    if (m_dates.isEmpty()) {
        QDate current = QDate::currentDate();
        start = current.addDays(-14);
        stop = current.addDays(14);
    } else {
        start = stop = m_dates[0];
        for (int index = 1; index < m_dates.size(); ++index) {
            QDate current = m_dates[index];
            if (current < start)
                start = current;
            if (current > stop)
                stop = current;
        }
    }

    // Trim the list of programmes outside the date range.
    TvProgramme *prog = m_programmes;
    TvProgramme *prev = 0;
    TvProgramme *next;
    bool changed = false;
    m_programmes = 0;
    while (prog != 0) {
        next = prog->m_next;
        if (start <= prog->stop().date() &&
                stop >= prog->start().date()) {
            if (prev)
                prev->m_next = prog;
            else
                m_programmes = prog;
            prog->m_next = 0;
            prev = prog;
        } else {
            delete prog;
            changed = true;
        }
        prog = next;
    }
    return changed;
}

QList<TvProgramme *> TvChannel::programmesForDay
    (const QDate &date, TimePeriods periods) const
{
    QList<TvProgramme *> list;
    TvProgramme *prog = m_programmes;
    QDateTime morningStart = QDateTime(date, QTime(6, 0, 0));
    QDateTime afternoonStart = QDateTime(date, QTime(12, 0, 0));
    QDateTime nightStart = QDateTime(date, QTime(18, 0, 0));
    QDateTime lateNightStart = QDateTime(date.addDays(1), QTime(0, 0, 0));
    QDateTime lateNightEnd = QDateTime(date.addDays(1), QTime(6, 0, 0));
    bool candidate;
    while (prog != 0) {
        candidate = false;
        if (periods & TvChannel::Morning) {
            if (prog->start() < afternoonStart &&
                    prog->stop() > morningStart)
                candidate = true;
        }
        if (periods & TvChannel::Afternoon) {
            if (prog->start() < nightStart &&
                    prog->stop() > afternoonStart)
                candidate = true;
        }
        if (periods & TvChannel::Night) {
            if (prog->start() < lateNightStart &&
                    prog->stop() > nightStart)
                candidate = true;
        }
        if (periods & TvChannel::LateNight) {
            if (prog->start() < lateNightEnd &&
                    prog->stop() > lateNightStart)
                candidate = true;
        }
        if (candidate) {
            TvBookmark *bookmark = 0;
            TvBookmark::Match match;
            match = channelList()->matchBookmarks(prog, &bookmark);
            if (match == TvBookmark::NoMatch)
                prog->setColor(QColor());
            else if (match == TvBookmark::FullMatch)
                prog->setColor(bookmark->color());
            else
                prog->setColor(bookmark->color().lighter(150));
            list.append(prog);
        }
        prog = prog->m_next;
    }
    return list;
}

// Find all bookmarked programmes within a specific date range.
QList<TvProgramme *> TvChannel::bookmarkedProgrammes
    (const QDate &first, const QDate &last) const
{
    QList<TvProgramme *> list;
    TvProgramme *prog = m_programmes;
    QDateTime startTime = QDateTime(first, QTime(6, 0, 0));
    QDateTime stopTime = QDateTime(last.addDays(1), QTime(6, 0, 0));
    while (prog != 0) {
        if ((prog->start() >= startTime &&
                    prog->start() < stopTime) ||
                (prog->stop() > startTime &&
                    prog->stop() <= stopTime)) {
            TvBookmark *bookmark = 0;
            TvBookmark::Match match;
            match = channelList()->matchBookmarks(prog, &bookmark);
            if (match != TvBookmark::NoMatch) {
                if (match == TvBookmark::FullMatch)
                    prog->setColor(bookmark->color());
                else
                    prog->setColor(bookmark->color().lighter(150));
                list.append(prog);
            }
        }
        prog = prog->m_next;
    }
    return list;
}

static int fetchField(const QString &str, int *posn, int size)
{
    int value = 0;
    while (size > 0 && *posn < str.length()) {
        uint ch = str.at((*posn)++).unicode();
        if (ch >= '0' && ch <= '9')
            value = value * 10 + int(ch - '0');
        --size;
    }
    return value;
}

// Format is "yyyyMMddhhmmss (+/-)zzzz" but isn't easily parseable
// by QDateTime::fromString().  So do it the long way.
QDateTime TvChannel::stringToDateTime(const QString &str)
{
    int posn = 0;
    int year = fetchField(str, &posn, 4);
    int month = fetchField(str, &posn, 2);
    int day = fetchField(str, &posn, 2);
    int hour = fetchField(str, &posn, 2);
    int min = fetchField(str, &posn, 2);
    int sec = fetchField(str, &posn, 2);
    int tz = 0;
    while (posn < str.length()) {
        uint ch = str.at(posn++).unicode();
        if (ch == '+') {
            tz = fetchField(str, &posn, 4);
            tz = (tz / 100) * 60 + (tz % 100);
            break;
        } else if (ch == '-') {
            tz = fetchField(str, &posn, 4);
            tz = -((tz / 100) * 60 + (tz % 100));
            break;
        } else if (ch != ' ') {
            break;
        }
    }
    QDateTime dt(QDate(year, month, day), QTime(hour, min, sec));
    dt.setUtcOffset(tz * 60);
    // Converting from a UTC offset to local must go via UTC.
    return dt.toTimeSpec(Qt::UTC).toTimeSpec(Qt::LocalTime);
}
