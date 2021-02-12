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

#include "bookmarkitemeditor.h"
#include "tvchannellist.h"
#include "dayselectiondialog.h"
#include "helpbrowser.h"
#include <QtWidgets/qcolordialog.h>
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
        channelsCombo->setIconSize(QSize(32, 32));
    else
        channelsCombo->setIconSize(QSize(16, 16));
    channelsCombo->setCurrentIndex(0);

    seasonEdit->setEnabled(false);
    yearEdit->setEnabled(false);

    connect(redButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));
    connect(greenButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));
    connect(blueButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));
    connect(orangeButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));
    connect(purpleButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));

    connect(otherDay, SIGNAL(clicked()), this, SLOT(selectOtherDay()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
    connect(titleEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOk()));
    connect(seasonEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOk()));
    connect(seasonEnable, SIGNAL(toggled(bool)), seasonEdit, SLOT(setEnabled(bool)));
    connect(seasonEnable, SIGNAL(stateChanged(int)), this, SLOT(updateOk()));
    connect(yearEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOk()));
    connect(yearEnable, SIGNAL(toggled(bool)), yearEdit, SLOT(setEnabled(bool)));
    connect(yearEnable, SIGNAL(stateChanged(int)), this, SLOT(updateOk()));

    connect(anyTimeCheck, SIGNAL(toggled(bool)), this, SLOT(anyTimeChanged(bool)));

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

static int colorDiff(const QColor &color, int r, int g, int b)
{
    int diff = (color.red() - r) * (color.red() - r);
    diff += (color.green() - g) * (color.green() - g);
    diff += (color.blue() - b) * (color.blue() - b);
    return diff;
}

void BookmarkItemEditor::setColor(const QColor &color)
{
    static int const colors[] = {
        0xFF, 0x00, 0x00,
        0x00, 0xAA, 0x00,
        0x00, 0x00, 0xFF,
        0xFF, 0xAA, 0x00,
        0xFF, 0x00, 0x7F
    };
    QRadioButton *buttons[] = {
        redButton,
        greenButton,
        blueButton,
        orangeButton,
        purpleButton
    };
    int closest = 0;
    int closestDiff = colorDiff(color, colors[0], colors[1], colors[2]);
    for (int index = 1; index < 5; ++index) {
        int diff = colorDiff(color, colors[index * 3], colors[index * 3 + 1], colors[index * 3 + 2]);
        if (diff < closestDiff) {
            closestDiff = diff;
            closest = index;
        }
    }
    buttons[closest]->setChecked(true);
    m_color = QColor::fromRgb(colors[closest * 3], colors[closest * 3 + 1], colors[closest * 3 + 2]);
}

void BookmarkItemEditor::copyFromBookmark(const TvBookmark *bookmark)
{
    setTitle(bookmark->title());
    setChannelId(bookmark->channelId());
    setStartTime(bookmark->startTime());
    setStopTime(bookmark->stopTime());
    setAnyTime(bookmark->anyTime());
    setDayOfWeek (bookmark->dayOfWeek(), bookmark->dayOfWeekMask());
    setColor(bookmark->color());
    setSeasons(bookmark->seasons());
    setSeasonsEnabled(!bookmark->seasonList().isEmpty());
    setYears(bookmark->years());
    setYearsEnabled(!bookmark->yearList().isEmpty());
    setOnAir(bookmark->isOnAir());
}

void BookmarkItemEditor::copyToBookmark(TvBookmark *bookmark)
{
    bookmark->setTitle(title());
    bookmark->setChannelId(channelId());
    bookmark->setStartTime(startTime());
    bookmark->setStopTime(stopTime());
    bookmark->setAnyTime(anyTime());
    bookmark->setDayOfWeekMask(dayOfWeekMask());
    bookmark->setColor(color());
    if (seasonsEnabled())
        bookmark->setSeasons(seasons());
    else
        bookmark->setSeasons(QString());
    if (yearsEnabled())
        bookmark->setYears(years());
    else
        bookmark->setYears(QString());
    bookmark->setOnAir(isOnAir());
}

void BookmarkItemEditor::changeColor()
{
    if (redButton->isChecked())
        m_color = QColor::fromRgb(0xFF, 0x00, 0x00);
    else if (greenButton->isChecked())
        m_color = QColor::fromRgb(0x00, 0xAA, 0x00);
    else if (blueButton->isChecked())
        m_color = QColor::fromRgb(0x00, 0x00, 0xFF);
    else if (orangeButton->isChecked())
        m_color = QColor::fromRgb(0xFF, 0xAA, 0x00);
    else if (purpleButton->isChecked())
        m_color = QColor::fromRgb(0xFF, 0x00, 0x7F);
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
    if (ok && yearEnable->isChecked()) {
        QString years = yearEdit->text();
        if (!years.isEmpty()) {
            bool sok = false;
            TvBookmark::parseSeasons(years, &sok);
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

void BookmarkItemEditor::anyTimeChanged(bool value)
{
    startTimeEdit->setEnabled(!value);
    stopTimeEdit->setEnabled(!value);
}
