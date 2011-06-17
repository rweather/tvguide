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

#include "mainwindow.h"
#include "tvprogrammedelegate.h"
#include "channeleditor.h"
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtGui/qitemselectionmodel.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    action_Open->setShortcuts(QKeySequence::Open);
    action_Quit->setShortcuts(QKeySequence::Quit);
    actionReload->setShortcuts(QKeySequence::Refresh);
    actionNextDay->setShortcuts(QKeySequence::MoveToNextPage);
    actionPreviousDay->setShortcuts(QKeySequence::MoveToPreviousPage);
    actionNextWeek->setShortcuts(QKeySequence::MoveToNextWord);
    actionPreviousWeek->setShortcuts(QKeySequence::MoveToPreviousWord);

    actionToday->setEnabled(false); // Calendar starts on today.

    connect(action_Quit, SIGNAL(triggered()), this, SLOT(close()));

    m_progress = new QProgressBar(this);
    m_progress->setVisible(false);
    m_progress->setMinimum(0);
    m_progress->setMaximum(1000);
    m_progress->setValue(1000);

    statusbar->addPermanentWidget(m_progress, 0);

    m_channelList = new TvChannelList(this);
    connect(m_channelList, SIGNAL(busyChanged(bool)),
            this, SLOT(busyChanged(bool)));
    connect(m_channelList, SIGNAL(progressChanged(qreal)),
            this, SLOT(progressChanged(qreal)));
    connect(m_channelList, SIGNAL(programmesChanged(TvChannel *)),
            this, SLOT(programmesChanged(TvChannel *)));
    m_channelModel = new TvChannelModel(m_channelList, this);
    channels->setModel(m_channelModel);

    m_programmeModel = new TvProgrammeModel(this);
    programmes->setModel(m_programmeModel);
    programmes->verticalHeader()->hide();
    programmes->horizontalHeader()->setStretchLastSection(true);
    programmes->setSelectionBehavior(QTableView::SelectRows);
    programmes->setItemDelegate(new TvProgrammeDelegate(programmes));

    QTimer::singleShot(1000, m_channelList, SLOT(refreshChannels()));

    m_hideProgressTimer = new QTimer(this);
    m_hideProgressTimer->setSingleShot(true);
    connect(m_hideProgressTimer, SIGNAL(timeout()),
            m_progress, SLOT(hide()));

    connect(actionReload, SIGNAL(triggered()),
            m_channelList, SLOT(reload()));
    connect(action_Stop, SIGNAL(triggered()),
            m_channelList, SLOT(abort()));
    action_Stop->setEnabled(false);
    connect(actionToday, SIGNAL(triggered()),
            this, SLOT(showToday()));
    connect(actionNextDay, SIGNAL(triggered()),
            this, SLOT(showNextDay()));
    connect(actionPreviousDay, SIGNAL(triggered()),
            this, SLOT(showPreviousDay()));
    connect(actionNextWeek, SIGNAL(triggered()),
            this, SLOT(showNextWeek()));
    connect(actionPreviousWeek, SIGNAL(triggered()),
            this, SLOT(showPreviousWeek()));
    connect(actionMorning, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionAfternoon, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionNight, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionLateNight, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionEditChannels, SIGNAL(triggered()),
            this, SLOT(editChannels()));

    connect(calendar, SIGNAL(selectionChanged()),
            this, SLOT(dateChanged()));
    connect(channels->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(channelChanged(QModelIndex)));

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("TimePeriods"));
    actionMorning->setChecked(settings.value(QLatin1String("morning"), true).toBool());
    actionAfternoon->setChecked(settings.value(QLatin1String("afternoon"), true).toBool());
    actionNight->setChecked(settings.value(QLatin1String("night"), true).toBool());
    actionLateNight->setChecked(settings.value(QLatin1String("latenight"), true).toBool());
    settings.endGroup();
}

MainWindow::~MainWindow()
{
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));

    settings.beginGroup(QLatin1String("TimePeriods"));
    TvChannel::TimePeriods periods = timePeriods();
    settings.setValue(QLatin1String("morning"),
                      (periods & TvChannel::Morning) != 0);
    settings.setValue(QLatin1String("afternoon"),
                      (periods & TvChannel::Afternoon) != 0);
    settings.setValue(QLatin1String("night"),
                      (periods & TvChannel::Night) != 0);
    settings.setValue(QLatin1String("latenight"),
                      (periods & TvChannel::LateNight) != 0);
    settings.endGroup();

    settings.sync();
}

void MainWindow::busyChanged(bool value)
{
    actionReload->setEnabled(!value);
    action_Stop->setEnabled(value);
    if (value) {
        m_progress->setVisible(value);
        m_hideProgressTimer->stop();
    } else {
        m_hideProgressTimer->start(500);
    }
}

void MainWindow::progressChanged(qreal progress)
{
    if (!m_channelList->useSimpleProgress() || progress >= 1.0f) {
        m_progress->setMaximum(1000);
        m_progress->setValue(qRound(progress * 1000.0f));
    } else {
        m_progress->setValue(0);
        m_progress->setMaximum(0);
    }
}

void MainWindow::dateChanged()
{
    QDate date = calendar->selectedDate();
    actionToday->setEnabled(date != QDate::currentDate());
    setDay(channels->selectionModel()->currentIndex(), date);
}

void MainWindow::channelChanged(const QModelIndex &index)
{
    TvChannel *channel;
    if (index.isValid()) {
        channel = static_cast<TvChannel *>(index.internalPointer());
        QDate first, last;
        channel->dataForRange(&first, &last);
        calendar->setMinimumDate(first);
        calendar->setMaximumDate(last);
    } else {
        calendar->setMinimumDate(QDate::fromJulianDay(1));
        calendar->setMaximumDate(QDate(7999, 12, 31));
    }
    setDay(index, calendar->selectedDate());
}

void MainWindow::programmesChanged(TvChannel *channel)
{
    QModelIndex index = channels->selectionModel()->currentIndex();
    if (index.isValid()) {
        TvChannel *indexChannel = static_cast<TvChannel *>(index.internalPointer());
        if (channel == indexChannel) {
            QList<TvProgramme *> programmes;
            QDate date = calendar->selectedDate();
            programmes = channel->programmesForDay(date, timePeriods());
            m_programmeModel->setProgrammes(programmes, channel, date);
            this->programmes->resizeRowsToContents();
        }
    }
}

void MainWindow::showToday()
{
    calendar->setSelectedDate(QDate::currentDate());
}

void MainWindow::showNextDay()
{
    calendar->setSelectedDate(calendar->selectedDate().addDays(1));
}

void MainWindow::showPreviousDay()
{
    calendar->setSelectedDate(calendar->selectedDate().addDays(-1));
}

void MainWindow::showNextWeek()
{
    calendar->setSelectedDate(calendar->selectedDate().addDays(7));
}

void MainWindow::showPreviousWeek()
{
    calendar->setSelectedDate(calendar->selectedDate().addDays(-7));
}

void MainWindow::updateTimePeriods()
{
    setDay(channels->selectionModel()->currentIndex(), calendar->selectedDate());
}

void MainWindow::editChannels()
{
    ChannelEditor ce(m_channelList, this);
    ce.exec();
}

TvChannel::TimePeriods MainWindow::timePeriods() const
{
    TvChannel::TimePeriods periods(0);
    if (actionMorning->isChecked())
        periods |= TvChannel::Morning;
    if (actionAfternoon->isChecked())
        periods |= TvChannel::Afternoon;
    if (actionNight->isChecked())
        periods |= TvChannel::Night;
    if (actionLateNight->isChecked())
        periods |= TvChannel::LateNight;
    return periods;
}

void MainWindow::setDay(const QModelIndex &index, const QDate &date)
{
    TvChannel *channel;
    if (index.isValid()) {
        // Display the current programmes for the channel and day.
        channel = static_cast<TvChannel *>(index.internalPointer());
        QList<TvProgramme *> programmes;
        programmes = channel->programmesForDay(date, timePeriods());
        m_programmeModel->setProgrammes(programmes, channel, date);
        this->programmes->resizeRowsToContents();

        // Explicitly request and update of the data.
        m_channelList->requestChannelDay(channel, date);
    } else {
        channel = 0;
        m_programmeModel->clear();
    }
}
