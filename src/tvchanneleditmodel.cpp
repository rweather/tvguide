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

#include "tvchanneleditmodel.h"

TvChannelEditable::TvChannelEditable(TvChannel *channel)
    : m_channel(channel)
{
    m_iconFile = channel->iconFile();
    m_icon = channel->icon();
    m_hidden = channel->isHidden();
    m_convertTimezone = channel->convertTimezone();
}

TvChannelEditable::~TvChannelEditable()
{
}

QString TvChannelEditable::name() const
{
    return m_channel->name();
}

QStringList TvChannelEditable::channelNumbers() const
{
    return m_channel->channelNumbers();
}

void TvChannelEditable::updateChannel()
{
    m_channel->setIconFile(m_iconFile);
    m_channel->setIcon(m_icon);
    m_channel->setHidden(m_hidden);
    m_channel->setConvertTimezone(m_convertTimezone);
}

TvChannelListEditable::TvChannelListEditable(TvChannelList *channelList)
    : m_channelList(channelList)
{
    QList<TvChannel *> channels = channelList->activeChannels();
    for (int index = 0; index < channels.size(); ++index)
        m_channels.append(new TvChannelEditable(channels.at(index)));
}

TvChannelListEditable::~TvChannelListEditable()
{
    qDeleteAll(m_channels);
}

QList<TvChannelEditable *> TvChannelListEditable::visibleChannels() const
{
    QList<TvChannelEditable *> result;
    for (int index = 0; index < m_channels.size(); ++index) {
        TvChannelEditable *channel = m_channels.at(index);
        if (!channel->isHidden())
            result.append(channel);
    }
    return result;
}

QList<TvChannelEditable *> TvChannelListEditable::hiddenChannels() const
{
    QList<TvChannelEditable *> result;
    for (int index = 0; index < m_channels.size(); ++index) {
        TvChannelEditable *channel = m_channels.at(index);
        if (channel->isHidden())
            result.append(channel);
    }
    return result;
}

void TvChannelListEditable::updateChannels()
{
    for (int index = 0; index < m_channels.size(); ++index) {
        TvChannelEditable *channel = m_channels.at(index);
        channel->updateChannel();
    }
}

TvChannelEditModel::TvChannelEditModel(TvChannelListEditable *channelList, bool isHiddenList, QObject *parent)
    : QAbstractItemModel(parent)
    , m_channelList(channelList)
    , m_isHiddenList(isHiddenList)
{
    refreshChannels();
}

TvChannelEditModel::~TvChannelEditModel()
{
}

QModelIndex TvChannelEditModel::index(int row, int column, const QModelIndex &) const
{
    if (column < 0 || column >= ColumnCount)
        return QModelIndex();
    if (row < 0 || row >= m_channels.size())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex TvChannelEditModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvChannelEditModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

int TvChannelEditModel::rowCount(const QModelIndex &) const
{
    return m_channels.size();
}

QVariant TvChannelEditModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row = index.row();
    if (row < 0 || row >= m_channels.size())
        return QVariant();
    TvChannelEditable *channel = m_channels.at(row);
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
        if (m_channelList->channelList()->haveChannelNumbers()) {
            if (index.column() == ColumnNumber)
                return channel->icon();
        } else {
            if (index.column() == ColumnName)
                return channel->icon();
        }
    }
    return QVariant();
}

QVariant TvChannelEditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == ColumnNumber)
            return tr("Number");
        if (section == ColumnName)
            return tr("Name");
    }
    return QVariant();
}

TvChannelEditable *TvChannelEditModel::channel(const QModelIndex &index) const
{
    int row = index.row();
    if (row < 0 || row >= m_channels.size())
        return 0;
    return m_channels.at(row);
}

bool TvChannelEditModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int leftRow = left.row();
    if (leftRow < 0 || leftRow >= m_channels.size())
        return false;
    int rightRow = right.row();
    if (rightRow < 0 || rightRow >= m_channels.size())
        return false;
    TvChannel *leftChannel = m_channels.at(leftRow)->channel();
    TvChannel *rightChannel = m_channels.at(rightRow)->channel();
    if (left.column() == ColumnNumber) {
        return leftChannel->compare(rightChannel) < 0;
    } else if (left.column() == ColumnName) {
        return leftChannel->name().compare(rightChannel->name(), Qt::CaseInsensitive) < 0;
    } else {
        return false;
    }
}

void TvChannelEditModel::refreshChannels()
{
    if (m_isHiddenList)
        m_channels = m_channelList->hiddenChannels();
    else
        m_channels = m_channelList->visibleChannels();
    reset();
}
