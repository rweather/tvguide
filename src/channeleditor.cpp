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

#include "channeleditor.h"
#include "tvchannellist.h"
#include "helpbrowser.h"
#include <QtWidgets/qfiledialog.h>
#include <QtCore/qfile.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qdebug.h>

ChannelEditor::ChannelEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
    , m_channels(channelList)
    , m_timezoneBlock(false)
    , m_timezonesChanged(false)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    m_activeChannels = new TvChannelEditModel(&m_channels, false, this);
    m_inactiveChannels = new TvChannelEditModel(&m_channels, true, this);
    m_activeChannelsSort = new TvChannelEditSortProxy(m_activeChannels, this);
    m_inactiveChannelsSort = new TvChannelEditSortProxy(m_inactiveChannels, this);

    m_activeChannelsSort->setDynamicSortFilter(true);
    m_inactiveChannelsSort->setDynamicSortFilter(true);

    activeChannels->setModel(m_activeChannelsSort);
    inactiveChannels->setModel(m_inactiveChannelsSort);

    int sortColumn;
    if (m_channelList->haveChannelNumbers())
        sortColumn = TvChannelEditModel::ColumnNumber;
    else
        sortColumn = TvChannelEditModel::ColumnName;

    activeChannels->verticalHeader()->hide();
    activeChannels->horizontalHeader()->setStretchLastSection(true);
    activeChannels->setSelectionBehavior(QTableView::SelectRows);
    activeChannels->setSortingEnabled(true);
    activeChannels->horizontalHeader()->setSortIndicator(sortColumn, Qt::AscendingOrder);
    activeChannels->setColumnHidden(TvChannelEditModel::ColumnNumber, !m_channelList->haveChannelNumbers());
    activeChannels->setColumnWidth(TvChannelEditModel::ColumnNumber, 70);
    activeChannels->resizeRowsToContents();
    activeChannels->resizeColumnsToContents();
    if (!m_channelList->haveChannelNumbers())
        activeChannels->horizontalHeader()->hide();

    inactiveChannels->verticalHeader()->hide();
    inactiveChannels->horizontalHeader()->setStretchLastSection(true);
    inactiveChannels->setSelectionBehavior(QTableView::SelectRows);
    inactiveChannels->setSortingEnabled(true);
    inactiveChannels->horizontalHeader()->setSortIndicator(sortColumn, Qt::AscendingOrder);
    inactiveChannels->setColumnHidden(TvChannelEditModel::ColumnNumber, !m_channelList->haveChannelNumbers());
    inactiveChannels->setColumnWidth(TvChannelEditModel::ColumnNumber, 70);
    inactiveChannels->resizeRowsToContents();
    inactiveChannels->resizeColumnsToContents();
    if (!m_channelList->haveChannelNumbers())
        inactiveChannels->horizontalHeader()->hide();

    if (channelList->largeIcons()) {
        activeChannels->setIconSize(QSize(32, 32));
        inactiveChannels->setIconSize(QSize(32, 32));
        largeIcons->setChecked(true);
    } else {
        activeChannels->setIconSize(QSize(16, 16));
        inactiveChannels->setIconSize(QSize(16, 16));
    }

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));

    connect(makeInactive, SIGNAL(clicked()), this, SLOT(moveToInactive()));
    connect(makeActive, SIGNAL(clicked()), this, SLOT(moveToActive()));
    connect(makeInactiveAll, SIGNAL(clicked()), this, SLOT(moveToInactiveAll()));
    connect(makeActiveAll, SIGNAL(clicked()), this, SLOT(moveToActiveAll()));
    connect(setIconButton, SIGNAL(clicked()), this, SLOT(setIcon()));
    connect(removeIconButton, SIGNAL(clicked()), this, SLOT(removeIcon()));
    connect(largeIcons, SIGNAL(toggled(bool)), this, SLOT(largeIconsChanged(bool)));
    connect(timezone, SIGNAL(toggled(bool)), this, SLOT(timezoneChanged(bool)));

    connect(activeChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateMakeInactive()));
    connect(activeChannels, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(activeDoubleClicked(QModelIndex)));
    connect(activeChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateSetIcon()));
    connect(activeChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateTimezone()));

    connect(inactiveChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateMakeActive()));
    connect(inactiveChannels, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(inactiveDoubleClicked(QModelIndex)));

    makeActive->setEnabled(false);
    makeInactive->setEnabled(false);
    setIconButton->setEnabled(false);
    removeIconButton->setEnabled(false);
    timezone->setEnabled(false);

    if (channelList->startUrl().host().endsWith(QLatin1String(".oztivo.net")))
        loadOzTivoRegions();

    if (m_regions.isEmpty()) {
        regionLabel->setVisible(false);
        regions->setVisible(false);
    } else {
        connect(regions, SIGNAL(currentIndexChanged(int)),
                this, SLOT(regionChanged(int)));
    }

    QMap<QString, Region *>::ConstIterator it;
    regions->addItem(tr("Select region ..."), QString());
    for (it = m_regions.constBegin(); it != m_regions.constEnd(); ++it) {
        if (it.value()->isSelectable)
            regions->addItem(it.value()->name, it.value()->id);
    }
}

ChannelEditor::~ChannelEditor()
{
    qDeleteAll(m_regions);
}

void ChannelEditor::accept()
{
    m_channels.updateChannels();
    m_channelList->updateChannels(largeIcons->isChecked());
    if (m_timezonesChanged)
        m_channelList->timezoneSettingsChanged();
    QDialog::accept();
}

void ChannelEditor::moveToInactive()
{
    QList<TvChannelEditable *> items = selectedActiveChannels();
    for (int index = 0; index < items.size(); ++index)
        items.at(index)->setHidden(true);
    activeChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::moveToActive()
{
    QList<TvChannelEditable *> items = selectedInactiveChannels();
    for (int index = 0; index < items.size(); ++index)
        items.at(index)->setHidden(false);
    inactiveChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::moveToInactiveAll()
{
    QList<TvChannelEditable *> items = m_activeChannels->channels();
    for (int index = 0; index < items.size(); ++index)
        items.at(index)->setHidden(true);
    activeChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::moveToActiveAll()
{
    QList<TvChannelEditable *> items = m_inactiveChannels->channels();
    for (int index = 0; index < items.size(); ++index)
        items.at(index)->setHidden(false);
    activeChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::setIcon()
{
    QList<TvChannelEditable *> items = selectedActiveChannels();
    if (items.size() != 1)
        return;
    TvChannelEditable *item = items.at(0);
    QString iconFile = item->iconFile();
    QString result = QFileDialog::getOpenFileName
        (this, tr("Select Icon"), iconFile,
         tr("Images (*.png *.jpg)"));
    if (!result.isEmpty() && result != iconFile) {
        item->setIconFile(result);
        item->setIcon(QIcon(result));
        updateSetIcon();
    }
}

void ChannelEditor::removeIcon()
{
    QList<TvChannelEditable *> items = selectedActiveChannels();
    if (items.size() != 1)
        return;
    TvChannelEditable *item = items.at(0);
    item->setIconFile(QString());
    item->setIcon(QIcon());
    updateSetIcon();
}

void ChannelEditor::updateMakeInactive()
{
    makeInactive->setEnabled(!selectedActiveChannels().isEmpty());
}

void ChannelEditor::updateMakeActive()
{
    makeActive->setEnabled(!selectedInactiveChannels().isEmpty());
}

void ChannelEditor::updateSetIcon()
{
    QList<TvChannelEditable *> items = selectedActiveChannels();
    setIconButton->setEnabled(items.size() == 1);
    removeIconButton->setEnabled(items.size() == 1 && !items.at(0)->icon().isNull());
}

void ChannelEditor::updateTimezone()
{
    m_timezoneBlock = true;
    QList<TvChannelEditable *> items = selectedActiveChannels();
    if (items.size() == 1) {
        timezone->setEnabled(true);
        timezone->setChecked(items.at(0)->convertTimezone());
    } else {
        timezone->setEnabled(false);
        timezone->setChecked(false);
    }
    m_timezoneBlock = false;
}

void ChannelEditor::activeDoubleClicked(const QModelIndex &index)
{
    TvChannelEditable *channel =
        m_activeChannels->channel
            (m_activeChannelsSort->mapToSource(index));
    if (!channel)
        return;
    channel->setHidden(true);
    activeChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::inactiveDoubleClicked(const QModelIndex &index)
{
    TvChannelEditable *channel =
        m_inactiveChannels->channel
            (m_inactiveChannelsSort->mapToSource(index));
    if (!channel)
        return;
    channel->setHidden(false);
    inactiveChannels->clearSelection();
    regions->setCurrentIndex(0);
    refreshChannels();
}

void ChannelEditor::largeIconsChanged(bool value)
{
    if (value) {
        activeChannels->setIconSize(QSize(32, 32));
        inactiveChannels->setIconSize(QSize(32, 32));
    } else {
        activeChannels->setIconSize(QSize(16, 16));
        inactiveChannels->setIconSize(QSize(16, 16));
    }
}

void ChannelEditor::regionChanged(int index)
{
    QString id = regions->itemData(index).toString();
    if (id.isEmpty())
        return;
    Region *region = m_regions.value(id);
    Q_ASSERT(region);

    QList<TvChannelEditable *> channels = m_channels.allChannels();
    for (int index = 0; index < channels.size(); ++index) {
        TvChannelEditable *channel = channels.at(index);
        Region *cregion = m_channelToRegion.value(channel->channel()->id(), 0);
        if (!cregion)
            continue;   // Region-less channels are not changed.
        channel->setHidden(!channelInRegion(cregion, region));
    }

    refreshChannels();
}

void ChannelEditor::timezoneChanged(bool value)
{
    if (m_timezoneBlock)
        return;
    QList<TvChannelEditable *> items = selectedActiveChannels();
    if (!items.isEmpty())
        items.at(0)->setConvertTimezone(value);
    m_timezonesChanged = true;
}

void ChannelEditor::loadOzTivoRegions()
{
    QFile file(QLatin1String(":/data/channels_oztivo.xml"));
    if (!file.open(QIODevice::ReadOnly))
        return;
    QXmlStreamReader reader(&file);
    while (!reader.hasError()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("channel")) {
                QString channelId = reader.attributes().value
                        (QLatin1String("id")).toString();
                QString regionId = reader.attributes().value
                        (QLatin1String("region")).toString();
                Region *reg = m_regions.value(regionId, 0);
                if (reg)
                    m_channelToRegion.insert(channelId, reg);
            } else if (reader.name() == QLatin1String("region")) {
                loadOzTivoRegionData(&reader);
            }
        }
    }
}

void ChannelEditor::loadOzTivoRegionData(QXmlStreamReader *reader)
{
    // Will leave the XML stream positioned on </region>.
    Q_ASSERT(reader->isStartElement());
    Q_ASSERT(reader->name() == QLatin1String("region"));
    Region *region = new Region;
    region->id = reader->attributes().value(QLatin1String("id")).toString();
    region->name = region->id;
    region->parent = m_regions.value
        (reader->attributes().value(QLatin1String("parent")).toString(), 0);
    region->otherParent = 0;
    region->isSelectable = false;
    m_regions.insert(region->id, region);
    while (!reader->hasError()) {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader->name() == QLatin1String("display-name")) {
                region->name = reader->readElementText
                    (QXmlStreamReader::SkipChildElements);
            } else if (reader->name() == QLatin1String("selectable")) {
                region->isSelectable = true;
            } else if (reader->name() == QLatin1String("other-parent")) {
                region->otherParent = m_regions.value
                    (reader->readElementText
                        (QXmlStreamReader::SkipChildElements));
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("region"))
                break;
        }
    }
}

bool ChannelEditor::channelInRegion(const Region *cregion, const Region *region)
{
    while (region != 0) {
        if (cregion == region)
            return true;
        if (region->otherParent &&
                channelInRegion(cregion, region->otherParent))
            return true;
        region = region->parent;
    }
    return false;
}

void ChannelEditor::help()
{
    HelpBrowser::showContextHelp(QLatin1String("edit_channels.html"), this);
}

QList<TvChannelEditable *> ChannelEditor::selectedActiveChannels() const
{
    QModelIndexList selected = activeChannels->selectionModel()->selectedRows();
    QList<TvChannelEditable *> channels;
    for (int index = 0; index < selected.size(); ++index) {
        channels.append(m_activeChannels->channel
            (m_activeChannelsSort->mapToSource(selected.at(index))));
    }
    return channels;
}

QList<TvChannelEditable *> ChannelEditor::selectedInactiveChannels() const
{
    QModelIndexList selected = inactiveChannels->selectionModel()->selectedRows();
    QList<TvChannelEditable *> channels;
    for (int index = 0; index < selected.size(); ++index) {
        channels.append(m_inactiveChannels->channel
            (m_inactiveChannelsSort->mapToSource(selected.at(index))));
    }
    return channels;
}

void ChannelEditor::refreshChannels()
{
    m_activeChannels->refreshChannels();
    m_inactiveChannels->refreshChannels();
}
