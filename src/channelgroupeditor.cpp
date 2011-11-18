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

#include "channelgroupeditor.h"
#include "helpbrowser.h"
#include <QtGui/qinputdialog.h>
#include <QtGui/qmessagebox.h>
#include <QtCore/qdebug.h>

ChannelGroupEditor::ChannelGroupEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
    , m_channels(channelList)
    , m_changingSelection(false)
{
    setupUi(this);

    QList<TvChannelGroup *> origGroups = channelList->groups();
    for (int index = 0; index < origGroups.size(); ++index)
        m_groups.append(new TvChannelGroup(origGroups.at(index)));

    setWindowModality(Qt::WindowModal);

    m_activeChannels = new TvChannelEditModel(&m_channels, false, this);
    m_activeChannelsSort = new TvChannelEditSortProxy(m_activeChannels, this);
    m_activeChannelsSort->setDynamicSortFilter(true);

    channels->setModel(m_activeChannelsSort);

    int sortColumn;
    if (m_channelList->haveChannelNumbers())
        sortColumn = TvChannelEditModel::ColumnNumber;
    else
        sortColumn = TvChannelEditModel::ColumnName;

    channels->verticalHeader()->hide();
    channels->horizontalHeader()->setStretchLastSection(true);
    channels->setSelectionBehavior(QTableView::SelectRows);
    channels->setSortingEnabled(true);
    channels->horizontalHeader()->setSortIndicator(sortColumn, Qt::AscendingOrder);
    channels->setColumnHidden(TvChannelEditModel::ColumnNumber, !m_channelList->haveChannelNumbers());
    channels->setColumnWidth(TvChannelEditModel::ColumnNumber, 70);
    channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
    if (!m_channelList->haveChannelNumbers())
        channels->horizontalHeader()->hide();

    if (channelList->largeIcons())
        channels->setIconSize(QSize(32, 32));
    else
        channels->setIconSize(QSize(16, 16));

    deleteButton->setEnabled(false);
    renameButton->setEnabled(false);
    channels->setEnabled(false);

    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));

    connect(newButton, SIGNAL(clicked()), this, SLOT(newGroup()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteGroup()));
    connect(renameButton, SIGNAL(clicked()), this, SLOT(renameGroup()));

    connect(channels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(channelSelectionChanged()));

    connect(groups, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(groupSelectionChanged(QListWidgetItem *)));

    updateGroupList();
}

ChannelGroupEditor::~ChannelGroupEditor()
{
    qDeleteAll(m_groups);
}

void ChannelGroupEditor::setActiveGroup(TvChannelGroup *group)
{
    int index;
    if (!group)
        return;
    for (index = 0; index < m_groups.size(); ++index) {
        if (m_groups.at(index)->name() == group->name())
            break;
    }
    if (index >= m_groups.size())
        return;
    int count = groups->count();
    for (int posn = 0; posn < count; ++posn) {
        QListWidgetItem *item = groups->item(posn);
        int itemIndex = item->data(Qt::UserRole).toInt();
        if (itemIndex == index) {
            item->setSelected(true);
            groups->scrollToItem(item);
            groups->setCurrentItem(item);
        } else {
            item->setSelected(false);
        }
    }
}

void ChannelGroupEditor::accept()
{
    m_channelList->setGroups(m_groups, true);
    m_groups.clear();
    QDialog::accept();
}

void ChannelGroupEditor::newGroup()
{
    QString name = QInputDialog::getText
        (this, QObject::tr("New Group"), QObject::tr("Name:"));
    if (name.isEmpty())
        return;
    TvChannelGroup *group;
    for (int index = 0; index < m_groups.size(); ++index) {
        group = m_groups.at(index);
        if (name == group->name()) {
            // Group already exists, so just switch to it.
            selectGroup(index);
            return;
        }
    }

    // Create a new group, initially empty of channels.
    group = new TvChannelGroup(m_channelList);
    group->setName(name);
    int posn = m_groups.size();
    m_groups.append(group);

    // Add a new list widget item for the group and select it.
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, posn);
    groups->addItem(item);
    groups->sortItems();
    selectGroup(posn);
}

void ChannelGroupEditor::deleteGroup()
{
    QList<QListWidgetItem *> selected = groups->selectedItems();
    if (!selected.isEmpty()) {
        QListWidgetItem *item = selected.at(0);
        int index = item->data(Qt::UserRole).toInt();
        TvChannelGroup *group = m_groups.at(index);
        m_groups.removeAt(index);
        delete group;
        updateGroupList();
    }
}

void ChannelGroupEditor::renameGroup()
{
    QList<QListWidgetItem *> selected = groups->selectedItems();
    if (!selected.isEmpty()) {
        QListWidgetItem *item = selected.at(0);
        int index = item->data(Qt::UserRole).toInt();
        TvChannelGroup *group = m_groups.at(index);
        QString name = group->name();
        bool ok = false;
        do {
            name = QInputDialog::getText
                (this, QObject::tr("Rename Group"),
                 QObject::tr("Name:"), QLineEdit::Normal, name);
            if (name.isEmpty() || name == group->name())
                return;
            int index2 = 0;
            for (; index2 < m_groups.size(); ++index2) {
                if (m_groups.at(index2)->name() == name)
                    break;
            }
            if (index2 < m_groups.size()) {
                QMessageBox::critical
                    (this, QObject::tr("Rename Channel Group"),
                     QObject::tr("The name '%1' is in use by another group.").arg(name));
            } else {
                ok = true;
            }
        } while (!ok);
        group->setName(name);
        updateGroupList();
    }
}

void ChannelGroupEditor::groupSelectionChanged(QListWidgetItem *item)
{
    deleteButton->setEnabled(item != 0);
    renameButton->setEnabled(item != 0);
    channels->setEnabled(item != 0);
    m_changingSelection = true;
    if (item) {
        TvChannelGroup *group = m_groups.at(item->data(Qt::UserRole).toInt());
        QItemSelection selection;
        int count = m_activeChannels->rowCount(QModelIndex());
        for (int row = 0; row < count; ++row) {
            QModelIndex startRow =
                m_activeChannels->index(row, 0, QModelIndex());
            QModelIndex endRow =
                m_activeChannels->index
                    (row, TvChannelEditModel::ColumnCount - 1,
                     QModelIndex());
            TvChannelEditable *channel =
                m_activeChannels->channel(startRow);
            if (group->channelIds().contains(channel->channel()->id()))
                selection.select(startRow, endRow);
        }
        channels->selectionModel()->select
            (m_activeChannelsSort->mapSelectionFromSource(selection),
             QItemSelectionModel::ClearAndSelect);
        memberCount->setText
            (QObject::tr("%1 members")
                .arg(QString::number(group->channelIds().size())));
    } else {
        channels->clearSelection();
        memberCount->setText(QString());
    }
    m_changingSelection = false;
}

void ChannelGroupEditor::channelSelectionChanged()
{
    if (m_changingSelection)
        return;
    QList<QListWidgetItem *> groupSelection = groups->selectedItems();
    if (groupSelection.isEmpty()) {
        memberCount->setText(QString());
        return;
    }
    QListWidgetItem *item = groupSelection.at(0);
    TvChannelGroup *group = m_groups.at(item->data(Qt::UserRole).toInt());
    QStringList ids;
    int count = m_activeChannels->rowCount(QModelIndex());
    QItemSelection selection =
        m_activeChannelsSort->mapSelectionToSource
            (channels->selectionModel()->selection());
    for (int row = 0; row < count; ++row) {
        QModelIndex startRow =
            m_activeChannels->index(row, 0, QModelIndex());
        if (selection.contains(startRow)) {
            TvChannelEditable *channel =
                m_activeChannels->channel(startRow);
            ids += channel->channel()->id();
        }
    }
    group->setChannelIds(ids);
    memberCount->setText
        (QObject::tr("%1 members").arg(QString::number(ids.size())));
}

void ChannelGroupEditor::help()
{
    HelpBrowser::showContextHelp(QLatin1String("groups.html"), this);
}

void ChannelGroupEditor::updateGroupList()
{
    groups->clear();
    for (int index = 0; index < m_groups.size(); ++index) {
        TvChannelGroup *group = m_groups.at(index);
        QListWidgetItem *item = new QListWidgetItem(group->name());
        item->setData(Qt::UserRole, index);
        groups->addItem(item);
    }
    groups->sortItems();
    if (groups->count() > 0) {
        groups->item(0)->setSelected(true);
        groups->setCurrentRow(0);
    }
}

void ChannelGroupEditor::selectGroup(int index)
{
    int count = groups->count();
    for (int posn = 0; posn < count; ++posn) {
        QListWidgetItem *item = groups->item(posn);
        int itemIndex = item->data(Qt::UserRole).toInt();
        if (itemIndex == index) {
            item->setSelected(true);
            groups->scrollToItem(item);
            groups->setCurrentItem(item);
        } else {
            item->setSelected(false);
        }
    }
}
