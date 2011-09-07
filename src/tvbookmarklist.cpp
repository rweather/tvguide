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

#include "tvbookmarklist.h"
#include "tvprogramme.h"
#include "tvchannel.h"

TvBookmarkList::TvBookmarkList(QObject *parent)
    : QObject(parent)
{
}

TvBookmarkList::~TvBookmarkList()
{
    clearBookmarks();
    clearTicks();
}

void TvBookmarkList::clearBookmarks()
{
    m_titleIndex.clear();
    for (int index = 0; index < TimeSlots; ++index)
        m_timeIndex[index].clear();
    qDeleteAll(m_bookmarks);
    m_bookmarks.clear();
}

void TvBookmarkList::clearTicks()
{
    qDeleteAll(m_ticks);
    m_ticks.clear();
}

void TvBookmarkList::addBookmark(TvBookmark *bookmark, bool notify)
{
    m_bookmarks.append(bookmark);

    m_titleIndex[bookmark->indexTitle()].append(bookmark);

    adjustTimeIndex(bookmark, true);

    if (notify)
        emit bookmarksChanged();
}

void TvBookmarkList::removeBookmark(TvBookmark *bookmark, bool notify)
{
    m_bookmarks.removeOne(bookmark);

    m_titleIndex[bookmark->indexTitle()].removeOne(bookmark);

    adjustTimeIndex(bookmark, false);

    if (notify)
        emit bookmarksChanged();
}

void TvBookmarkList::replaceBookmarks(const QList<TvBookmark *> &bookmarks)
{
    clearBookmarks();

    for (int index = 0; index < bookmarks.size(); ++index)
        addBookmark(bookmarks.at(index), false);

    emit bookmarksChanged();
}

void TvBookmarkList::addTick(const TvProgramme *programme)
{
    TvTick *tick = new TvTick();
    tick->setTitle(programme->title());
    tick->setChannelId(programme->channel()->id());
    tick->setStart(programme->start());
    tick->setTimestamp(QDateTime::currentDateTime());
    m_ticks.insert(tick->title(), tick);
    emit ticksChanged();
}

void TvBookmarkList::removeTick(const TvProgramme *programme)
{
    QMultiMap<QString, TvTick *>::Iterator it;
    it = m_ticks.find(programme->title());
    while (it != m_ticks.end()) {
        TvTick *tick = it.value();
        if (tick->start() == programme->start() &&
                tick->channelId() == programme->channel()->id()) {
            m_ticks.erase(it);
            break;
        }
        ++it;
    }
    emit ticksChanged();
}

void TvBookmarkList::loadTicks(QSettings *settings)
{
    clearTicks();
    int size = settings->beginReadArray(QLatin1String("ticks"));
    QDateTime expiry = QDateTime::currentDateTime().addDays(-30);
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        TvTick *tick = new TvTick();
        tick->load(settings);
        if (tick->timestamp() < expiry)
            delete tick;
        else
            m_ticks.insert(tick->title(), tick);
    }
    settings->endArray();
}

void TvBookmarkList::saveTicks(QSettings *settings)
{
    settings->beginWriteArray(QLatin1String("ticks"));
    int index = 0;
    QMultiMap<QString, TvTick *>::ConstIterator it;
    for (it = m_ticks.constBegin(); it != m_ticks.constEnd(); ++it) {
        settings->setArrayIndex(index++);
        it.value()->save(settings);
    }
    settings->endArray();
}

static inline int indexFromDateTime(const QDateTime &dt)
{
    return (dt.date().dayOfWeek() - 1) * 24 * 2 +
           dt.time().hour() * 2 +
           dt.time().minute() / 30;
}

static inline void mergeBookmarkLists
    (QList<TvBookmark *> &list, const QList<TvBookmark *> &list2)
{
    for (int index = 0; index < list2.size(); ++index) {
        TvBookmark *bookmark = list2.at(index);
        if (!list.contains(bookmark))
            list.append(bookmark);
    }
}

QList<TvBookmark *> TvBookmarkList::candidates(const TvProgramme *programme) const
{
    QList<TvBookmark *> list;

    // Find all bookmarks that have an exact title match.
    QHash< QString, QList<TvBookmark *> >::ConstIterator it;
    it = m_titleIndex.constFind(programme->indexTitle());
    if (it != m_titleIndex.constEnd())
        list = it.value();

    // Scan the programme's time range to find bookmarks that
    // may give a "failed match" (same time, different title).
    QHash< int, QList<TvBookmark *> >::ConstIterator it2;
    QString channel = programme->channel()->id();
    int start = indexFromDateTime(programme->start());
    int stop = indexFromDateTime(programme->stop());
    if (stop < start)
        stop += TimeSlots;
    for (int timeslot = start; timeslot < stop; ++timeslot) {
        int key = timeslot;
        if (key >= TimeSlots)
            key = TimeSlots;
        mergeBookmarkLists(list, m_timeIndex[key]);
    }

    // Return the candidate list.
    return list;
}

TvBookmark::Match TvBookmarkList::match
    (const TvProgramme *programme, TvBookmark **bookmark,
     TvBookmark::MatchOptions options) const
{
    TvBookmark::Match result = TvBookmark::NoMatch;

    // Check the list of ticked programmes first as ticking takes
    // precedence over bookmark matching.
    QMultiMap<QString, TvTick *>::ConstIterator tickit;
    tickit = m_ticks.constFind(programme->title());
    while (tickit != m_ticks.constEnd()) {
        const TvTick *tick = tickit.value();
        if (tick->match(programme)) {
            const_cast<TvProgramme *>(programme)->setTicked(true);
            *bookmark = 0;
            result = TvBookmark::TickMatch;
            break;
        }
        ++tickit;
    }

    // Look for a bookmark match.
    QList<TvBookmark *> candidates = this->candidates(programme);
    QList<TvBookmark *>::ConstIterator it = candidates.constBegin();
    while (it != candidates.constEnd()) {
        TvBookmark::Match match = (*it)->match(programme, options);
        if (match != TvBookmark::NoMatch) {
            if (match == TvBookmark::ShouldMatch) {
                if (result != TvBookmark::TitleMatch) {
                    *bookmark = *it;
                    result = TvBookmark::ShouldMatch;
                }
            } else if (match != TvBookmark::TitleMatch) {
                *bookmark = *it;
                return match;
            } else {
                *bookmark = *it;
                result = TvBookmark::TitleMatch;
            }
        }
        ++it;
    }
    return result;
}

// The time index is used to map a specific day and time to a bookmark
// that will probably match that timeslot.  This is used for quickly
// looking up failed matches where the title is different.  The key
// is a combination of the day number and a value corresponding to a
// 30 minute timeslot that overlaps the bookmark.
void TvBookmarkList::adjustTimeIndex(TvBookmark *bookmark, bool add)
{
    if (bookmark->anyTime())
        return;     // Any time bookmarks never give failed matches.
    QString channel = bookmark->channelId();
    int mask = bookmark->dayOfWeekMask();
    for (int day = 1; day <= 7; ++day) {
        if ((mask & (1 << day)) == 0)
            continue;
        int start = bookmark->startTime().hour() * 2 +
                    bookmark->startTime().minute() / 30;
        int stop = bookmark->stopTime().hour() * 2 +
                   bookmark->stopTime().minute() / 30;
        if (stop < start)
            stop += TimeSlotsPerDay;
        start += (day - 1) * TimeSlotsPerDay;
        stop += (day - 1) * TimeSlotsPerDay;
        for (int timeslot = start; timeslot < stop; ++timeslot) {
            int key = timeslot;
            if (key >= TimeSlots)
                key = TimeSlots;
            if (add)
                m_timeIndex[key].append(bookmark);
            else
                m_timeIndex[key].removeOne(bookmark);
        }
    }
}
