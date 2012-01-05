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

#ifndef _TVCHANNELEDITMODEL_H
#define _TVCHANNELEDITMODEL_H

#include "tvchannellist.h"
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qicon.h>
#include <QtGui/qsortfilterproxymodel.h>

class TvChannelEditable
{
public:
    explicit TvChannelEditable(TvChannel *channel);
    ~TvChannelEditable();

    QString name() const;
    QStringList channelNumbers() const;

    TvChannel *channel() const { return m_channel; }

    QString iconFile() const { return m_iconFile; }
    void setIconFile(const QString &file) { m_iconFile = file; }

    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon &icon) { m_icon = icon; }

    bool isHidden() const { return m_hidden; }
    void setHidden(bool value) { m_hidden = value; }

    bool convertTimezone() const { return m_convertTimezone; }
    void setConvertTimezone(bool value) { m_convertTimezone = value; }

    void updateChannel();

private:
    TvChannel *m_channel;
    QString m_iconFile;
    QIcon m_icon;
    bool m_hidden;
    bool m_convertTimezone;
};

class TvChannelListEditable
{
public:
    explicit TvChannelListEditable(TvChannelList *channelList);
    ~TvChannelListEditable();

    TvChannelList *channelList() const { return m_channelList; }

    QList<TvChannelEditable *> visibleChannels() const;
    QList<TvChannelEditable *> hiddenChannels() const;
    QList<TvChannelEditable *> allChannels() const { return m_channels; }

    void updateChannels();

private:
    TvChannelList *m_channelList;
    QList<TvChannelEditable *> m_channels;
};

class TvChannelEditModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TvChannelEditModel(TvChannelListEditable *channelList, bool isHiddenList, QObject *parent = 0);
    ~TvChannelEditModel();

    static const int ColumnCount    = 2;
    static const int ColumnNumber   = 0;
    static const int ColumnName     = 1;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    TvChannelEditable *channel(const QModelIndex &index) const;

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    void refreshChannels();

    QList<TvChannelEditable *> channels() const { return m_channels; }

private:
    TvChannelListEditable *m_channelList;
    bool m_isHiddenList;
    QList<TvChannelEditable *> m_channels;
};

class TvChannelEditSortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    TvChannelEditSortProxy(TvChannelEditModel *source, QObject *parent)
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
    TvChannelEditModel *m_source;
};

#endif
