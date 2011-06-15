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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QtGui/qmainwindow.h>
#include <QtGui/qprogressbar.h>
#include "ui_mainwindow.h"
#include "tvchannellist.h"
#include "tvchannelmodel.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void busyChanged(bool value);
    void progressChanged(qreal progress);
    void dateChanged();
    void channelChanged(const QModelIndex &index);

    void showToday();
    void showNextDay();
    void showPreviousDay();
    void showNextWeek();
    void showPreviousWeek();

private:
    TvChannelList *m_channelList;
    TvChannelModel *m_channelModel;
    QProgressBar *m_progress;
    QTimer *m_hideProgressTimer;

    void setDay(const QModelIndex &index, const QDate &date);
};

#endif
