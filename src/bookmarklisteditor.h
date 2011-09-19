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

#ifndef _BOOKMARKLISTEDITOR_H
#define _BOOKMARKLISTEDITOR_H

#include <QtGui/qdialog.h>
#include <QtGui/qsortfilterproxymodel.h>
#include "ui_bookmarklisteditor.h"
#include "tvbookmarkmodel.h"

class TvChannelList;

class BookmarkListSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    BookmarkListSortModel(TvBookmarkModel *source, QObject *parent)
        : QSortFilterProxyModel(parent), m_source(source)
    {
        setSourceModel(source);
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        return m_source->lessThan(left, right);
    }

private:
    TvBookmarkModel *m_source;
};

class BookmarkListEditor : public QDialog, private Ui::BookmarkListEditor
{
    Q_OBJECT
public:
    explicit BookmarkListEditor(TvChannelList *channelList, QWidget *parent = 0);
    ~BookmarkListEditor();

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void editBookmark();
    void deleteBookmark();
    void newBookmark();
    void currentChanged(const QModelIndex &index);
    void help();

private:
    TvChannelList *m_channelList;
    TvBookmarkModel *m_model;
    QSortFilterProxyModel *m_proxy;
};

#endif
