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

#ifndef _CHANNELGROUPEDITOR_H
#define _CHANNELGROUPEDITOR_H

#include <QtGui/qdialog.h>
#include "ui_channelgroupeditor.h"
#include "tvchanneleditmodel.h"
#include "tvchannelgroup.h"

class ChannelGroupEditor : public QDialog, private Ui::ChannelGroupEditor
{
    Q_OBJECT
public:
    explicit ChannelGroupEditor(TvChannelList *channelList, QWidget *parent = 0);
    ~ChannelGroupEditor();

    void setActiveGroup(TvChannelGroup *group);

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void newGroup();
    void deleteGroup();
    void renameGroup();
    void groupSelectionChanged(QListWidgetItem *item);
    void channelSelectionChanged();
    void help();

private:
    TvChannelList *m_channelList;
    TvChannelListEditable m_channels;
    TvChannelEditModel *m_activeChannels;
    TvChannelEditSortProxy *m_activeChannelsSort;
    QList<TvChannelGroup *> m_groups;
    bool m_changingSelection;

    void updateGroupList();
    void selectGroup(int index);
};

#endif
