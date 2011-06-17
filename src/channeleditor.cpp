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

#include "channeleditor.h"
#include "tvchannellist.h"

ChannelEditor::ChannelEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
{
    setupUi(this);

    QList<TvChannel *> channels = channelList->activeChannels();

    for (int index = 0; index < channels.size(); ++index) {
        TvChannel *channel = channels.at(index);
        QListWidgetItem *item1 = new QListWidgetItem(channel->name());
        item1->setData(Qt::UserRole, QVariant::fromValue<void *>(channel));
        activeChannels->addItem(item1);
        item1->setHidden(channel->isHidden());

        QListWidgetItem *item2 = new QListWidgetItem(channel->name());
        item2->setData(Qt::UserRole, QVariant::fromValue<void *>(channel));
        inactiveChannels->addItem(item2);
        item2->setHidden(!channel->isHidden());

        item1->setData(Qt::UserRole + 1, QVariant::fromValue<void *>(item2));
        item2->setData(Qt::UserRole + 1, QVariant::fromValue<void *>(item1));
    }

    activeChannels->setSortingEnabled(true);
    inactiveChannels->setSortingEnabled(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(makeInactive, SIGNAL(clicked()), this, SLOT(moveToInactive()));
    connect(makeActive, SIGNAL(clicked()), this, SLOT(moveToActive()));
    connect(makeInactiveAll, SIGNAL(clicked()), this, SLOT(moveToInactiveAll()));
    connect(makeActiveAll, SIGNAL(clicked()), this, SLOT(moveToActiveAll()));

    connect(activeChannels, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateMakeInactive()));
    connect(activeChannels, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(itemDoubleClicked(QListWidgetItem*)));

    connect(inactiveChannels, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateMakeActive()));
    connect(inactiveChannels, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(itemDoubleClicked(QListWidgetItem*)));

    makeActive->setEnabled(false);
    makeInactive->setEnabled(false);
}

ChannelEditor::~ChannelEditor()
{
}

void ChannelEditor::accept()
{
    int count = activeChannels->count();
    for (int index = 0; index < count; ++index) {
        QListWidgetItem *item = activeChannels->item(index);
        TvChannel *channel = static_cast<TvChannel *>
            (item->data(Qt::UserRole).value<void *>());
        channel->setHidden(item->isHidden());
    }
    m_channelList->updateHidden();
    QDialog::accept();
}

void ChannelEditor::moveToInactive()
{
    QList<QListWidgetItem *> items = activeChannels->selectedItems();
    for (int index = 0; index < items.size(); ++index) {
        QListWidgetItem *item = items.at(index);
        QListWidgetItem *other = static_cast<QListWidgetItem *>
            (item->data(Qt::UserRole + 1).value<void *>());
        item->setHidden(true);
        other->setHidden(false);
    }
    activeChannels->clearSelection();
}

void ChannelEditor::moveToActive()
{
    QList<QListWidgetItem *> items = inactiveChannels->selectedItems();
    for (int index = 0; index < items.size(); ++index) {
        QListWidgetItem *item = items.at(index);
        QListWidgetItem *other = static_cast<QListWidgetItem *>
            (item->data(Qt::UserRole + 1).value<void *>());
        item->setHidden(true);
        other->setHidden(false);
    }
    inactiveChannels->clearSelection();
}

void ChannelEditor::moveToInactiveAll()
{
    int count = activeChannels->count();
    for (int index = 0; index < count; ++index) {
        QListWidgetItem *item = activeChannels->item(index);
        item->setHidden(true);
        item = inactiveChannels->item(index);
        item->setHidden(false);
    }
    activeChannels->clearSelection();
    inactiveChannels->clearSelection();
}

void ChannelEditor::moveToActiveAll()
{
    int count = activeChannels->count();
    for (int index = 0; index < count; ++index) {
        QListWidgetItem *item = activeChannels->item(index);
        item->setHidden(false);
        item = inactiveChannels->item(index);
        item->setHidden(true);
    }
    activeChannels->clearSelection();
    inactiveChannels->clearSelection();
}

void ChannelEditor::updateMakeInactive()
{
    makeInactive->setEnabled(!activeChannels->selectedItems().isEmpty());
}

void ChannelEditor::updateMakeActive()
{
    makeActive->setEnabled(!inactiveChannels->selectedItems().isEmpty());
}

void ChannelEditor::itemDoubleClicked(QListWidgetItem *item)
{
    QListWidgetItem *other = static_cast<QListWidgetItem *>
        (item->data(Qt::UserRole + 1).value<void *>());
    item->setHidden(true);
    other->setHidden(false);
    activeChannels->clearSelection();
    inactiveChannels->clearSelection();
}
