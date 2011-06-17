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

TvChannelModel::TvChannelModel(TvChannelList *channelList, QObject *parent)
    : QAbstractItemModel(parent)
    , m_channelList(channelList)
{
    connect(channelList, SIGNAL(channelsChanged()),
            this, SLOT(channelsChanged()));
    connect(channelList, SIGNAL(hiddenChannelsChanged()),
            this, SLOT(channelsChanged()));
    loadVisibleChannels();
}

TvChannelModel::~TvChannelModel()
{
}

QModelIndex TvChannelModel::index(int row, int column, const QModelIndex &) const
{
    if (column != 0 || row < 0 || row >= m_visibleChannels.size())
        return QModelIndex();
    return createIndex(row, column, m_visibleChannels.at(row));
}

QModelIndex TvChannelModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvChannelModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int TvChannelModel::rowCount(const QModelIndex &) const
{
    return m_visibleChannels.size();
}

QVariant TvChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.column() == 0 && role == Qt::DisplayRole) {
        int row = index.row();
        if (row >= 0 && row < m_visibleChannels.size())
            return m_visibleChannels.at(row)->name();
    }
    return QVariant();
}

QVariant TvChannelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
    }
    return QVariant();
}

void TvChannelModel::channelsChanged()
{
    loadVisibleChannels();
    reset();
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
}
