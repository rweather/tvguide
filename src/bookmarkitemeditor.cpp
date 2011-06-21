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
#include <QtGui/qcolordialog.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qicon.h>

BookmarkItemEditor::BookmarkItemEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
{
    setupUi(this);

    dayOfWeekCombo->setCurrentIndex(0);
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

    connect(colorSelect, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(titleEdit, SIGNAL(textChanged(QString)), this, SLOT(titleChanged(QString)));

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
    return dayOfWeekCombo->currentIndex();
}

void BookmarkItemEditor::setDayOfWeek(int value)
{
    dayOfWeekCombo->setCurrentIndex(value);
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

void BookmarkItemEditor::titleChanged(const QString &text)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}
