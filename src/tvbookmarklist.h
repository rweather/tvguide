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

#ifndef _TVBOOKMARKLIST_H
#define _TVBOOKMARKLIST_H

#include "tvbookmark.h"
#include "tvtick.h"
#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qsettings.h>

class TvChannelList;

class TvBookmarkList : public QObject
{
    Q_OBJECT
public:
    TvBookmarkList(TvChannelList *channelList, QObject *parent = 0);
    ~TvBookmarkList();

    void clearBookmarks();
    void clearTicks();

    void addBookmark(TvBookmark *bookmark, bool notify = true);
    void removeBookmark(TvBookmark *bookmark, bool notify = true);

    QList<TvBookmark *> bookmarks() const { return m_bookmarks; }
    void replaceBookmarks(const QList<TvBookmark *> &bookmarks);

    void addTick(const TvProgramme *programme);
    void removeTick(const TvProgramme *programme);

    void loadTicks(QSettings *settings);
    void saveTicks(QSettings *settings);

    QList<TvBookmark *> candidates(const TvProgramme *programme) const;

    TvBookmark::Match match
        (const TvProgramme *programme, TvBookmark **bookmark,
         TvBookmark::MatchOptions options) const;

    enum ImportResult
    {
        Import_OK,
        Import_NothingNew,
        Import_CannotOpen,
        Import_BadFormat,
        Import_WrongService
    };

    void exportBookmarks(const QString &filename);
    ImportResult importBookmarks(const QString &filename);

Q_SIGNALS:
    void bookmarksChanged();
    void ticksChanged();

private:
    static const int TimeSlotsPerDay = 24 * 2;
    static const int TimeSlots = 7 * TimeSlotsPerDay;

    TvChannelList *m_channelList;
    QList<TvBookmark *> m_bookmarks;
    QHash< QString, QList<TvBookmark *> > m_titleIndex;
    QList<TvBookmark *> m_timeIndex[TimeSlots];
    QMultiMap<QString, TvTick *> m_ticks;

    void adjustTimeIndex(TvBookmark *bookmark, bool add);
    bool importBookmark(TvBookmark *bookmark);
    bool importTick(TvTick *tick);
    QString convertChannelId(const QString &id);
};

#endif
