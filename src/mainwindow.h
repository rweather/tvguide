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
#include <QtGui/qlineedit.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qtoolbutton.h>
#include "ui_mainwindow.h"
#include "tvchannellist.h"
#include "tvchannelmodel.h"

class HelpBrowser;
class ProgrammeView;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showContextHelp(const QString &name);

private Q_SLOTS:
    void busyChanged(bool value);
    void progressChanged(qreal progress);
    void dateChanged();
    void channelChanged();
    void programmeChanged();
    void programmesChanged(TvChannel *channel);
    void networkRequest(TvChannel *channel, const QDate &date, bool isIconFetch);

    void showToday();

    void showMorning();
    void showAfternoon();
    void showNight();
    void showLateNight();

    void updateTimePeriods();
    void sevenDayOutlookChanged();

    void editChannels();
    void editChannelGroups();
    void addBookmark();
    void organizeBookmarks();
    void tickShow();

    void selectService();
    void webSearch();

    void hiddenChannelsChanged();
    void channelIndexLoaded();
    void channelIconsChanged();
    void refineChannels();

    void help();
    void helpDeleted();

    void about();

    void zoomIn();
    void zoomOut();
    void zoomReset();
    void zoomUpdate();

    void searchFilterChanged(const QString &text);
    void selectSearchCategory();
    void selectSearchCredit();

    void toggleAdvancedSearch(bool value);

    void advancedSearchChanged();

    void recalculateChannelSpans();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    TvChannelList *m_channelList;
    TvChannelModel *m_channelModel;
    QProgressBar *m_progress;
    QTimer *m_hideProgressTimer;
    bool m_firstTimeChannelList;
    bool m_fetching;
    qreal m_baseFontSize;
    qreal m_fontMultiplier;
    HelpBrowser *m_helpBrowser;
    QLabel *m_searchLabel;
    QLineEdit *m_searchFilter;
    ProgrammeView *programmeView;
    QStringList m_preselectedChannels;

    TvBookmark::MatchOptions matchOptions() const;
    void setDay(const QList<TvChannel *> &channels, const QDate &date, TvChannel *changedChannel = 0, bool request = true);
    void updateProgrammes(TvChannel *channel, const QDate &date, bool request);
    void updateMultiChannelProgrammes(const QDate &date, const QList<TvChannel *> channels, bool request);
    QList<TvProgramme *> combineShowings(const QList<TvProgramme *> &programmes);

    void selectView();
    void clearView();
    TvProgramme *selectedProgramme() const;

    QList<TvChannel *> selectedChannels(const QModelIndexList &list) const;
    QList<TvChannel *> selectedChannels() const;
};

#endif
