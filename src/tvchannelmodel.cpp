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

#include "tvchannelmodel.h"
#include <QtCore/qdebug.h>

TvChannelModel::TvChannelModel(TvChannelList *channelList, QObject *parent)
    : QAbstractItemModel(parent)
    , m_channelList(channelList)
{
    connect(channelList, SIGNAL(channelsChanged()),
            this, SLOT(channelsChanged()));
    connect(channelList, SIGNAL(hiddenChannelsChanged()),
            this, SLOT(channelsChanged()));
    connect(channelList, SIGNAL(channelIconsChanged()),
            this, SLOT(channelIconsChanged()));
    connect(channelList, SIGNAL(groupsChanged()),
            this, SLOT(channelsChanged()));
    loadVisibleChannels();
}

TvChannelModel::~TvChannelModel()
{
}

// Index 0 is always "All Channels".  Indexes 1..N are the N groups.
// The remaining indexes are individual channels.
QModelIndex TvChannelModel::index(int row, int column, const QModelIndex &) const
{
    if (column < 0 || column >= ColumnCount)
        return QModelIndex();
    if (row >= 0 && row <= m_groups.size())
        return createIndex(row, column);
    int chrow = row - m_groups.size() - 1;
    if (chrow < 0 || chrow >= m_visibleChannels.size())
        return QModelIndex();
    return createIndex(row, column, m_visibleChannels.at(chrow));
}

QModelIndex TvChannelModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvChannelModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

int TvChannelModel::rowCount(const QModelIndex &) const
{
    return m_groups.size() + m_visibleChannels.size() + 1;
}

QVariant TvChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row = index.row();
    if (!row) {
        if (role == Qt::DisplayRole)
            return tr("All Channels");
        return QVariant();
    }
    if (row > 0 && row <= m_groups.size()) {
        TvChannelGroup *group = m_groups.at(row - 1);
        if (role == Qt::DisplayRole)
            return group->name();
        return QVariant();
    }
    row -= m_groups.size() + 1;
    if (row < 0 || row >= m_visibleChannels.size())
        return QVariant();
    TvChannel *channel = m_visibleChannels.at(row);
    if (role == Qt::DisplayRole) {
        if (index.column() == ColumnName) {
            return channel->name();
        } else if (index.column() == ColumnNumber) {
            QStringList numbers = channel->channelNumbers();
            if (numbers.isEmpty())
                return QString();
            else
                return numbers.join(QLatin1String(", "));
        }
    } else if (role == Qt::DecorationRole) {
        if (m_channelList->haveChannelNumbers()) {
            if (index.column() == ColumnNumber)
                return channel->icon();
        } else {
            if (index.column() == ColumnName)
                return channel->icon();
        }
    }
    return QVariant();
}

QVariant TvChannelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == ColumnNumber)
            return tr("Number");
        if (section == ColumnName)
            return tr("Name");
    }
    return QVariant();
}

QList<TvChannel *> TvChannelModel::channelsForIndex(const QModelIndex &index) const
{
    QList<TvChannel *> channels;
    TvChannel *channel = static_cast<TvChannel *>(index.internalPointer());
    if (channel) {
        channels.append(channel);
    } else if (index.row() != 0) {
        TvChannelGroup *group = m_groups.at(index.row() - 1);
        QStringList ids = group->channelIds();
        for (int posn = 0; posn < ids.size(); ++posn) {
            channel = m_channelList->channel(ids.at(posn));
            if (channel)
                channels.append(channel);
        }
    } else {
        return m_visibleChannels;
    }
    return channels;
}

TvChannelGroup *TvChannelModel::groupForIndex(const QModelIndex &index) const
{
    int row = index.row();
    if (row > 0 && row <= m_groups.size())
        return m_groups.at(row - 1);
    else
        return 0;
}

QString TvChannelModel::itemToId(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    int row = index.row();
    if (!row) {
        return QLatin1String("all");
    } else if (row > 0 && row <= m_groups.size()) {
        return QLatin1String("group:") + m_groups.at(row - 1)->name();
    } else {
        row -= m_groups.size() + 1;
        return QLatin1String("channel:") + m_visibleChannels.at(row)->id();
    }
}

QModelIndex TvChannelModel::itemFromId(const QString &id) const
{
    if (id == QLatin1String("all")) {
        return createIndex(0, 0);
    } else if (id.startsWith(QLatin1String("group:"))) {
        QString name = id.mid(6);
        for (int index = 0; index < m_groups.size(); ++index) {
            if (m_groups.at(index)->name() == name)
                return createIndex(index + 1, 0);
        }
    } else if (id.startsWith(QLatin1String("channel:"))) {
        QString channelId = id.mid(8);
        for (int index = 0; index < m_visibleChannels.size(); ++index) {
            TvChannel *channel = m_visibleChannels.at(index);
            if (channel->id() == channelId)
                return createIndex(index + m_groups.size() + 1, 0, channel);
        }
    }
    return QModelIndex();
}

QStringList TvChannelModel::itemListToIds(const QModelIndexList &list) const
{
    QStringList ids;
    for (int index = 0; index < list.size(); ++index) {
        QString id = itemToId(list.at(index));
        if (!id.isEmpty())
            ids.append(id);
    }
    return ids;
}

QModelIndexList TvChannelModel::itemListFromIds(const QStringList &ids) const
{
    QModelIndexList list;
    for (int index = 0; index < ids.size(); ++index) {
        QModelIndex itemIndex = itemFromId(ids.at(index));
        if (itemIndex.isValid())
            list.append(itemIndex);
    }
    return list;
}

void TvChannelModel::channelsChanged()
{
    loadVisibleChannels();
    reset();
}

void TvChannelModel::channelIconsChanged()
{
    reset();
}

static bool sortGroups(TvChannelGroup *g1, TvChannelGroup *g2)
{
    return g1->name().compare(g2->name(), Qt::CaseInsensitive) < 0;
}

void TvChannelModel::loadVisibleChannels()
{
    QList<TvChannel *> channels = m_channelList->activeChannels();
    m_visibleChannels.clear();
    for (int index = 0; index < channels.size(); ++index) {
        TvChannel *channel = channels.at(index);
        if (!channel->isHidden())
            m_visibleChannels.append(channel);
    }
    m_groups = m_channelList->groups();
    qSort(m_groups.begin(), m_groups.end(), sortGroups);
}
