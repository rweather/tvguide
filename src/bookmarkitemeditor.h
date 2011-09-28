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

#ifndef _BOOKMARKITEMEDITOR_H
#define _BOOKMARKITEMEDITOR_H

#include <QtGui/qdialog.h>
#include "ui_bookmarkitemeditor.h"

class TvChannelList;
class TvBookmark;

class BookmarkItemEditor : public QDialog, private Ui::BookmarkItemEditor
{
    Q_OBJECT
public:
    explicit BookmarkItemEditor(TvChannelList *channelList, QWidget *parent = 0);
    ~BookmarkItemEditor();

    QString title() const { return titleEdit->text(); }
    void setTitle(const QString &title) { titleEdit->setText(title); }

    QString channelId() const;
    void setChannelId(const QString &id);

    QTime startTime() const { return startTimeEdit->time(); }
    void setStartTime(const QTime &time) { startTimeEdit->setTime(time); }

    QTime stopTime() const { return stopTimeEdit->time(); }
    void setStopTime(const QTime &time) { stopTimeEdit->setTime(time); }

    bool anyTime() const { return anyTimeCheck->isChecked(); }
    void setAnyTime(bool value) { anyTimeCheck->setChecked(value); }

    int dayOfWeek() const;
    int dayOfWeekMask() const;
    void setDayOfWeek(int value, int mask);

    QColor color() const { return m_color; }
    void setColor(const QColor &color);

    QString seasons() const { return seasonEdit->text(); }
    void setSeasons(const QString &seasons) { seasonEdit->setText(seasons); }

    bool seasonsEnabled() const { return seasonEnable->isChecked(); }
    void setSeasonsEnabled(bool value) { seasonEnable->setChecked(value); }

    QString years() const { return yearEdit->text(); }
    void setYears(const QString &year) { yearEdit->setText(year); }

    bool yearsEnabled() const { return yearEnable->isChecked(); }
    void setYearsEnabled(bool value) { yearEnable->setChecked(value); }

    bool isOnAir() const { return onAirButton->isChecked(); }
    void setOnAir(bool value)
    {
        if (value)
            onAirButton->setChecked(true);
        else
            offAirButton->setChecked(true);
    }

    void copyFromBookmark(const TvBookmark *bookmark);
    void copyToBookmark(TvBookmark *bookmark);

private Q_SLOTS:
    void changeColor();
    void updateOk();
    void selectOtherDay();
    void help();
    void anyTimeChanged(bool value);

private:
    TvChannelList *m_channelList;
    QColor m_color;
    int m_extraItem;
};

#endif
