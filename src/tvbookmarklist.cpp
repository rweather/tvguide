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

#include "tvbookmarklist.h"
#include "tvprogramme.h"
#include "tvchannel.h"
#include "tvchannellist.h"
#include <QtCore/qfile.h>
#include <QtCore/qxmlstream.h>

TvBookmarkList::TvBookmarkList(TvChannelList *channelList, QObject *parent)
    : QObject(parent)
    , m_channelList(channelList)
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

    delete bookmark;

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
            key -= TimeSlots;
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
            *bookmark = 0;
            return TvBookmark::TickMatch;
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

void TvBookmarkList::exportBookmarks(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument(QLatin1String("1.0"), true);
    writer.writeStartElement(QLatin1String("bookmarks"));
    writer.writeAttribute(QLatin1String("service"), m_channelList->startUrl().toString());
    for (int index = 0; index < m_bookmarks.size(); ++index)
        m_bookmarks.at(index)->saveXml(&writer);
    QMultiMap<QString, TvTick *>::ConstIterator it;
    for (it = m_ticks.constBegin(); it != m_ticks.constEnd(); ++it)
        it.value()->saveXml(&writer);
    writer.writeEndElement();
    writer.writeEndDocument();
    file.close();
}

TvBookmarkList::ImportResult TvBookmarkList::importBookmarks(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return Import_CannotOpen;
    QXmlStreamReader reader(&file);
    bool newBookmarks = false;
    bool newTicks = false;
    while (!reader.hasError() && !reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("bookmarks")) {
                QString service = reader.attributes().value(QLatin1String("service")).toString();
                QString expected = m_channelList->startUrl().toString();
                if (service != expected)
                    return Import_WrongService;
            } else if (reader.name() == QLatin1String("bookmark")) {
                TvBookmark *bookmark = new TvBookmark();
                bookmark->loadXml(&reader);
                if (importBookmark(bookmark))
                    newBookmarks = true;
                else
                    delete bookmark;
            } else if (reader.name() == QLatin1String("tick")) {
                TvTick *tick = new TvTick();
                tick->loadXml(&reader);
                if (importTick(tick))
                    newTicks = true;
                else
                    delete tick;
            }
        }
    }
    if (newBookmarks)
        emit bookmarksChanged();
    if (newTicks)
        emit ticksChanged();
    if (reader.hasError())
        return Import_BadFormat;
    else if (!newBookmarks && !newTicks)
        return Import_NothingNew;
    else
        return Import_OK;
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
                key -= TimeSlots;
            if (add)
                m_timeIndex[key].append(bookmark);
            else
                m_timeIndex[key].removeOne(bookmark);
        }
    }
}

bool TvBookmarkList::importBookmark(TvBookmark *bookmark)
{
    // Search for an existing bookmark with the same parameters.
    QList<TvBookmark *> list;
    QHash< QString, QList<TvBookmark *> >::ConstIterator it;
    it = m_titleIndex.constFind(bookmark->title().toLower());
    if (it != m_titleIndex.constEnd()) {
        list = it.value();
        for (int index = 0; index < list.size(); ++index) {
            // Everything except the color, channel id, and on-air flag needs
            // to be the same for the two bookmarks to be considered identical.
            // The channel id must be the same or a regional variant.
            TvBookmark *oldBookmark = list.at(index);
            if (oldBookmark->channelId() != bookmark->channelId()) {
                TvChannel *channel = m_channelList->channel(oldBookmark->channelId());
                if (!channel)
                    continue;
                if (!channel->isSameChannel(bookmark->channelId()))
                    continue;
            }
            if (oldBookmark->dayOfWeekMask() != bookmark->dayOfWeekMask())
                continue;
            if (oldBookmark->startTime() != bookmark->startTime())
                continue;
            if (oldBookmark->stopTime() != bookmark->stopTime())
                continue;
            if (oldBookmark->anyTime() != bookmark->anyTime())
                continue;
            if (oldBookmark->seasonList() != bookmark->seasonList())
                continue;
            if (oldBookmark->yearList() != bookmark->yearList())
                continue;
            return false;
        }
    }

    // Add the new bookmark.
    bookmark->setChannelId(convertChannelId(bookmark->channelId()));
    addBookmark(bookmark, false);
    return true;
}

bool TvBookmarkList::importTick(TvTick *tick)
{
    QMultiMap<QString, TvTick *>::ConstIterator tickit;
    tickit = m_ticks.constFind(tick->title());
    while (tickit != m_ticks.constEnd()) {
        const TvTick *oldTick = tickit.value();
        if (tick->channelId() == oldTick->channelId() &&
                tick->start() == oldTick->start()) {
            // We alread have this tick object.
            return false;
        }
        ++tickit;
    }
    tick->setChannelId(convertChannelId(tick->channelId()));
    m_ticks.insert(tick->title(), tick);
    return true;
}

// If the incoming channel id is in a different region, then convert
// it into its equivalent in the current region, if possible.
QString TvBookmarkList::convertChannelId(const QString &id)
{
    QString channelId(id);
    TvChannel *channel = m_channelList->channel(channelId);
    if (channel && channel->isHidden() && !channel->commonId().isEmpty()) {
        QList<TvChannel *> active = m_channelList->activeChannels();
        for (int index = 0; index < active.size(); ++index) {
            TvChannel *other = active.at(index);
            if (other->commonId() == channel->commonId() && !other->isHidden()) {
                channelId = other->id();
                break;
            }
        }
    }
    return channelId;
}
