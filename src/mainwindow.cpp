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
#include "bookmarkitemeditor.h"
#include "bookmarklisteditor.h"
#include "serviceselector.h"
#include "helpbrowser.h"
#include "ui_aboutdialog.h"
#include "websearchdialog.h"
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtGui/qitemselectionmodel.h>
#include <QtGui/qmessagebox.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qevent.h>
#include <QtGui/qtoolbutton.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_firstTimeChannelList(false)
    , m_fetching(false)
    , m_baseFontSize(12.0f)
    , m_fontMultiplier(1.0f)
    , m_helpBrowser(0)
{
    setupUi(this);

    action_Quit->setShortcuts(QKeySequence::Quit);
    actionReload->setShortcuts(QKeySequence::Refresh);
    actionNextDay->setShortcuts(QKeySequence::MoveToNextPage);
    actionPreviousDay->setShortcuts(QKeySequence::MoveToPreviousPage);
    actionNextWeek->setShortcuts(QKeySequence::MoveToNextWord);
    actionPreviousWeek->setShortcuts(QKeySequence::MoveToPreviousWord);
    actionZoomIn->setShortcuts(QKeySequence::ZoomIn);
    actionZoomOut->setShortcuts(QKeySequence::ZoomOut);

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
    connect(m_channelList, SIGNAL(bookmarksChanged()),
            this, SLOT(updateTimePeriods()));
    connect(m_channelList, SIGNAL(hiddenChannelsChanged()),
            this, SLOT(hiddenChannelsChanged()));
    connect(m_channelList, SIGNAL(channelIndexLoaded()),
            this, SLOT(channelIndexLoaded()));
    connect(m_channelList, SIGNAL(channelIconsChanged()),
            this, SLOT(channelIconsChanged()));
    connect(m_channelList, SIGNAL(networkRequest(TvChannel *, QDate, bool)),
            this, SLOT(networkRequest(TvChannel *, QDate, bool)));
    m_channelModel = new TvChannelModel(m_channelList, this);
    channels->setModel(m_channelModel);

    if (m_channelList->largeIcons()) {
        channels->setIconSize(QSize(32, 32));
        programmes->setIconSize(QSize(32, 32));
    } else {
        channels->setIconSize(QSize(16, 16));
        programmes->setIconSize(QSize(16, 16));
    }
    channels->verticalHeader()->hide();
    channels->horizontalHeader()->hide();
    channels->horizontalHeader()->setStretchLastSection(true);
    channels->setSelectionBehavior(QTableView::SelectRows);
    channels->setColumnHidden(TvChannelModel::ColumnNumber, true);
    channels->setColumnWidth(TvChannelModel::ColumnNumber, 70);

    m_programmeModel = new TvProgrammeModel(this);
    programmes->setModel(m_programmeModel);
    programmes->verticalHeader()->hide();
    programmes->horizontalHeader()->setStretchLastSection(true);
    programmes->setSelectionBehavior(QTableView::SelectRows);
    programmes->setItemDelegate(new TvProgrammeDelegate(programmes));
    programmes->setColumnHidden(TvProgrammeModel::ColumnDay, true);
    programmes->setColumnHidden(TvProgrammeModel::ColumnChannel, true);
    m_baseFontSize = programmes->font().pointSizeF();

    if (m_channelList->hasService())
        QTimer::singleShot(0, m_channelList, SLOT(refreshChannels()));
    else
        QTimer::singleShot(0, this, SLOT(selectService()));

    m_hideProgressTimer = new QTimer(this);
    m_hideProgressTimer->setSingleShot(true);
    connect(m_hideProgressTimer, SIGNAL(timeout()),
            m_progress, SLOT(hide()));

    connect(actionSelectService, SIGNAL(triggered()),
            this, SLOT(selectService()));
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
    connect(actionAddBookmark, SIGNAL(triggered()),
            this, SLOT(addBookmark()));
    connect(actionOrganizeBookmarks, SIGNAL(triggered()),
            this, SLOT(organizeBookmarks()));
    connect(action7DayOutlook, SIGNAL(toggled(bool)),
            this, SLOT(sevenDayOutlookChanged()));
    connect(actionShowPartialMatches, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionShowFailedMatches, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionTickShow, SIGNAL(triggered()),
            this, SLOT(tickShow()));
    connect(actionMultiChannel, SIGNAL(toggled(bool)),
            this, SLOT(multiChannelChanged()));
    connect(actionWebSearch, SIGNAL(triggered()),
            this, SLOT(webSearch()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(help()));
    connect(actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(actionZoomReset, SIGNAL(triggered()), this, SLOT(zoomReset()));
    connect(actionClearCache, SIGNAL(triggered()),
            m_channelList, SLOT(clearCache()));

    connect(calendar, SIGNAL(selectionChanged()),
            this, SLOT(dateChanged()));
    connect(channels->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(channelChanged(QModelIndex)));

    connect(programmes->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(programmeChanged(QModelIndex)));
    connect(m_programmeModel, SIGNAL(modelReset()),
            this, SLOT(programmesReset()));
    actionTickShow->setEnabled(false);

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("TimePeriods"));
    actionMorning->setChecked(settings.value(QLatin1String("morning"), true).toBool());
    actionAfternoon->setChecked(settings.value(QLatin1String("afternoon"), true).toBool());
    actionNight->setChecked(settings.value(QLatin1String("night"), true).toBool());
    actionLateNight->setChecked(settings.value(QLatin1String("latenight"), true).toBool());
    settings.endGroup();

    settings.beginGroup(QLatin1String("View"));
    m_fontMultiplier = qreal(settings.value(QLatin1String("zoom"), 1.0).toDouble());
    actionShowPartialMatches->setChecked(settings.value(QLatin1String("partial"), true).toBool());
    //actionShowFailedMatches->setChecked(settings.value(QLatin1String("failed"), false).toBool());
    actionShowFailedMatches->setChecked(true);
    action7DayOutlook->setChecked(settings.value(QLatin1String("outlook7days"), false).toBool());
    actionMultiChannel->setChecked(settings.value(QLatin1String("allchannels"), false).toBool());
    m_programmeModel->setFilterOptions(settings.value(QLatin1String("filteroptions"), TvProgramme::SearchTitle).toInt());
    settings.endGroup();

    toolBar->addSeparator();

    QToolButton *searchOptions = new QToolButton(this);
    searchOptions->setText(tr("Search"));
    searchOptions->setPopupMode(QToolButton::InstantPopup);
    toolBar->addWidget(searchOptions);

    QMenu *menu = new QMenu(this);
    menu->setTearOffEnabled(true);
    menu->addAction(actionSearchFilterTitle);
    menu->addAction(actionSearchFilterEpisodeName);
    menu->addAction(actionSearchFilterDescription);
    menu->addAction(actionSearchFilterCredits);
    menu->addAction(actionSearchFilterCategories);
    searchOptions->setMenu(menu);

    int options = m_programmeModel->filterOptions();
    actionSearchFilterTitle->setChecked
        ((options & TvProgramme::SearchTitle) != 0);
    actionSearchFilterEpisodeName->setChecked
        ((options & TvProgramme::SearchSubTitle) != 0);
    actionSearchFilterDescription->setChecked
        ((options & TvProgramme::SearchDescription) != 0);
    actionSearchFilterCredits->setChecked
        ((options & TvProgramme::SearchCredits) != 0);
    actionSearchFilterCategories->setChecked
        ((options & TvProgramme::SearchCategories) != 0);

    connect(actionSearchFilterTitle, SIGNAL(toggled(bool)),
            this, SLOT(searchFilterOptionsChanged()));
    connect(actionSearchFilterEpisodeName, SIGNAL(toggled(bool)),
            this, SLOT(searchFilterOptionsChanged()));
    connect(actionSearchFilterDescription, SIGNAL(toggled(bool)),
            this, SLOT(searchFilterOptionsChanged()));
    connect(actionSearchFilterCredits, SIGNAL(toggled(bool)),
            this, SLOT(searchFilterOptionsChanged()));
    connect(actionSearchFilterCategories, SIGNAL(toggled(bool)),
            this, SLOT(searchFilterOptionsChanged()));

    m_searchFilter = new QLineEdit(this);
    toolBar->addWidget(m_searchFilter);
    connect(m_searchFilter, SIGNAL(textChanged(QString)),
            this, SLOT(searchFilterChanged(QString)));

    zoomUpdate();
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

    settings.beginGroup(QLatin1String("View"));
    settings.setValue(QLatin1String("zoom"), m_fontMultiplier);
    settings.setValue(QLatin1String("partial"),
                      actionShowPartialMatches->isChecked());
    //settings.setValue(QLatin1String("failed"),
    //                  actionShowFailedMatches->isChecked());
    settings.remove(QLatin1String("failed"));
    settings.setValue(QLatin1String("outlook7days"),
                      action7DayOutlook->isChecked());
    settings.setValue(QLatin1String("allchannels"),
                      actionMultiChannel->isChecked());
    settings.setValue(QLatin1String("filteroptions"),
                      m_programmeModel->filterOptions());
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
        statusBar()->clearMessage();
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

void MainWindow::networkRequest(TvChannel *channel, const QDate &date, bool isIconFetch)
{
    if (channel) {
        if (isIconFetch)
            statusBar()->showMessage(tr("Fetching channel logo for %1 ...").arg(channel->name()));
        else
            statusBar()->showMessage(tr("Fetching %1, %2...").arg(channel->name(), date.toString(Qt::DefaultLocaleLongDate)));
    } else {
        statusBar()->showMessage(tr("Fetching channel list ..."));
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

void MainWindow::programmeChanged(const QModelIndex &index)
{
    actionTickShow->setEnabled(index.isValid());
}

void MainWindow::programmesReset()
{
    actionTickShow->setEnabled(false);
}

void MainWindow::programmesChanged(TvChannel *channel)
{
    if (m_fetching)
        return;
    QDate date = calendar->selectedDate();
    if (actionMultiChannel->isChecked()) {
        updateMultiChannelProgrammes(date, false);
    } else {
        QModelIndex index = channels->selectionModel()->currentIndex();
        if (index.isValid()) {
            TvChannel *indexChannel = static_cast<TvChannel *>(index.internalPointer());
            if (channel == indexChannel)
                updateProgrammes(channel, date, false);
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

void MainWindow::sevenDayOutlookChanged()
{
    programmes->setColumnHidden
        (TvProgrammeModel::ColumnDay, !action7DayOutlook->isChecked());
    updateTimePeriods();
}

void MainWindow::multiChannelChanged()
{
    bool multi = actionMultiChannel->isChecked();
    programmes->setColumnHidden
        (TvProgrammeModel::ColumnChannel, !multi);
    channels->setEnabled(!multi);
    if (multi)
        channelChanged(QModelIndex());
    else
        channelChanged(channels->selectionModel()->currentIndex());
}

void MainWindow::editChannels()
{
    ChannelEditor ce(m_channelList, this);
    ce.exec();
}

void MainWindow::addBookmark()
{
    BookmarkItemEditor bookmarkDlg(m_channelList, this);
    QModelIndex index = channels->selectionModel()->currentIndex();
    if (index.isValid()) {
        TvChannel *channel = static_cast<TvChannel *>(index.internalPointer());
        bookmarkDlg.setChannelId(channel->id());
    }
    index = programmes->selectionModel()->currentIndex();
    TvBookmark *editBookmark = 0;
    if (index.isValid()) {
        TvProgramme *programme = static_cast<TvProgramme *>(index.internalPointer());
        bookmarkDlg.setChannelId(programme->channel()->id());
        bookmarkDlg.setTitle(programme->title());
        bookmarkDlg.setStartTime(programme->start().time());
        bookmarkDlg.setStopTime(programme->stop().time());
        bookmarkDlg.setDayOfWeek(programme->start().date().dayOfWeek(), 0);
        int season = programme->season();
        if (season > 0)
            bookmarkDlg.setSeasons(QString::number(season));
        bookmarkDlg.setSeasonsEnabled(false);
        int year = programme->year();
        if (year > 0)
            bookmarkDlg.setYears(QString::number(year));
        bookmarkDlg.setYearsEnabled(false);
        editBookmark = programme->bookmark();
        if (editBookmark) {
            if (QMessageBox::question
                    (this, tr("Add Bookmark"),
                     tr("Do you wish to edit the existing bookmark?"),
                     QMessageBox::Yes | QMessageBox::No,
                     QMessageBox::Yes) == QMessageBox::Yes) {
                bookmarkDlg.copyFromBookmark(editBookmark);
            } else {
                editBookmark = 0;
            }
        }
    } else {
        bookmarkDlg.setDayOfWeek
            (calendar->selectedDate().dayOfWeek(), 0);
    }
    if (!editBookmark)
        bookmarkDlg.setWindowTitle(tr("Add Bookmark"));
    if (bookmarkDlg.exec() == QDialog::Accepted) {
        TvBookmark *bookmark = new TvBookmark();
        bookmarkDlg.copyToBookmark(bookmark);
        if (editBookmark)
            m_channelList->bookmarkList()->removeBookmark(editBookmark, false);
        m_channelList->bookmarkList()->addBookmark(bookmark);
    }
}

void MainWindow::organizeBookmarks()
{
    BookmarkListEditor dialog(m_channelList, this);
    dialog.exec();
}

void MainWindow::tickShow()
{
    QModelIndex index = programmes->selectionModel()->currentIndex();
    if (!index.isValid())
        return;
    TvProgramme *programme = static_cast<TvProgramme *>(index.internalPointer());
    if (programme->match() == TvBookmark::TickMatch)
        m_channelList->bookmarkList()->removeTick(programme);
    else
        m_channelList->bookmarkList()->addTick(programme);
    programme->refreshBookmark();
    m_programmeModel->updateTick(index.row());
}

void MainWindow::selectService()
{
    bool firstTime = !m_channelList->hasService();
    ServiceSelector dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_firstTimeChannelList = firstTime;
        m_programmeModel->clear();
        m_channelList->reloadService();
    }
}

void MainWindow::webSearch()
{
    WebSearchDialog searchDlg(this);
    QModelIndex index = programmes->selectionModel()->currentIndex();
    if (index.isValid()) {
        TvProgramme *programme = static_cast<TvProgramme *>(index.internalPointer());
        searchDlg.setSearchText(programme->title());
        searchDlg.addSearchItem(programme->title());
        if (!programme->subTitle().isEmpty()) {
            searchDlg.addSearchItem
                (programme->title() + QLatin1Char(' ') +
                 programme->subTitle());
        }
        searchDlg.addSearchItems(programme->actors());
        searchDlg.addSearchItems(programme->directors());
    }
    if (searchDlg.exec() == QDialog::Accepted) {
        searchDlg.saveSettings();
        QDesktopServices::openUrl(searchDlg.url());
    }
}

void MainWindow::hiddenChannelsChanged()
{
    if (m_channelList->largeIcons()) {
        channels->setIconSize(QSize(32, 32));
        programmes->setIconSize(QSize(32, 32));
    } else {
        channels->setIconSize(QSize(16, 16));
        programmes->setIconSize(QSize(16, 16));
    }
    channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
}

void MainWindow::channelIndexLoaded()
{
    if (m_firstTimeChannelList) {
        m_firstTimeChannelList = false;
        QTimer::singleShot(0, this, SLOT(refineChannels()));
    }
    channels->setColumnHidden
        (TvChannelModel::ColumnNumber, !m_channelList->haveChannelNumbers());
    channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
    updateTimePeriods();
}

void MainWindow::channelIconsChanged()
{
    channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
    if (actionMultiChannel->isChecked()) {
        m_programmeModel->updateIcons();
        programmes->resizeRowsToContents();
    }
}

void MainWindow::refineChannels()
{
    if (QMessageBox::question
            (this, tr("TV Guide"),
             tr("Tools->Edit Channels can be used to refine the "
                "list of channels that are shown.  Do you wish to "
                "do that now?"),
             QMessageBox::Yes | QMessageBox::No,
             QMessageBox::Yes) == QMessageBox::Yes)
        editChannels();
}

void MainWindow::showContextHelp(const QString &name)
{
    help();
    m_helpBrowser->setContextHelp(name);
}

void MainWindow::help()
{
    if (m_helpBrowser) {
        m_helpBrowser->activateWindow();
    } else {
        m_helpBrowser = new HelpBrowser();
        connect(m_helpBrowser, SIGNAL(destroyed()),
                this, SLOT(helpDeleted()));
        m_helpBrowser->show();
    }
}

void MainWindow::helpDeleted()
{
    m_helpBrowser = 0;
}

void MainWindow::about()
{
    QDialog dlg(this);
    Ui::AboutDialog about;
    about.setupUi(&dlg);
    about.version->setText(TVGUIDE_VERSION);
    QString name = m_channelList->serviceName();
    if (!name.isEmpty())
        about.serviceName->setText(name);
    dlg.exec();
}

void MainWindow::zoomIn()
{
    m_fontMultiplier += 0.25f;
    zoomUpdate();
}

void MainWindow::zoomOut()
{
    if (m_fontMultiplier > 0.25f)
        m_fontMultiplier -= 0.25f;
    zoomUpdate();
}

void MainWindow::zoomReset()
{
    m_fontMultiplier = 1.0f;
    zoomUpdate();
}

void MainWindow::zoomUpdate()
{
    actionZoomOut->setEnabled(m_fontMultiplier > 0.25f);
    actionZoomReset->setEnabled(m_fontMultiplier != 1.0f);

    QFont font(programmes->font());
    font.setPointSizeF(m_baseFontSize * m_fontMultiplier);
    programmes->setFont(font);
    programmes->resizeRowsToContents();
}

void MainWindow::searchFilterChanged(const QString &text)
{
    m_programmeModel->setFilter(text);
    programmes->resizeRowsToContents();
}

void MainWindow::searchFilterOptionsChanged()
{
    int options = m_programmeModel->filterOptions();

    if (actionSearchFilterTitle->isChecked())
        options |= TvProgramme::SearchTitle;
    else
        options &= ~TvProgramme::SearchTitle;

    if (actionSearchFilterEpisodeName->isChecked())
        options |= TvProgramme::SearchSubTitle;
    else
        options &= ~TvProgramme::SearchSubTitle;

    if (actionSearchFilterDescription->isChecked())
        options |= TvProgramme::SearchDescription;
    else
        options &= ~TvProgramme::SearchDescription;

    if (actionSearchFilterCredits->isChecked())
        options |= TvProgramme::SearchCredits;
    else
        options &= ~TvProgramme::SearchCredits;

    if (actionSearchFilterCategories->isChecked())
        options |= TvProgramme::SearchCategories;
    else
        options &= ~TvProgramme::SearchCategories;

    m_programmeModel->setFilterOptions(options);
    programmes->resizeRowsToContents();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Equal &&
            (event->modifiers() & Qt::ControlModifier) != 0) {
        // Ctrl/= is an alias for Ctrl/+ for convenience.
        zoomIn();
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
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

TvBookmark::MatchOptions MainWindow::matchOptions() const
{
    TvBookmark::MatchOptions options(0);
    if (actionShowPartialMatches->isChecked())
        options |= TvBookmark::PartialMatches;
    if (actionShowFailedMatches->isChecked())
        options |= TvBookmark::NonMatching;
    return options;
}

void MainWindow::setDay(const QModelIndex &index, const QDate &date)
{
    TvChannel *channel;
    if (actionMultiChannel->isChecked()) {
        updateMultiChannelProgrammes(date, true);
    } else {
        if (index.isValid()) {
            channel = static_cast<TvChannel *>(index.internalPointer());
            updateProgrammes(channel, date, true);
        } else {
            channel = 0;
            m_programmeModel->clear();
        }
    }
}

void MainWindow::updateProgrammes
    (TvChannel *channel, const QDate &date, bool request)
{
    // Display the current programmes for the channel and day.
    QList<TvProgramme *> programmes;
    m_fetching = true;
    if (action7DayOutlook->isChecked()) {
        if (request)
            m_channelList->requestChannelDay(channel, date, 7);
        programmes = channel->bookmarkedProgrammes
            (date, date.addDays(6), matchOptions());
    } else {
        if (request)
            m_channelList->requestChannelDay(channel, date);
        programmes = channel->programmesForDay(date, timePeriods(), matchOptions());
    }
    programmes = combineShowings(programmes);
    m_programmeModel->setProgrammes(programmes, channel, date);
    this->programmes->resizeRowsToContents();
    m_fetching = false;
}

static bool sortByStartTimeAndChannel(TvProgramme *p1, TvProgramme *p2)
{
    if (p1->start() < p2->start())
        return true;
    else if (p1->start() > p2->start())
        return false;
    return p1->channel()->compare(p2->channel()) < 0;
}

void MainWindow::updateMultiChannelProgrammes
    (const QDate &date, bool request)
{
    // Collect up the programmes for all channels on this date.
    QList<TvProgramme *> programmes;
    QList<TvChannel *> channels = m_channelList->activeChannels();
    m_fetching = true;
    for (int index = 0; index < channels.size(); ++index) {
        TvChannel *channel = channels.at(index);
        if (channel->isHidden())
            continue;
        if (action7DayOutlook->isChecked()) {
            if (request)
                m_channelList->requestChannelDay(channel, date, 7, index == 0);
            programmes += channel->bookmarkedProgrammes
                (date, date.addDays(6), matchOptions());
        } else {
            if (request)
                m_channelList->requestChannelDay(channel, date, 1, index == 0);
            programmes += channel->programmesForDay(date, timePeriods(), matchOptions());
        }
    }

    // Combine multiple showings of the same episode.
    programmes = combineShowings(programmes);

    // Sort the programmes to intermix the channel data and
    // then display them.
    qSort(programmes.begin(), programmes.end(), sortByStartTimeAndChannel);
    m_programmeModel->setProgrammes(programmes, 0, date);
    this->programmes->resizeRowsToContents();
    m_fetching = false;
}

QList<TvProgramme *> MainWindow::combineShowings
    (const QList<TvProgramme *> &programmes)
{
    int index, index2;
    QList<TvProgramme *> newProgrammes;
    TvProgramme *prog;

    // Clear all "other showings" indications.
    for (index = 0; index < programmes.size(); ++index) {
        prog = programmes.at(index);
        prog->setSuppressed(false);
    }

    // Filter the list in 7-day mode to combine the showings.
    if (action7DayOutlook->isChecked()) {
        for (index = 0; index < programmes.size(); ++index) {
            TvProgramme *prog = programmes.at(index);
            if (prog->isSuppressed())
                continue;   // We've already combined this programme.
            TvBookmark *bookmark = prog->bookmark();
            if (!bookmark || prog->match() == TvBookmark::ShouldMatch || prog->match() == TvBookmark::TickMatch || prog->subTitle().isEmpty()) {
                // Failed, ticked, or no episode title - there will be no other showings.
                newProgrammes.append(prog);
                continue;
            }
            for (index2 = index + 1; index2 < programmes.size(); ++index2) {
                TvProgramme *prog2 = programmes.at(index2);
                if (prog2->isSuppressed() || prog2->bookmark() != bookmark || prog2->match() == TvBookmark::TickMatch || prog2->match() == TvBookmark::ShouldMatch || prog2->subTitle().isEmpty())
                    continue;
                if (prog->subTitle() != prog2->subTitle() ||
                        prog->episodeNumber() != prog2->episodeNumber())
                    continue;
                if (prog2->match() != TvBookmark::TitleMatch &&
                        prog->match() == TvBookmark::TitleMatch) {
                    // The new programme is a better candidate for the
                    // "primary match" amongst the showings.
                    prog = 0;
                    break;
                } else {
                    prog2->setSuppressed(true);
                }
            }
            if (prog)
                newProgrammes.append(prog);
        }
        return newProgrammes;
    } else {
        return programmes;
    }
}
