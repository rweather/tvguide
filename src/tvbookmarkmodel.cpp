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

#include "tvbookmarkmodel.h"
#include "tvchannellist.h"
#include <QtGui/qbrush.h>
#include <QtGui/qpalette.h>
#include <QtGui/qwidget.h>
#include <QtCore/qdebug.h>

TvBookmarkModel::TvBookmarkModel(TvChannelList *channelList, QObject *parent)
    : QAbstractItemModel(parent)
    , m_channelList(channelList)
{
    QList<TvBookmark *> bookmarks = channelList->bookmarkList()->bookmarks();
    for (int index = 0; index < bookmarks.size(); ++index)
        m_bookmarks.append(new TvBookmark(*bookmarks.at(index)));
}

TvBookmarkModel::~TvBookmarkModel()
{
    qDeleteAll(m_bookmarks);
}

#define MODEL_NUM_COLS      6
#define MODEL_CHECK         0
#define MODEL_DAY           1
#define MODEL_START_TIME    2
#define MODEL_STOP_TIME     3
#define MODEL_CHANNEL       4
#define MODEL_TITLE         5

QModelIndex TvBookmarkModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || row >= m_bookmarks.size())
        return QModelIndex();
    if (column < 0 || column >= MODEL_NUM_COLS)
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex TvBookmarkModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvBookmarkModel::columnCount(const QModelIndex &) const
{
    return MODEL_NUM_COLS;
}

int TvBookmarkModel::rowCount(const QModelIndex &) const
{
    return m_bookmarks.size();
}

QVariant TvBookmarkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row = index.row();
    if (row < 0 || row >= m_bookmarks.size())
        return QVariant();
    TvBookmark *bookmark = m_bookmarks.at(row);
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case MODEL_DAY:
            return bookmark->dayOfWeekName();
        case MODEL_START_TIME:
            if (bookmark->anyTime())
                return tr("Any time");
            return bookmark->startTime().toString(Qt::LocaleDate);
        case MODEL_STOP_TIME:
            if (bookmark->anyTime())
                return QString();
            return bookmark->stopTime().toString(Qt::LocaleDate);
        case MODEL_CHANNEL: {
            QString id = bookmark->channelId();
            if (id.isEmpty()) {
                return tr("Any channel");
            } else {
                TvChannel *channel = m_channelList->channel(id);
                if (channel)
                    return channel->name();
                else
                    return id;
            }
            break; }
        case MODEL_TITLE: {
            QString title = bookmark->title();
            if (!bookmark->seasonList().isEmpty()) {
                title = QObject::tr("%1, Season %2")
                            .arg(title).arg(bookmark->seasons());
            }
            return title; }
        default: break;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == MODEL_TITLE)
            return QBrush(bookmark->color());
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == MODEL_CHECK) {
            if (bookmark->isOnAir())
                return int(Qt::Checked);
            else
                return int(Qt::Unchecked);
        }
    } else if (role == Qt::ToolTipRole) {
        if (index.column() == MODEL_CHECK) {
            if (bookmark->isOnAir())
                return tr("On Air");
            else
                return tr("Off Air");
        }
    }
    return QVariant();
}

bool TvBookmarkModel::setData
    (const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;
    if (!index.isValid())
        return false;
    int row = index.row();
    if (row < 0 || row >= m_bookmarks.size())
        return false;
    TvBookmark *bookmark = m_bookmarks.at(row);
    bookmark->setOnAir(value.toInt() == int(Qt::Checked));
    return true;
}

QVariant TvBookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case MODEL_CHECK:           return QString();
        case MODEL_DAY:             return tr("Day");
        case MODEL_START_TIME:      return tr("Start");
        case MODEL_STOP_TIME:       return tr("Stop");
        case MODEL_CHANNEL:         return tr("Channel");
        case MODEL_TITLE:           return tr("Title");
        }
    }
    return QVariant();
}

Qt::ItemFlags TvBookmarkModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == MODEL_CHECK)
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

static bool sortDay(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->dayOfWeek() < b2->dayOfWeek())
        return true;
    if (b1->dayOfWeek() > b2->dayOfWeek())
        return false;
    return b1->startTime() < b2->startTime();
}

static bool sortStart(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->startTime() < b2->startTime())
        return true;
    if (b1->startTime() > b2->startTime())
        return false;
    return b1->dayOfWeek() < b2->dayOfWeek();
}

static bool sortStop(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->stopTime() < b2->stopTime())
        return true;
    if (b1->stopTime() > b2->stopTime())
        return false;
    return b1->dayOfWeek() < b2->dayOfWeek();
}

static bool sortChannel(TvBookmark *b1, TvBookmark *b2)
{
    return b1->channelId().compare(b2->channelId(), Qt::CaseInsensitive) < 0;
}

static bool sortTitle(TvBookmark *b1, TvBookmark *b2)
{
    return b1->title().compare(b2->title(), Qt::CaseInsensitive) < 0;
}

static bool sortDayD(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->dayOfWeek() > b2->dayOfWeek())
        return true;
    if (b1->dayOfWeek() < b2->dayOfWeek())
        return false;
    return b1->startTime() > b2->startTime();
}

static bool sortStartD(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->startTime() > b2->startTime())
        return true;
    if (b1->startTime() < b2->startTime())
        return false;
    return b1->dayOfWeek() > b2->dayOfWeek();
}

static bool sortStopD(TvBookmark *b1, TvBookmark *b2)
{
    if (b1->stopTime() > b2->stopTime())
        return true;
    if (b1->stopTime() < b2->stopTime())
        return false;
    return b1->dayOfWeek() > b2->dayOfWeek();
}

static bool sortChannelD(TvBookmark *b1, TvBookmark *b2)
{
    return b1->channelId().compare(b2->channelId(), Qt::CaseInsensitive) > 0;
}

static bool sortTitleD(TvBookmark *b1, TvBookmark *b2)
{
    return b1->title().compare(b2->title(), Qt::CaseInsensitive) > 0;
}

void TvBookmarkModel::sort(int column, Qt::SortOrder order)
{
    if (column == MODEL_CHECK)
        return;
    if (order == Qt::AscendingOrder) {
        switch (column) {
        case MODEL_DAY:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortDay);
        case MODEL_START_TIME:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortStart);
        case MODEL_STOP_TIME:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortStop);
        case MODEL_CHANNEL:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortChannel);
        case MODEL_TITLE:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortTitle);
        }
    } else {
        switch (column) {
        case MODEL_DAY:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortDayD);
        case MODEL_START_TIME:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortStartD);
        case MODEL_STOP_TIME:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortStopD);
        case MODEL_CHANNEL:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortChannelD);
        case MODEL_TITLE:
            return qSort(m_bookmarks.begin(), m_bookmarks.end(), sortTitleD);
        }
    }
    reset();
}

void TvBookmarkModel::addBookmark(TvBookmark *bookmark)
{
    beginInsertRows(QModelIndex(), m_bookmarks.size(), m_bookmarks.size());
    m_bookmarks.append(bookmark);
    endInsertRows();
}

void TvBookmarkModel::updateBookmark(int index)
{
    emit dataChanged(createIndex(index, 0),
                     createIndex(index, MODEL_NUM_COLS - 0));
}

void TvBookmarkModel::removeBookmark(int index)
{
    if (index < 0 || index >= m_bookmarks.size())
        return;
    beginRemoveRows(QModelIndex(), index, index);
    delete m_bookmarks.at(index);
    m_bookmarks.removeAt(index);
    endRemoveRows();
}

QList<TvBookmark *> TvBookmarkModel::detachBookmarks()
{
    QList<TvBookmark *> list = m_bookmarks;
    m_bookmarks = QList<TvBookmark *>();
    return list;
}
