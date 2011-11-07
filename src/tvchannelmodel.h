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

#ifndef _TVCHANNELMODEL_H
#define _TVCHANNELMODEL_H

#include "tvchannellist.h"
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qicon.h>

class TvChannelModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TvChannelModel(TvChannelList *channelList, QObject *parent = 0);
    ~TvChannelModel();

    static const int ColumnCount    = 2;
    static const int ColumnNumber   = 0;
    static const int ColumnName     = 1;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QList<TvChannel *> channelsForIndex(const QModelIndex &index) const;
    TvChannelGroup *groupForIndex(const QModelIndex &index) const;

    int groupCount() const { return m_groups.size(); }

private Q_SLOTS:
    void channelsChanged();
    void channelIconsChanged();

private:
    TvChannelList *m_channelList;
    QList<TvChannel *> m_visibleChannels;
    QList<TvChannelGroup *> m_groups;

    void loadVisibleChannels();
};

#endif
