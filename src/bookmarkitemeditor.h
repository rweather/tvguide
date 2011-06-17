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

    // 0 = any day, 1..7 = Mon..Sun, 8 = Mon-Fri
    int dayOfWeek() const;
    void setDayOfWeek(int value);

    QColor color() const { return m_color; }
    void setColor(const QColor &color);

private Q_SLOTS:
    void changeColor();
    void titleChanged(const QString &text);

private:
    TvChannelList *m_channelList;
    QColor m_color;
};

#endif
