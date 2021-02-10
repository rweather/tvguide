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

#ifndef _CHANNELEDITOR2_H
#define _CHANNELEDITOR2_H

#include <QtWidgets/qdialog.h>
#include <QtCore/qmap.h>
#include <QtCore/qxmlstream.h>
#include "ui_channeleditor.h"
#include "tvchanneleditmodel.h"

class TvChannelList;

class ChannelEditor : public QDialog, private Ui::ChannelEditor
{
    Q_OBJECT
public:
    explicit ChannelEditor(TvChannelList *channelList, QWidget *parent = 0);
    ~ChannelEditor();

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void moveToInactive();
    void moveToActive();
    void moveToInactiveAll();
    void moveToActiveAll();
    void setIcon();
    void removeIcon();
    void updateMakeInactive();
    void updateMakeActive();
    void updateSetIcon();
    void updateTimezone();
    void activeDoubleClicked(const QModelIndex &index);
    void inactiveDoubleClicked(const QModelIndex &index);
    void largeIconsChanged(bool value);
    void regionChanged(int index);
    void timezoneChanged(bool value);
    void help();

private:
    struct Region
    {
        Region *parent;
        Region *otherParent;
        QString id;
        QString name;
        bool isSelectable;
    };

    TvChannelList *m_channelList;
    TvChannelListEditable m_channels;
    TvChannelEditModel *m_activeChannels;
    TvChannelEditModel *m_inactiveChannels;
    TvChannelEditSortProxy *m_activeChannelsSort;
    TvChannelEditSortProxy *m_inactiveChannelsSort;
    QMap<QString, Region *> m_regions;
    QMap<QString, Region *> m_channelToRegion;
    bool m_timezoneBlock;
    bool m_timezonesChanged;

    void loadOzTivoRegions();
    void loadOzTivoRegionData(QXmlStreamReader *reader);
    static bool channelInRegion(const Region *cregion, const Region *region);

    QList<TvChannelEditable *> selectedActiveChannels() const;
    QList<TvChannelEditable *> selectedInactiveChannels() const;
    void refreshChannels();
};

#endif
