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

#ifndef _CHANNELEDITOR_H
#define _CHANNELEDITOR_H

#include <QtGui/qdialog.h>
#include "ui_channeleditor.h"

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
    void itemDoubleClicked(QListWidgetItem *item);
    void largeIconsChanged(bool value);

private:
    TvChannelList *m_channelList;
};

#endif
