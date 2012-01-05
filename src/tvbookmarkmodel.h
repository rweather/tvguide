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

#ifndef _TVBOOKMARKMODEL_H
#define _TVBOOKMARKMODEL_H

#include "tvbookmark.h"
#include <QtCore/qabstractitemmodel.h>

class TvChannelList;

class TvBookmarkModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TvBookmarkModel(TvChannelList *channelList, QObject *parent = 0);
    ~TvBookmarkModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void addBookmark(TvBookmark *bookmark);
    void updateBookmark(int index);
    void removeBookmark(int index);
    QList<TvBookmark *> detachBookmarks();

    TvBookmark *bookmarkAt(int index) const { return m_bookmarks.at(index); }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    TvChannelList *m_channelList;
    QList<TvBookmark *> m_bookmarks;
};

#endif
