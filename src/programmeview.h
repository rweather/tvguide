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

#ifndef _PROGRAMMEVIEW_H
#define _PROGRAMMEVIEW_H

#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qstyleoption.h>
#include <QtCore/qvector.h>
#include "tvprogramme.h"
#include "tvprogrammefilter.h"

class ProgrammeHeaderView;

class ProgrammeView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit ProgrammeView(QWidget *parent = 0);
    ~ProgrammeView();

    enum Mode {
        SingleDaySingleChannel,
        SingleDayMultiChannel,
        BookmarksSingleChannel,
        BookmarksMultiChannel
    };

    Mode mode() const { return m_mode; }

    void setProgrammes(const QDate &date, const QList<TvProgramme *> &programmes, Mode mode);
    void setMultiChannelProgrammes(const QDate &date, const QList<TvProgramme *> &programmes, Mode mode);

    TvProgramme *selectedProgramme() const { return m_selection.prog; }
    void updateSelection();

    void scrollToTime(const QTime &time);

    void updateIcons();

    QString filter() const { return m_filter; }
    void setFilter(const QString &str);

    TvProgrammeFilter *advancedFilter() const { return m_advancedFilter; }
    void setAdvancedFilter(TvProgrammeFilter *filter);

Q_SIGNALS:
    void selectionChanged();

protected:
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    friend class ProgrammeHeaderView;

    struct ProgInfo
    {
        ProgInfo(TvProgramme *programme)
            : prog(programme)
            , document(0)
            , enabled(true)
            , dayNumber(0)
            {}
        TvProgramme *prog;
        QTextDocument *document;
        QRectF rect;
        bool enabled;
        short dayNumber;
    };
    struct ColumnInfo
    {
        ColumnInfo() {}
        ~ColumnInfo();

        QList<ProgInfo> programmes;
        QVector<qreal> hourOffsets;
        QRectF columnRect;
    };
    struct Selection
    {
        Selection() : column(-1), row(-1), prog(0) {}
        int column;
        int row;
        TvProgramme *prog;

        bool operator==(const Selection &other) const
        {
            return prog == other.prog;
        }
        bool operator!=(const Selection &other) const
        {
            return prog != other.prog;
        }
    };
    QList<ColumnInfo *> m_columns;
    QList<ColumnInfo *> m_activeColumns;
    QSize m_timeSize;
    QSize m_channelNameSize;
    QRectF m_totalRect;
    bool m_is24HourClock;
    int m_columnWidth;
    int m_columnSpacing;
    int m_actualColumnWidth;
    int m_options;
    int m_savedScrollTime;
    Mode m_mode;
    QDate m_startDate;
    ProgrammeHeaderView *m_headerView;
    QImage m_tickIcon;
    QImage m_returnedIcon;
    Selection m_selection;
    TvProgramme *m_prevSelection;
    QString m_filter;
    TvProgrammeFilter *m_advancedFilter;
    QList<qreal> m_dayHeights;
    QList<qreal> m_dayOffsets;

    enum DisplayMode
    {
        FullDay,
        SubsetOfDay,
        Bookmarks
    };
    DisplayMode m_displayMode;

    void paintFullDay(QPainter *painter);
    void paintSearchResults(QPainter *painter);
    void paintBookmarks(QPainter *painter);

    void writeColumn(ColumnInfo *column, int columnIndex);
    void writeProgramme(ProgInfo *info, int index);

    void clearColumns();
    void layout();
    void layoutColumns(bool shortcut);
    void layoutHeaderView();
    void levelHours();
    qreal calculateDayHeights();
    void updateScrollBars();

    static int adjustTimeSpan(QTime *start, QTime *stop, int posn);
    void drawTimeSpan(QPainter *painter, const QRectF &rect, const QTime &start, const QTime &stop, int posn);
    void drawChannelName(QPainter *painter, const QRectF &rect, TvProgramme *prog);
    static qreal yOffset(const ColumnInfo *column, int timeValue);

    Selection programmeAtPosition(const QPoint &pos) const;

    void applyFilter();
    bool hasFilter() const { return !m_filter.isEmpty() || (m_advancedFilter != 0 && !m_advancedFilter->isDefault()); }

    bool hasCollisions(const ColumnInfo *column, const TvProgramme *prog, int offset) const;
};

class ProgrammeHeaderView : public QWidget
{
    Q_OBJECT
public:
    ProgrammeHeaderView(ProgrammeView *view);
    ~ProgrammeHeaderView();

    int bestHeight();

protected:
    void paintEvent(QPaintEvent *event);

private:
    ProgrammeView *m_view;

    void drawSection(QPainter *painter, const QRect &rect, int index,
                     const QString &text, const QIcon &icon,
                     QStyleOptionHeader::SectionPosition position);
    void drawPadding(QPainter *painter, const QRect &rect);
};

#endif
