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

#include "bookmarkitemeditor.h"
#include "tvchannellist.h"
#include "dayselectiondialog.h"
#include "helpbrowser.h"
#include <QtGui/qcolordialog.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qicon.h>

BookmarkItemEditor::BookmarkItemEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
    , m_extraItem(-1)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    dayOfWeekCombo->setCurrentIndex(0);
    dayOfWeekCombo->setItemData(TvBookmark::AnyDay, 0xFE);
    dayOfWeekCombo->setItemData(TvBookmark::Monday, 0x02);
    dayOfWeekCombo->setItemData(TvBookmark::Tuesday, 0x04);
    dayOfWeekCombo->setItemData(TvBookmark::Wednesday, 0x08);
    dayOfWeekCombo->setItemData(TvBookmark::Thursday, 0x10);
    dayOfWeekCombo->setItemData(TvBookmark::Friday, 0x20);
    dayOfWeekCombo->setItemData(TvBookmark::Saturday, 0x40);
    dayOfWeekCombo->setItemData(TvBookmark::Sunday, 0x80);
    dayOfWeekCombo->setItemData(TvBookmark::MondayToFriday, 0x3E);
    dayOfWeekCombo->setItemData(TvBookmark::SaturdayAndSunday, 0xC0);
    setColor(QColor(Qt::red));

    channelsCombo->addItem(tr("Any channel"));
    QList<TvChannel *> active = channelList->activeChannels();
    for (int index = 0; index < active.size(); ++index) {
        TvChannel *channel = active.at(index);
        if (!channel->isHidden())
            channelsCombo->addItem(channel->icon(), channel->name(), channel->id());
    }
    if (channelList->largeIcons())
        channelsCombo->setIconSize(QSize(64, 64));
    channelsCombo->setCurrentIndex(0);

    seasonEdit->setEnabled(false);

    connect(colorSelect, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(otherDay, SIGNAL(clicked()), this, SLOT(selectOtherDay()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
    connect(titleEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOk()));
    connect(seasonEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOk()));
    connect(seasonEnable, SIGNAL(toggled(bool)), seasonEdit, SLOT(setEnabled(bool)));
    connect(seasonEnable, SIGNAL(stateChanged(int)), this, SLOT(updateOk()));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

BookmarkItemEditor::~BookmarkItemEditor()
{
}

QString BookmarkItemEditor::channelId() const
{
    QVariant data = channelsCombo->itemData(channelsCombo->currentIndex());
    if (data.isValid())
        return data.toString();
    else
        return QString();
}

void BookmarkItemEditor::setChannelId(const QString &id)
{
    TvChannel *channel = m_channelList->channel(id);
    if (id.isEmpty()) {
        channelsCombo->setCurrentIndex(0);
    } else if (!channel) {
        // Unknown channel id - add it directly.  Could happen
        // because the data source has changed and now the
        // channel id's are different.
        channelsCombo->addItem(id, id);
        channelsCombo->setCurrentIndex(channelsCombo->count() - 1);
    } else {
        int size = channelsCombo->count();
        for (int index = 1; index < size; ++index) {
            QVariant data = channelsCombo->itemData(index);
            if (data.isValid() && data.toString() == id) {
                channelsCombo->setCurrentIndex(index);
                return;
            }
        }
        // Hidden channel - add it to the end of the list.
        channelsCombo->addItem(channel->name(), channel->id());
        channelsCombo->setCurrentIndex(channelsCombo->count() - 1);
    }
}

int BookmarkItemEditor::dayOfWeek() const
{
    int index = dayOfWeekCombo->currentIndex();
    if (index != m_extraItem)
        return index;
    else
        return TvBookmark::Mask;
}

int BookmarkItemEditor::dayOfWeekMask() const
{
    return dayOfWeekCombo->itemData(dayOfWeekCombo->currentIndex()).toInt();
}

void BookmarkItemEditor::setDayOfWeek(int value, int mask)
{
    if (value != TvBookmark::Mask) {
        dayOfWeekCombo->setCurrentIndex(value);
    } else {
        QString name = TvBookmark::dayOfWeekLongName(mask);
        if (m_extraItem != -1) {
            dayOfWeekCombo->setItemText(m_extraItem, name);
            dayOfWeekCombo->setItemData(m_extraItem, mask);
        } else {
            m_extraItem = dayOfWeekCombo->count();
            dayOfWeekCombo->addItem(name, mask);
        }
        dayOfWeekCombo->setCurrentIndex(m_extraItem);
    }
}

void BookmarkItemEditor::setColor(const QColor &color)
{
    m_color = color;
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    colorSelect->setIcon(QIcon(pixmap));
}

void BookmarkItemEditor::changeColor()
{
    QColor color = QColorDialog::getColor
        (m_color, this, tr("Bookmark highlight color"));
    if (color.isValid())
        setColor(color);
}

void BookmarkItemEditor::updateOk()
{
    bool ok = !titleEdit->text().isEmpty();
    if (ok && seasonEnable->isChecked()) {
        QString seasons = seasonEdit->text();
        if (!seasons.isEmpty()) {
            bool sok = false;
            TvBookmark::parseSeasons(seasons, &sok);
            if (!sok)
                ok = false;
        }
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

void BookmarkItemEditor::selectOtherDay()
{
    DaySelectionDialog dlg(this);
    dlg.setMask(dayOfWeekMask());
    if (dlg.exec() == QDialog::Accepted) {
        int mask = dlg.mask();
        int count = dayOfWeekCombo->count();
        for (int index = 0; index < count; ++index) {
            if (dayOfWeekCombo->itemData(index).toInt() == mask) {
                dayOfWeekCombo->setCurrentIndex(index);
                return;
            }
        }
        setDayOfWeek(TvBookmark::Mask, mask);
    }
}

void BookmarkItemEditor::help()
{
    HelpBrowser::showContextHelp(QLatin1String("bookmarks.html"), this);
}
