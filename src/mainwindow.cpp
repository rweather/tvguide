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
#include "categoryselector.h"
#include "channeleditor.h"
#include "channelgroupeditor.h"
#include "bookmarkitemeditor.h"
#include "bookmarklisteditor.h"
#include "programmeview.h"
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
#include <QtGui/qtexttable.h>
#include <QtGui/qfiledialog.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_firstTimeChannelList(false)
    , m_fetching(false)
    , m_baseFontSize(12.0f)
    , m_fontMultiplier(1.0f)
    , m_helpBrowser(0)
{
    setupUi(this);

    setWindowIcon(QIcon(QLatin1String(":images/tvlogo72x72.png")));

    programmeView = new ProgrammeView();
    viewStack->addWidget(programmeView);

    action_Quit->setShortcuts(QKeySequence::Quit);
    actionReload->setShortcuts(QKeySequence::Refresh);
    actionZoomIn->setShortcuts(QKeySequence::ZoomIn);
    actionZoomOut->setShortcuts(QKeySequence::ZoomOut);

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

    connect(m_channelModel, SIGNAL(modelReset()), this,
            SLOT(recalculateChannelSpans()));

    if (m_channelList->largeIcons())
        channels->setIconSize(QSize(32, 32));
    else
        channels->setIconSize(QSize(16, 16));
    channels->verticalHeader()->hide();
    channels->horizontalHeader()->hide();
    channels->horizontalHeader()->setStretchLastSection(true);
    channels->setSelectionBehavior(QTableView::SelectRows);
    channels->setColumnHidden(TvChannelModel::ColumnNumber, true);
    channels->setColumnWidth(TvChannelModel::ColumnNumber, 70);

    m_baseFontSize = programmeView->font().pointSizeF();

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
    connect(actionMorning, SIGNAL(triggered()),
            this, SLOT(showMorning()));
    connect(actionAfternoon, SIGNAL(triggered()),
            this, SLOT(showAfternoon()));
    connect(actionNight, SIGNAL(triggered()),
            this, SLOT(showNight()));
    connect(actionLateNight, SIGNAL(triggered()),
            this, SLOT(showLateNight()));
    connect(actionEditChannels, SIGNAL(triggered()),
            this, SLOT(editChannels()));
    connect(actionChannelGroups, SIGNAL(triggered()),
            this, SLOT(editChannelGroups()));
    connect(actionAddBookmark, SIGNAL(triggered()),
            this, SLOT(addBookmark()));
    connect(actionOrganizeBookmarks, SIGNAL(triggered()),
            this, SLOT(organizeBookmarks()));
    connect(actionExportBookmarks, SIGNAL(triggered()),
            this, SLOT(exportBookmarks()));
    connect(actionImportBookmarks, SIGNAL(triggered()),
            this, SLOT(importBookmarks()));
    connect(action7DayOutlook, SIGNAL(toggled(bool)),
            this, SLOT(sevenDayOutlookChanged()));
    connect(actionShowPartialMatches, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionShowFailedMatches, SIGNAL(toggled(bool)),
            this, SLOT(updateTimePeriods()));
    connect(actionTickShow, SIGNAL(triggered()),
            this, SLOT(tickShow()));
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
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(channelChanged()));

    connect(programmeView, SIGNAL(selectionChanged()),
            this, SLOT(programmeChanged()));
    actionTickShow->setEnabled(false);

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("View"));
    m_fontMultiplier = qreal(settings.value(QLatin1String("zoom"), 1.0).toDouble());
    actionShowPartialMatches->setChecked(settings.value(QLatin1String("partial"), true).toBool());
    //actionShowFailedMatches->setChecked(settings.value(QLatin1String("failed"), false).toBool());
    actionShowFailedMatches->setChecked(true);
    action7DayOutlook->setChecked(settings.value(QLatin1String("outlook7days"), false).toBool());
    m_preselectedChannels = settings.value(QLatin1String("selectedChannels")).toStringList();
    settings.endGroup();

    toolBar->addSeparator();

    m_searchLabel = new QLabel(this);
    m_searchLabel->setText(tr("Quick Search:"));
    toolBar->addWidget(m_searchLabel);

    m_searchFilter = new QLineEdit(this);
    toolBar->addWidget(m_searchFilter);
    connect(m_searchFilter, SIGNAL(textChanged(QString)),
            this, SLOT(searchFilterChanged(QString)));

    toolBar->addAction(actionAdvancedSearch);
    connect(actionAdvancedSearch, SIGNAL(toggled(bool)),
            this, SLOT(toggleAdvancedSearch(bool)));
    advancedSearchDock->setVisible(false);

    connect(titleEdit, SIGNAL(textChanged(QString)),
            this, SLOT(advancedSearchChanged()));
    connect(episodeNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(advancedSearchChanged()));
    connect(descriptionEdit, SIGNAL(textChanged(QString)),
            this, SLOT(advancedSearchChanged()));
    connect(creditEdit, SIGNAL(textChanged(QString)),
            this, SLOT(advancedSearchChanged()));
    connect(categoryEdit, SIGNAL(textChanged(QString)),
            this, SLOT(advancedSearchChanged()));
    connect(andButton, SIGNAL(toggled(bool)),
            this, SLOT(advancedSearchChanged()));

    connect(categoryChoose, SIGNAL(clicked()),
            this, SLOT(selectSearchCategory()));
    connect(creditChoose, SIGNAL(clicked()),
            this, SLOT(selectSearchCredit()));

    zoomUpdate();

    selectView();
}

MainWindow::~MainWindow()
{
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));

    settings.beginGroup(QLatin1String("View"));
    settings.setValue(QLatin1String("zoom"), m_fontMultiplier);
    settings.setValue(QLatin1String("partial"),
                      actionShowPartialMatches->isChecked());
    //settings.setValue(QLatin1String("failed"),
    //                  actionShowFailedMatches->isChecked());
    settings.remove(QLatin1String("failed"));
    settings.setValue(QLatin1String("outlook7days"),
                      action7DayOutlook->isChecked());
    settings.remove(QLatin1String("allchannels"));
    settings.remove(QLatin1String("filteroptions"));
    settings.setValue(QLatin1String("selectedChannels"),
                      m_channelModel->itemListToIds
                          (channels->selectionModel()->selectedRows()));
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
    setDay(selectedChannels(), date);
}

void MainWindow::channelChanged()
{
    QList<TvChannel *> channels;
    m_preselectedChannels = QStringList();
    channels = selectedChannels();
    if (channels.size() != 0) {
        TvChannel *channel = channels.at(0);
        QDate first, last;
        channel->dataForRange(&first, &last);
        for (int index = 1; index < channels.size(); ++index) {
            channel = channels.at(index);
            QDate cfirst, clast;
            channel->dataForRange(&cfirst, &clast);
            if (cfirst < first)
                first = cfirst;
            if (clast > last)
                last = clast;
        }
        calendar->setMinimumDate(first);
        calendar->setMaximumDate(last);
    } else {
        calendar->setMinimumDate(QDate::fromJulianDay(1));
        calendar->setMaximumDate(QDate(7999, 12, 31));
    }
    setDay(channels, calendar->selectedDate());
}

void MainWindow::programmeChanged()
{
    actionTickShow->setEnabled(selectedProgramme() != 0);
}

void MainWindow::programmesChanged(TvChannel *channel)
{
    if (m_fetching)
        return;
    setDay(selectedChannels(), calendar->selectedDate(), channel, false);
}

void MainWindow::showToday()
{
    calendar->setSelectedDate(QDate::currentDate());
}

void MainWindow::showMorning()
{
    programmeView->scrollToTime(QTime(6, 0));
}

void MainWindow::showAfternoon()
{
    programmeView->scrollToTime(QTime(12, 0));
}

void MainWindow::showNight()
{
    programmeView->scrollToTime(QTime(18, 0));
}

void MainWindow::showLateNight()
{
    programmeView->scrollToTime(QTime(0, 0));
}

void MainWindow::updateTimePeriods()
{
    setDay(selectedChannels(), calendar->selectedDate());
}

void MainWindow::sevenDayOutlookChanged()
{
    selectView();
    updateTimePeriods();
}

void MainWindow::editChannels()
{
    ChannelEditor ce(m_channelList, this);
    ce.exec();
}

void MainWindow::editChannelGroups()
{
    ChannelGroupEditor ce(m_channelList, this);
    QModelIndexList selected = channels->selectionModel()->selectedRows();
    if (!selected.isEmpty())
        ce.setActiveGroup(m_channelModel->groupForIndex(selected.at(0)));
    ce.exec();
}

void MainWindow::addBookmark()
{
    BookmarkItemEditor bookmarkDlg(m_channelList, this);
    QList<TvChannel *> channels = selectedChannels();
    if (!channels.isEmpty())
        bookmarkDlg.setChannelId(channels.at(0)->id());
    TvProgramme *programme = selectedProgramme();
    TvBookmark *editBookmark = 0;
    if (programme) {
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

void MainWindow::exportBookmarks()
{
    QString filename = QFileDialog::getSaveFileName
        (this, tr("Export Bookmarks"), QString(),
         tr("TV Bookmarks (*.tvb)"));
    if (filename.isEmpty())
        return;
    m_channelList->bookmarkList()->exportBookmarks(filename);
}

void MainWindow::importBookmarks()
{
    QString filename = QFileDialog::getOpenFileName
        (this, tr("Import Bookmarks"), QString(),
         tr("TV Bookmarks (*.tvb)"));
    if (filename.isEmpty())
        return;
    switch (m_channelList->bookmarkList()->importBookmarks(filename)) {
    case TvBookmarkList::Import_OK:
        if (QMessageBox::question
                (this, tr("Import Bookmarks"),
                 tr("New bookmarks have been imported.  Do you wish to "
                    "edit the bookmark list now?"),
                 QMessageBox::Yes | QMessageBox::No,
                 QMessageBox::Yes) == QMessageBox::Yes)
            organizeBookmarks();
        break;
    case TvBookmarkList::Import_NothingNew:
        QMessageBox::information
            (this, tr("Import Bookmarks"),
             tr("No new bookmarks found for importing."));
        break;
    case TvBookmarkList::Import_CannotOpen:
    case TvBookmarkList::Import_BadFormat:
        QMessageBox::critical
            (this, tr("Import Bookmarks"),
             tr("Could not import the bookmarks from %1.").arg(filename));
        break;
    case TvBookmarkList::Import_WrongService:
        QMessageBox::critical
            (this, tr("Import Bookmarks"),
             tr("Bookmarks are not applicable to the current guide service."));
        break;
    }
}

void MainWindow::tickShow()
{
    TvProgramme *programme = selectedProgramme();
    if (!programme)
        return;
    if (programme->match() == TvBookmark::TickMatch)
        m_channelList->bookmarkList()->removeTick(programme);
    else
        m_channelList->bookmarkList()->addTick(programme);
    programme->refreshBookmark();
    programmeView->updateSelection();
}

void MainWindow::selectService()
{
    bool firstTime = !m_channelList->hasService();
    ServiceSelector dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_firstTimeChannelList = firstTime;
        clearView();
        m_channelList->reloadService();
    }
}

void MainWindow::webSearch()
{
    WebSearchDialog searchDlg(this);
    TvProgramme *programme = selectedProgramme();
    if (programme) {
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
    if (m_channelList->largeIcons())
        channels->setIconSize(QSize(32, 32));
    else
        channels->setIconSize(QSize(16, 16));
    //channels->resizeRowsToContents();
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
    //channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
    if (!m_preselectedChannels.isEmpty()) {
        // Automatically select the channels that were loaded from
        // the configuration settings at startup.
        QModelIndexList list = m_channelModel->itemListFromIds(m_preselectedChannels);
        m_preselectedChannels = QStringList();
        QItemSelection selection;
        for (int index = 0; index < list.size(); ++index) {
            int row = list.at(index).row();
            selection.select(m_channelModel->index(row, 0, QModelIndex()),
                             m_channelModel->index(row, TvChannelModel::ColumnCount - 1, QModelIndex()));
        }
        if (!list.isEmpty())
            channels->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
    updateTimePeriods();
}

void MainWindow::channelIconsChanged()
{
    //channels->resizeRowsToContents();
    channels->resizeColumnsToContents();
    programmeView->updateIcons();
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

    QFont font(programmeView->font());
    font.setPointSizeF(m_baseFontSize * m_fontMultiplier);
    programmeView->setFont(font);
}

void MainWindow::searchFilterChanged(const QString &text)
{
    programmeView->setFilter(text);
    selectView();
}

void MainWindow::selectSearchCategory()
{
    CategorySelector selector(this);
    selector.setCategories(m_channelList->categories());
    if (selector.exec() == QDialog::Accepted)
        categoryEdit->setText(selector.selectedCategory());
}

void MainWindow::selectSearchCredit()
{
    CategorySelector selector(this);
    selector.setWindowTitle(tr("Select Credit"));
    selector.setCategories(m_channelList->credits());
    if (selector.exec() == QDialog::Accepted)
        creditEdit->setText(selector.selectedCategory());
}

void MainWindow::toggleAdvancedSearch(bool value)
{
    m_searchLabel->setEnabled(!value);
    m_searchFilter->setEnabled(!value);
    advancedSearchDock->setVisible(value);
    if (value) {
        titleEdit->setFocus();
        advancedSearchChanged();
    } else {
        programmeView->setAdvancedFilter(0);
        selectView();
    }
}

void MainWindow::advancedSearchChanged()
{
    TvProgrammeFilter *filter = new TvProgrammeFilter();
    filter->setTitle(titleEdit->text());
    filter->setEpisodeName(episodeNameEdit->text());
    filter->setDescription(descriptionEdit->text());
    filter->setCredit(creditEdit->text());
    filter->setCategory(categoryEdit->text());
    if (andButton->isChecked())
        filter->setCombineMode(TvProgrammeFilter::CombineAnd);
    else
        filter->setCombineMode(TvProgrammeFilter::CombineOr);
    programmeView->setAdvancedFilter(filter);
    selectView();
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

TvBookmark::MatchOptions MainWindow::matchOptions() const
{
    TvBookmark::MatchOptions options(0);
    //if (actionShowPartialMatches->isChecked())
        options |= TvBookmark::PartialMatches;
    if (actionShowFailedMatches->isChecked())
        options |= TvBookmark::NonMatching;
    return options;
}

void MainWindow::setDay(const QList<TvChannel *> &channels, const QDate &date, TvChannel *changedChannel, bool request)
{
    if (channels.size() == 1) {
        TvChannel *channel = channels.at(0);
        if (!changedChannel || channel == changedChannel)
            updateProgrammes(channel, date, request);
    } else if (channels.size() > 1) {
        if (!changedChannel || channels.contains(changedChannel))
            updateMultiChannelProgrammes(date, channels, request);
    } else {
        clearView();
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
        programmes = channel->programmesForDay(date, matchOptions());
    }
    programmes = combineShowings(programmes);
    m_fetching = false;
    if (action7DayOutlook->isChecked())
        programmeView->setProgrammes(date, programmes, ProgrammeView::BookmarksSingleChannel);
    else
        programmeView->setProgrammes(date, programmes, ProgrammeView::SingleDaySingleChannel);
}

static bool sortTimeAndChannel(TvProgramme *p1, TvProgramme *p2)
{
    if (p1->start() < p2->start())
        return true;
    else if (p1->start() > p2->start())
        return false;
    return p1->channel()->compare(p2->channel()) < 0;
}

void MainWindow::updateMultiChannelProgrammes
    (const QDate &date, const QList<TvChannel *> channels, bool request)
{
    // Collect up the programmes for all channels on this date.
    QList<TvProgramme *> programmes;
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
            programmes += channel->programmesForDay(date, matchOptions());
        }
    }

    // Combine multiple showings of the same episode.
    programmes = combineShowings(programmes);

    // Sort the programmes in 7 days mode to interleave the channels.
    if (action7DayOutlook->isChecked())
        qSort(programmes.begin(), programmes.end(), sortTimeAndChannel);

    // Set the programme list on the view.
    m_fetching = false;
    if (action7DayOutlook->isChecked())
        programmeView->setProgrammes(date, programmes, ProgrammeView::BookmarksMultiChannel);
    else
        programmeView->setMultiChannelProgrammes(date, programmes, ProgrammeView::SingleDayMultiChannel);
}

QList<TvProgramme *> MainWindow::combineShowings
    (const QList<TvProgramme *> &programmes)
{
    int index, index2;
    QList<TvProgramme *> newProgrammes;
    TvProgramme *prog;

    // Filter the list in 7-day mode to combine the showings.
    if (action7DayOutlook->isChecked()) {
        for (index = 0; index < programmes.size(); ++index) {
            prog = programmes.at(index);
            prog->setSuppressed(false);
        }
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

void MainWindow::selectView()
{
    viewStack->setCurrentIndex(0);
}

void MainWindow::clearView()
{
    programmeView->setProgrammes(QDate(), QList<TvProgramme *>(), ProgrammeView::SingleDaySingleChannel);
}

TvProgramme *MainWindow::selectedProgramme() const
{
    return programmeView->selectedProgramme();
}

static bool sortChannels(TvChannel *c1, TvChannel *c2)
{
    return c1->compare(c2) < 0;
}

QList<TvChannel *> MainWindow::selectedChannels(const QModelIndexList &list) const
{
    QList<TvChannel *> channels;
    for (int posn = 0; posn < list.size(); ++posn) {
        QList<TvChannel *> indexChannels =
            m_channelModel->channelsForIndex(list.at(posn));
        for (int index = 0; index < indexChannels.size(); ++index) {
            TvChannel *channel = indexChannels.at(index);
            if (!channels.contains(channel))
                channels.append(channel);
        }
    }
    qSort(channels.begin(), channels.end(), sortChannels);
    return channels;
}

QList<TvChannel *> MainWindow::selectedChannels() const
{
    return selectedChannels(channels->selectionModel()->selectedRows());
}

void MainWindow::recalculateChannelSpans()
{
    int count = m_channelModel->groupCount() + 1; // Extra for "All Channels"
    channels->clearSpans();
    while (count-- > 0)
        channels->setSpan(count, 0, 1, TvChannelModel::ColumnCount);
}
