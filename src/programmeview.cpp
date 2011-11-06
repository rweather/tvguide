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

#include "programmeview.h"
#include "tvchannel.h"
#include <QtGui/qtextdocument.h>
#include <QtGui/qtexttable.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtGui/qtooltip.h>
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>

#define MINUTES_PER_DAY     (24 * 60)
#define MINUTES_OF_6AM      (6 * 60)
#define MINUTES_SINGLE_STEP 15

ProgrammeView::ProgrammeView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , m_columnWidth(200)
    , m_columnSpacing(2)
    , m_actualColumnWidth(200)
    , m_options(TvProgramme::Write_Short)
    , m_savedScrollTime(0)
    , m_mode(ProgrammeView::SingleDaySingleChannel)
    , m_tickIcon(QLatin1String(":/images/tick.png"))
    , m_returnedIcon(QLatin1String(":/images/ledred.png"))
    , m_prevSelection(0)
    , m_advancedFilter(0)
    , m_displayMode(ProgrammeView::FullDay)
{
    m_tickIcon = m_tickIcon.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_returnedIcon = m_returnedIcon.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Does the system have a 24 hour clock?
    QString str = QTime(23, 0).toString(Qt::SystemLocaleShortDate);
    int value = 0;
    for (int index = 0; index < str.length(); ++index) {
        int ch = str.at(index).unicode();
        if (ch >= '0' && ch <= '9')
            value = value * 10 + ch - '0';
        else if (ch != ' ')
            break;
    }
    m_is24HourClock = (value == 23);

    // Construct the header view.
    m_headerView = new ProgrammeHeaderView(this);
    layoutHeaderView();

    updateScrollBars();

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
            m_headerView, SLOT(update()));

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("View"));
    QString scrollTime = settings.value(QLatin1String("scrolltime")).toString();
    if (!scrollTime.isEmpty()) {
        QTime time = QTime::fromString(scrollTime, QLatin1String("hh:mm"));
        int hour = time.hour();
        if (hour < 6)
            hour += 24;
        int timeValue = (hour - 6) * 60 + time.minute();
        verticalScrollBar()->setValue(timeValue);
    }
    settings.endGroup();
}

ProgrammeView::~ProgrammeView()
{
    clearColumns();

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("View"));
    int timeValue;
    if (m_displayMode == FullDay)
        timeValue = verticalScrollBar()->value() + MINUTES_OF_6AM;
    else
        timeValue = m_savedScrollTime + MINUTES_OF_6AM;
    int hour = timeValue / 60;
    if (hour >= 24)
        hour -= 24;
    QTime time(hour, timeValue % 60);
    settings.setValue(QLatin1String("scrolltime"), time.toString(QLatin1String("hh:mm")));
    settings.endGroup();
    settings.sync();

    delete m_advancedFilter;
}

void ProgrammeView::setProgrammes(const QDate &date, const QList<TvProgramme *> &programmes, Mode mode)
{
    // Clear the current columns.
    m_prevSelection = m_selection.prog;
    m_mode = mode;
    m_startDate = date;
    clearColumns();

    // Write all of the programme details to a single column.
    m_options = TvProgramme::Write_Short;
    if (!programmes.isEmpty()) {
        ColumnInfo *column = new ColumnInfo();
        for (int index = 0; index < programmes.size(); ++index) {
            ProgInfo info(programmes.at(index));
            info.dayNumber = date.daysTo(info.prog->start().date());
            column->programmes.append(info);
        }
        writeColumn(column, 0);
        m_columns.append(column);
    }

    // Lay out the view and update it.
    m_prevSelection = 0;
    layout();
    if (m_selection.prog)
        emit selectionChanged();
}

static bool sortByChannelAndStartTime(TvProgramme *p1, TvProgramme *p2)
{
    int cmp = p1->channel()->compare(p2->channel());
    if (cmp != 0)
        return cmp < 0;
    return p1->start() < p2->start();
}

void ProgrammeView::setMultiChannelProgrammes(const QDate &date, const QList<TvProgramme *> &programmes, Mode mode)
{
    // Clear the current columns.
    m_prevSelection = m_selection.prog;
    m_mode = mode;
    m_startDate = date;
    clearColumns();

    // Break the list up into columns and format them.
    m_options = TvProgramme::Write_EpisodeName | TvProgramme::Write_StarRating;
    if (mode == BookmarksMultiChannel)
        m_options |= TvProgramme::Write_OtherShowings;
    int start = 0;
    int end;
    while (start < programmes.size()) {
        TvChannel *channel = programmes.at(start)->channel();
        end = start + 1;
        while (end < programmes.size() && programmes.at(end)->channel() == channel)
            ++end;
        ColumnInfo *column = new ColumnInfo();
        for (int index = start; index < end; ++index) {
            ProgInfo info(programmes.at(index));
            info.dayNumber = date.daysTo(info.prog->start().date());
            column->programmes.append(info);
        }
        writeColumn(column, m_columns.size());
        m_columns.append(column);
        start = end;
    }

    // Lay out the view and update it.
    m_prevSelection = 0;
    layout();
    if (m_selection.prog)
        emit selectionChanged();
}

void ProgrammeView::updateSelection()
{
    if (m_selection.prog) {
        ProgInfo &info = m_columns.at(m_selection.column)->programmes[m_selection.row];
        writeProgramme(&info, m_selection.row);
        layout();
    }
}

void ProgrammeView::scrollToTime(const QTime &time)
{
    int timeValue = time.minute() - MINUTES_OF_6AM;
    int hour = time.hour();
    if (hour < 6)
        timeValue += (hour + 24) * 60;
    else
        timeValue += hour * 60;
    if (m_displayMode == FullDay)
        verticalScrollBar()->setValue(timeValue);
    else
        m_savedScrollTime = timeValue;
}

void ProgrammeView::updateIcons()
{
    m_headerView->update();
}

void ProgrammeView::setFilter(const QString &str)
{
    if (m_filter != str) {
        m_filter = str;
        if (!m_advancedFilter)
            applyFilter();
    }
}

void ProgrammeView::setAdvancedFilter(TvProgrammeFilter *filter)
{
    if (m_advancedFilter != filter) {
        delete m_advancedFilter;
        m_advancedFilter = filter;
        applyFilter();
    }
}

bool ProgrammeView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange)
        layout();
    return QAbstractScrollArea::event(event);
}

void ProgrammeView::resizeEvent(QResizeEvent *)
{
    layoutColumns(true);
    layoutHeaderView();
}

bool ProgrammeView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
        TvProgramme *prog = programmeAtPosition(helpEvent->pos()).prog;
        if (prog) {
            QToolTip::showText
                (helpEvent->globalPos(), prog->longDescription());
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QAbstractScrollArea::viewportEvent(event);
}

void ProgrammeView::mousePressEvent(QMouseEvent *event)
{
    Selection selection = programmeAtPosition(event->pos());
    if (selection != m_selection) {
        m_selection = selection;
        viewport()->update();
        event->accept();
        emit selectionChanged();
        return;
    }
    QWidget::mousePressEvent(event);
}

static inline int minuteIndexForProgramme(TvProgramme *prog, int posn)
{
    QTime time = prog->start().time();
    int hour = time.hour();
    if (hour < 6 && posn != 0) {
        // Programme has wrapped around past midnight.
        hour += 24;
    }
    int value = hour * 60 + time.minute();
    if (value < MINUTES_OF_6AM) {
        // Early morning shows are aligned with 6am.
        value = MINUTES_OF_6AM;
    }
    return value;
}

void ProgrammeView::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());
    if (m_displayMode == FullDay)
        paintFullDay(&painter);
    else if (m_displayMode == Bookmarks)
        paintBookmarks(&painter);
    else
        paintSearchResults(&painter);
}

void ProgrammeView::paintFullDay(QPainter *painter)
{
    // Consult the vertical scroll bar to determine which minute
    // of the day we should display at the top of the window.
    int timeValue = verticalScrollBar()->value() + MINUTES_OF_6AM;

    // Align the target time with the scrollbar step value.
    if ((timeValue % MINUTES_SINGLE_STEP) < ((MINUTES_SINGLE_STEP + 1) / 2))
        timeValue -= timeValue % MINUTES_SINGLE_STEP;
    else
        timeValue += MINUTES_SINGLE_STEP - (timeValue % MINUTES_SINGLE_STEP);
    int offsetHour = timeValue / 60 - 6;
    qreal offsetProgress = (timeValue % 60) / 60.0;

    // Render the columns, offset to show the specified minute.
    QPen linePen(palette().mid(), 0);
    for (int index = 0; index < m_columns.size(); ++index) {
        // Skip the column if it is obviously empty or off-screen.
        const ColumnInfo *column = m_columns.at(index);
        if (column->programmes.isEmpty())
            continue;
        qreal x = column->columnRect.left() -
                  horizontalScrollBar()->value();
        if ((x + column->columnRect.width()) <= 0 || x >= width())
            continue;

        // Draw the column at the offset for this hour range.
        qreal y;
        if (offsetHour >= 24) {
            y = column->hourOffsets.at(offsetHour);
        } else {
            y = (1 - offsetProgress) * column->hourOffsets.at(offsetHour) +
                offsetProgress * column->hourOffsets.at(offsetHour + 1);
        }
        painter->save();
        painter->translate(x, -y);
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            const ProgInfo &info = column->programmes.at(index2);
            QRectF timeRect(0, info.rect.y(),
                            m_timeSize.width(), info.rect.height());
            drawTimeSpan(painter, timeRect, info.prog->start().time(),
                         info.prog->stop().time(), index2);
            painter->translate(info.rect.x(), info.rect.y());
            info.document->drawContents(painter);
            painter->translate(-info.rect.x(), -info.rect.y());
            painter->setPen(linePen);
            painter->drawLine
                (QPointF(info.rect.left(), info.rect.bottom() - 1),
                 QPointF(info.rect.right() - 1, info.rect.bottom() - 1));
            if (info.prog == m_selection.prog) {
                painter->setPen(QPen(palette().highlight(), 2));
                painter->drawRect(info.rect);
            }
        }
        painter->restore();
    }
}

void ProgrammeView::paintSearchResults(QPainter *painter)
{
    QPen linePen(palette().mid(), 0);
    qreal columnx = 0;
    for (int index = 0; index < m_activeColumns.size(); ++index) {
        // Skip the column if it is obviously empty or off-screen.
        const ColumnInfo *column = m_activeColumns.at(index);
        if (column->programmes.isEmpty())
            continue;
        qreal x = columnx - horizontalScrollBar()->value();
        columnx += column->columnRect.width() + m_columnSpacing;
        if ((x + column->columnRect.width()) <= 0 || x >= width())
            continue;

        // Draw the column at the vertical scrollbar offset.
        painter->save();
        painter->translate(x, -verticalScrollBar()->value());
        qreal y = 0;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            const ProgInfo &info = column->programmes.at(index2);
            if (!info.enabled)
                continue;
            QRectF timeRect(0, y, m_timeSize.width(), info.rect.height());
            drawTimeSpan(painter, timeRect, info.prog->start().time(),
                         info.prog->stop().time(), index2);
            painter->translate(info.rect.x(), y);
            info.document->drawContents(painter);
            painter->translate(-info.rect.x(), -y);
            painter->setPen(linePen);
            painter->drawLine
                (QPointF(info.rect.left(), y + info.rect.height() - 1),
                 QPointF(info.rect.right() - 1, y + info.rect.height() - 1));
            if (info.prog == m_selection.prog) {
                QRectF rect(info.rect.x(), y,
                            info.rect.width(), info.rect.height());
                painter->setPen(QPen(palette().highlight(), 2));
                painter->drawRect(rect);
            }
            y += info.rect.height();
        }
        painter->restore();
    }
}

void ProgrammeView::paintBookmarks(QPainter *painter)
{
    QPen linePen(palette().mid(), 0);
    qreal columnx = 0;
    for (int index = 0; index < m_activeColumns.size(); ++index) {
        // Skip the column if it is obviously empty or off-screen.
        const ColumnInfo *column = m_activeColumns.at(index);
        if (column->programmes.isEmpty())
            continue;
        qreal x = columnx - horizontalScrollBar()->value();
        columnx += column->columnRect.width() + m_columnSpacing;
        if ((x + column->columnRect.width()) <= 0 || x >= width())
            continue;

        // Draw the column at the vertical scrollbar offset.
        painter->save();
        painter->translate(x, -verticalScrollBar()->value());
        qreal y = 0;
        int dayNumber = -1;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            const ProgInfo &info = column->programmes.at(index2);
            if (!info.enabled)
                continue;
            if (info.dayNumber != dayNumber) {
                dayNumber = info.dayNumber;
                y = m_dayOffsets[dayNumber];
            }
            QRectF timeRect(0, y, m_timeSize.width(), info.rect.height());
            drawTimeSpan(painter, timeRect, info.prog->start().time(),
                         info.prog->stop().time(), index2);
            painter->translate(info.rect.x(), y);
            info.document->drawContents(painter);
            painter->translate(-info.rect.x(), -y);
            painter->setPen(linePen);
            painter->drawLine
                (QPointF(info.rect.left(), y + info.rect.height() - 1),
                 QPointF(info.rect.right() - 1, y + info.rect.height() - 1));
            if (info.prog == m_selection.prog) {
                QRectF rect(info.rect.x(), y,
                            info.rect.width(), info.rect.height());
                painter->setPen(QPen(palette().highlight(), 2));
                painter->drawRect(rect);
            }
            y += info.rect.height();
        }
        painter->restore();
    }
    QFontMetrics metrics(font(), this);
    qreal headerHeight = metrics.height() + 5;
    QPen textPen(palette().buttonText(), 0);
    for (int index = 0; index < m_dayHeights.size(); ++index) {
        // Draw the day names between each block of bookmarks.
        if (m_dayHeights.at(index) == 0)
            continue;
        QRectF rect(0, m_dayOffsets.at(index) - headerHeight -
                            verticalScrollBar()->value(),
                    viewport()->width(), headerHeight);
        QDate date = m_startDate.addDays(index);
        QString text = date.toString(Qt::DefaultLocaleLongDate);
        painter->fillRect(rect, palette().button());
        painter->setPen(linePen);
        painter->drawLine(rect.left(), rect.y(), rect.right(), rect.y());
        painter->setPen(textPen);
        painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine, text);
    }
}

void ProgrammeView::writeColumn(ColumnInfo *column, int columnIndex)
{
    bool filter = hasFilter();
    for (int index = 0; index < column->programmes.size(); ++index) {
        ProgInfo &info = column->programmes[index];
        writeProgramme(&info, index);
        if (filter) {
            if (m_advancedFilter) {
                info.enabled = m_advancedFilter->match(info.prog);
            } else {
                info.enabled = info.prog->containsSearchString
                    (m_filter, TvProgramme::SearchAll);
            }
        } else {
            info.enabled = true;
        }
        if (m_prevSelection == info.prog && info.enabled) {
            m_selection.column = columnIndex;
            m_selection.row = index;
            m_selection.prog = info.prog;
        }
    }
}

void ProgrammeView::writeProgramme(ProgInfo *info, int index)
{
    delete info->document;
    info->document = new QTextDocument();
    info->document->setDocumentMargin(0);
    QTextCursor cursor(info->document);
    int opts = m_options;
    if (info->prog->isMovie())
        opts |= TvProgramme::Write_Date | TvProgramme::Write_Actor | TvProgramme::Write_Category;
    if (!index && info->prog->start().time() < QTime(6, 0)) {
        if (m_mode != BookmarksSingleChannel &&
                m_mode != BookmarksMultiChannel) {
            opts |= TvProgramme::Write_Continued;
        }
    }
    TvBookmark::Match match = info->prog->match();
    TvBookmark *bookmark = info->prog->bookmark();
    if (match == TvBookmark::TickMatch) {
        cursor.insertImage(m_tickIcon, QString());
    } else if (bookmark && !bookmark->isOnAir() &&
               match != TvBookmark::ShouldMatch) {
        cursor.insertImage(m_returnedIcon, QString());
    }
    info->prog->writeShortDescription(&cursor, opts);
}

void ProgrammeView::clearColumns()
{
    qDeleteAll(m_columns);
    m_columns.clear();
    m_activeColumns.clear();
    m_dayHeights.clear();
    m_dayOffsets.clear();
    if (m_selection.prog) {
        m_selection = Selection();
        emit selectionChanged();
    }
}

void ProgrammeView::layout()
{
    layoutColumns(false);
    layoutHeaderView();
    viewport()->update();
    m_headerView->update();
}

void ProgrammeView::layoutColumns(bool shortcut)
{
    int index;
    int columnWidth = m_columnWidth;
    if (!m_columns.isEmpty()) {
        int tempWidth = viewport()->width() / m_columns.size();
        if (tempWidth > columnWidth)
            columnWidth = tempWidth;
    }
    if (shortcut && m_actualColumnWidth == columnWidth)
        return;     // Don't need to re-layout the columns for a resize.
    m_actualColumnWidth = columnWidth;
    QFontMetrics metrics(font(), this);
    QRect bounds = metrics.boundingRect(QLatin1String(" 00:00 "));
    m_timeSize = bounds.size();
    int detailsOffset = m_timeSize.width() + m_columnSpacing;
    int detailsWidth = columnWidth - detailsOffset;
    qreal x = 0;
    qreal maxHeight = 0;
    qreal maxFilteredHeight = 0;
    qreal filteredWidth = 0;
    m_activeColumns.clear();
    bool bookmarksOnly = (m_mode == BookmarksSingleChannel ||
                          m_mode == BookmarksMultiChannel);
    for (index = 0; index < m_columns.size(); ++index) {
        // Lay out the columns across the viewing area.
        ColumnInfo *column = m_columns.at(index);
        qreal height = 0;
        qreal filteredHeight = 0;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            // Lay out the programme details.
            ProgInfo &info = column->programmes[index2];
            info.document->setDefaultFont(font());
            info.document->setTextWidth(detailsWidth);
            QSizeF size = info.document->size();

            // Derive a minimum height from the number of minutes
            // that are covered by the programme.  One line of text
            // is equated with 15 minutes of time.  We don't do this
            // if there is only one column or in bookmarks-only mode.
            qreal detailsHeight;
            if (m_columns.size() > 1 && !bookmarksOnly) {
                QTime start = info.prog->start().time();
                QTime stop = info.prog->stop().time();
                int length = adjustTimeSpan(&start, &stop, index2);
                detailsHeight = length / 900.0f * m_timeSize.height();
                if (detailsHeight < size.height())
                    detailsHeight = size.height();
            } else {
                detailsHeight = size.height();
            }
            detailsHeight += 4; // Extra spacing between rows.

            // Calculate the bounding rectangle for the details.
            info.rect = QRectF(detailsOffset, height, size.width(), detailsHeight);
            height += info.rect.height();

            // Calculate the filtered height of this column.
            if (info.enabled)
                filteredHeight += info.rect.height();
        }
        column->columnRect = QRectF(x, 0, columnWidth, height);
        if (height > maxHeight)
            maxHeight = height;
        if (filteredHeight > maxFilteredHeight)
            maxFilteredHeight = filteredHeight;
        x += columnWidth + m_columnSpacing;
        if (filteredHeight > 0) {
            m_activeColumns.append(column);
            filteredWidth += columnWidth + m_columnSpacing;
        }
    }
    if (bookmarksOnly)
        m_totalRect = QRectF(0, 0, filteredWidth, calculateDayHeights());
    else if (hasFilter())
        m_totalRect = QRectF(0, 0, filteredWidth, maxFilteredHeight);
    else
        m_totalRect = QRectF(0, 0, x, maxHeight);
    levelHours();
    updateScrollBars();
}

void ProgrammeView::layoutHeaderView()
{
    int headerHeight = m_headerView->bestHeight();
    QRect viewportRect = viewport()->geometry();
    setViewportMargins(0, headerHeight, 0, 0);
    m_headerView->setGeometry
        (viewportRect.left(), viewportRect.top() - headerHeight,
         viewportRect.width(), headerHeight);
}

// Level off the hour offsets so that certain times are exactly
// aligned and other times will interpolate the offsets.
void ProgrammeView::levelHours()
{
    int index, hour;
    QVector<double> minOffsets;
    QVector<int> minColumns;

    if (m_columns.isEmpty())
        return;

    // Calculate the offsets of major time points: 6am, 12pm, 6pm, 12am
    minOffsets.resize(25);
    minColumns.resize(25);
    for (index = 0; index < m_columns.size(); ++index) {
        ColumnInfo *column = m_columns.at(index);
        column->hourOffsets.resize(25);
        for (hour = 0; hour <= 24; hour += 6) {
            qreal y = yOffset(column, (hour + 6) * 60);
            column->hourOffsets[hour] = y;
            if (index) {
                if (minOffsets[hour] > y) {
                    minOffsets[hour] = y;
                    minColumns[hour] = index;
                }
            } else {
                minOffsets[hour] = y;
                minColumns[hour] = index;
            }
        }
    }

    // For two hours before each major time point except 6am,
    // we want to align all columns at the time point.
    // For example, between 10am and 12pm, the 12pm line will
    // be aligned across all columns.
    for (hour = 4; hour < 24; hour += 6) {
        index = minColumns[hour + 2];
        ColumnInfo *column = m_columns.at(index);
        qreal y1 = yOffset(column, (hour + 6) * 60);
        qreal y2 = minOffsets[hour + 2];
        qreal diff = y2 - y1;
        for (index = 0; index < m_columns.size(); ++index) {
            ColumnInfo *column = m_columns.at(index);
            qreal y = column->hourOffsets[hour + 2];
            column->hourOffsets[hour] = y - diff;
            column->hourOffsets[hour + 1] = y - diff / 2;
        }
    }

    // Fill in the remaining offsets by straight interpolation.
    for (hour = 0; hour <= 24; ++hour) {
        int subhour = hour % 6;
        if (subhour == 0 || subhour == 4 || subhour == 5)
            continue;
        for (index = 0; index < m_columns.size(); ++index) {
            ColumnInfo *column = m_columns.at(index);
            qreal y1 = column->hourOffsets[hour - subhour];
            qreal y2 = column->hourOffsets[hour - subhour + 4];
            column->hourOffsets[hour] = y1 + (y2 - y1) * subhour / 4;
        }
    }
}

qreal ProgrammeView::calculateDayHeights()
{
    m_dayHeights.clear();
    m_dayOffsets.clear();
    for (int index = 0; index < 10; ++index)
        m_dayHeights.append(0.0f);
    for (int index = 0; index < m_columns.size(); ++index) {
        const ColumnInfo *column = m_columns.at(index);
        qreal height = 0.0f;
        int dayNumber = 0;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            const ProgInfo &info = column->programmes.at(index2);
            if (!info.enabled)
                continue;
            if (info.dayNumber != dayNumber) {
                m_dayHeights[dayNumber] =
                    qMax(m_dayHeights[dayNumber], height);
                dayNumber = info.dayNumber;
                height = 0;
            }
            height += info.rect.height();
        }
        m_dayHeights[dayNumber] = qMax(m_dayHeights[dayNumber], height);
    }
    QFontMetrics metrics(font(), this);
    qreal headerHeight = metrics.height() + 5;
    qreal y = 0;
    for (int index = 0; index < 10; ++index) {
        if (m_dayHeights[index] != 0)
            y += headerHeight;
        m_dayOffsets.append(y);
        y += m_dayHeights[index];
    }
    return y;
}

void ProgrammeView::updateScrollBars()
{
    // Determine the display mode for the view.
    DisplayMode displayMode;
    int horizontalValue = horizontalScrollBar()->value();
    int verticalValue = verticalScrollBar()->value();
    if (m_mode == BookmarksSingleChannel ||
            m_mode == BookmarksMultiChannel) {
        displayMode = Bookmarks;
    } else if (hasFilter()) {
        displayMode = SubsetOfDay;
    } else {
        displayMode = FullDay;
    }
    if (m_displayMode != displayMode) {
        if (displayMode == FullDay) {
            verticalValue = m_savedScrollTime;
        } else if (m_displayMode == FullDay) {
            m_savedScrollTime = verticalValue;
            verticalValue = 0;
        }
        m_displayMode = displayMode;
        horizontalValue = 0;
    }

    // Update the scroll bars to match the current display mode.
    horizontalScrollBar()->setSingleStep(m_actualColumnWidth / 10);
    horizontalScrollBar()->setPageStep(m_actualColumnWidth + m_columnSpacing);
    horizontalScrollBar()->setRange(0, m_totalRect.width() - viewport()->width());
    if (displayMode != FullDay) {
        verticalScrollBar()->setSingleStep(20);
        verticalScrollBar()->setPageStep(viewport()->height() - 20);
        verticalScrollBar()->setRange(0, m_totalRect.height() - viewport()->height());
    } else {
        verticalScrollBar()->setSingleStep(MINUTES_SINGLE_STEP);
        verticalScrollBar()->setPageStep(60 * 4);
        verticalScrollBar()->setRange(0, MINUTES_PER_DAY - 60);
    }
    horizontalScrollBar()->setValue(horizontalValue);
    verticalScrollBar()->setValue(verticalValue);
}

ProgrammeView::ColumnInfo::~ColumnInfo()
{
    for (int index = 0; index < programmes.size(); ++index)
        delete programmes.at(index).document;
}

// Adjusts the time span of a programme so that it always starts
// after 6am and always stops before 6am.  Returns the number of
// seconds in the span after it has been adjusted.
int ProgrammeView::adjustTimeSpan(QTime *start, QTime *stop, int posn)
{
    QTime sixam(6, 0);
    if (*start < sixam) {
        if (*stop >= sixam) {
            if (!posn)
                *start = sixam;
            else
                *stop = sixam;
        }
    } else if (*stop <= *start) {
        if (*stop >= sixam)
            *stop = sixam;
    }
    if (*start <= *stop)
        return start->secsTo(*stop);
    else
        return 86400 + start->secsTo(*stop);
}

void ProgrammeView::drawTimeSpan
    (QPainter *painter, const QRectF &rect,
     const QTime &_start, const QTime &_stop, int posn)
{
    // Adjust the time span to start and end at 6am.
    if (m_mode == BookmarksSingleChannel ||
            m_mode == BookmarksMultiChannel)
        posn = 1;
    QTime start(_start);
    QTime stop(_stop);
    int length = adjustTimeSpan(&start, &stop, posn);

    // Draw the background rectangle.
    QTime colorTime(start);
    int totalLength = length;
    QRectF r(rect);
    while (length > 0) {
        // Find the cross-over point to the next 6 hour color slice.
        int hour = colorTime.hour();
        QTime nextTime;
        int sliceLength;
        if (hour < 6) {
            nextTime = QTime(6, 0);
            sliceLength = colorTime.secsTo(nextTime);
        } else if (hour < 12) {
            nextTime = QTime(12, 0);
            sliceLength = colorTime.secsTo(nextTime);
        } else if (hour < 18) {
            nextTime = QTime(18, 0);
            sliceLength = colorTime.secsTo(nextTime);
        } else {
            nextTime = QTime(0, 0);
            sliceLength = 86400 + colorTime.secsTo(nextTime);
        }
        if (sliceLength >= length) {
            nextTime = stop;
            sliceLength = length;
        }

        // Determine the color of this slice and draw it.
        QBrush brush;
        if (hour < 6)
            brush = QBrush(Qt::gray);
        else if (hour < 12)
            brush = QBrush(QColor(0, 192, 64));
        else if (hour < 18)
            brush = QBrush(QColor(192, 192, 64));
        else
            brush = QBrush(Qt::yellow);
        if (sliceLength < length) {
            qreal height = rect.height() * sliceLength / totalLength;
            QRectF paintRect(r.x(), r.y(), r.width(), height);
            painter->fillRect(paintRect, brush);
            r.setTop(r.top() + height);
        } else {
            // Omit the last pixel to create the appearance of a
            // dividing line between programmes.
            QRectF paintRect = r.adjusted(0, 0, 0, -1);
            painter->fillRect(paintRect, brush);
        }
        length -= sliceLength;
        colorTime = nextTime;
    }

    // Draw the time in either 12-hour or 24-hour format.
    int hour = start.hour();
    if (!m_is24HourClock) {
        if (hour == 0)
            hour = 12;
        else if (hour > 12)
            hour -= 12;
    }
    QString str;
    str.sprintf("%d:%02d ", hour, start.minute());
    painter->setFont(font());
    painter->setPen(QPen(palette().foreground(), 0));
    painter->drawText
        (rect, Qt::AlignRight | Qt::AlignTop | Qt::TextSingleLine, str);
}

// Determine the y offset from the top of a column of a
// specific time value between 6am and 6am next day.
qreal ProgrammeView::yOffset(const ColumnInfo *column, int timeValue)
{
    if (column->programmes.isEmpty())
        return 0.0f;

    // Find the programme in this column that overlaps with
    // the starting time value.
    int start = 0;
    int startValue = 0;
    for (; start < column->programmes.size(); ++start) {
        TvProgramme *prog = column->programmes.at(start).prog;
        int pvalue = minuteIndexForProgramme(prog, start);
        if (pvalue == timeValue) {
            startValue = pvalue;
            break;
        } else if (pvalue > timeValue) {
            if (start > 0)
                --start;
            else
                startValue = pvalue;
            break;
        }
        startValue = pvalue;
    }
    if (start >= column->programmes.size())
        --start;    // Must fall within the last programme.

    // Determine the progress through this programme's visual
    // layout to the designated time; where 0 shows the whole
    // programme, 0.5 the bottom half, 1 the next programme.
    TvProgramme *prog = column->programmes.at(start).prog;
    int length = prog->start().secsTo(prog->stop()) / 60;
    qreal progress;
    if (length <= 0)
        progress = 0.0f;
    else
        progress = qreal(timeValue - startValue) / qreal(length);

    // Draw the column at the calculated offset.
    QRectF rect = column->programmes.at(start).rect;
    return rect.top() * (1 - progress) + rect.bottom() * progress;
}

ProgrammeView::Selection ProgrammeView::programmeAtPosition(const QPoint &pos) const
{
    int timeValue = verticalScrollBar()->value() + MINUTES_OF_6AM;
    if ((timeValue % MINUTES_SINGLE_STEP) < ((MINUTES_SINGLE_STEP + 1) / 2))
        timeValue -= timeValue % MINUTES_SINGLE_STEP;
    else
        timeValue += MINUTES_SINGLE_STEP - (timeValue % MINUTES_SINGLE_STEP);
    int offsetHour = timeValue / 60 - 6;
    qreal offsetProgress = (timeValue % 60) / 60.0;
    qreal x = pos.x() + horizontalScrollBar()->value();
    qreal columnx = 0;
    bool filter = (m_displayMode != FullDay);
    for (int index = 0; index < m_activeColumns.size(); ++index) {
        const ColumnInfo *column = m_activeColumns.at(index);
        if (x < columnx)
            break;
        if (x >= (columnx + column->columnRect.width())) {
            columnx += column->columnRect.width() + m_columnSpacing;
            continue;
        }
        qreal y = pos.y();
        if (filter) {
            y += verticalScrollBar()->value();
        } else if (offsetHour >= 24) {
            y += column->hourOffsets.at(offsetHour);
        } else {
            y += (1 - offsetProgress) * column->hourOffsets.at(offsetHour) +
                offsetProgress * column->hourOffsets.at(offsetHour + 1);
        }
        x -= columnx;
        qreal recty = 0;
        int dayNumber = -1;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            const ProgInfo &info = column->programmes.at(index2);
            if (!info.enabled)
                continue;
            if (m_displayMode == Bookmarks && info.dayNumber != dayNumber) {
                dayNumber = info.dayNumber;
                recty = m_dayOffsets[dayNumber];
            }
            if (y < recty)
                break;
            recty += info.rect.height();
            if (y >= recty)
                continue;
            if (x >= info.rect.left()) {
                Selection result;
                result.column = index;
                result.row = index2;
                result.prog = info.prog;
                return result;
            }
        }
        break;
    }
    return Selection();
}

void ProgrammeView::applyFilter()
{
    bool filter = hasFilter();
    qreal maxHeight = 0.0f;
    m_activeColumns.clear();
    qreal width = 0;
    for (int index = 0; index < m_columns.size(); ++index) {
        ColumnInfo *column = m_columns.at(index);
        qreal height = 0.0f;
        for (int index2 = 0; index2 < column->programmes.size(); ++index2) {
            ProgInfo &info = column->programmes[index2];
            if (filter) {
                if (m_advancedFilter) {
                    info.enabled = m_advancedFilter->match(info.prog);
                } else {
                    info.enabled = info.prog->containsSearchString
                        (m_filter, TvProgramme::SearchAll);
                }
            } else {
                info.enabled = true;
            }
            if (info.enabled) {
                height += info.rect.height();
            } else if (m_selection.prog == info.prog) {
                // Selected item is no longer visible.
                m_selection.prog = 0;
                m_selection.column = -1;
                m_selection.row = -1;
            }
        }
        if (height > maxHeight)
            maxHeight = height;
        if (height > 0) {
            m_activeColumns.append(column);
            width += column->columnRect.width() + m_columnSpacing;
        }
    }
    if (m_mode == BookmarksSingleChannel ||
            m_mode == BookmarksMultiChannel) {
        maxHeight = calculateDayHeights();
    }
    m_totalRect.setWidth(width);
    m_totalRect.setHeight(maxHeight);
    updateScrollBars();
    viewport()->update();
    m_headerView->update();
}

ProgrammeHeaderView::ProgrammeHeaderView(ProgrammeView *view)
    : QWidget(view)
    , m_view(view)
{
}

ProgrammeHeaderView::~ProgrammeHeaderView()
{
}

int ProgrammeHeaderView::bestHeight()
{
    QFontMetrics metrics(font(), this);
    return metrics.height() + 5;
}

void ProgrammeHeaderView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_view->m_columns.isEmpty() || m_view->m_activeColumns.isEmpty()) {
        drawSection(&painter, rect(), 0, QString(), QIcon(),
                    QStyleOptionHeader::OnlyOneSection);
    } else if (m_view->m_activeColumns.size() == 1 &&
               m_view->m_displayMode != ProgrammeView::Bookmarks) {
        TvProgramme *prog = m_view->m_activeColumns.at(0)->programmes.at(0).prog;
        QString text = prog->channel()->name();
        text += QLatin1String(" - ");
        text += prog->start().date().toString(Qt::DefaultLocaleLongDate);
        drawSection(&painter, rect(), 0, text, prog->channel()->icon(),
                    QStyleOptionHeader::OnlyOneSection);
    } else {
        painter.translate(-m_view->horizontalScrollBar()->value(), 0);
        qreal x = 0;
        for (int index = 0; index < m_view->m_activeColumns.size(); ++index) {
            const ProgrammeView::ColumnInfo *column = m_view->m_activeColumns.at(index);
            const ProgrammeView::ProgInfo &info = column->programmes[0];
            QRect rect(int(x), 0, (int)(column->columnRect.width()), height());
            QStyleOptionHeader::SectionPosition position;
            if (!index) {
                position = QStyleOptionHeader::Beginning;
            } else if (index == (m_view->m_columns.size() - 1)) {
                position = QStyleOptionHeader::End;
                int right = m_view->horizontalScrollBar()->value() + width();
                if (right > rect.right())
                    rect.setRight(right);
            } else {
                position = QStyleOptionHeader::Middle;
            }
            drawSection(&painter, rect, index,
                        info.prog->channel()->name(),
                        info.prog->channel()->icon(),
                        position);
            x += column->columnRect.width() + m_view->m_columnSpacing;
        }
    }
}

void ProgrammeHeaderView::drawSection
    (QPainter *painter, const QRect &rect, int index,
     const QString &text, const QIcon &icon,
     QStyleOptionHeader::SectionPosition position)
{
    QStyleOptionHeader opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_Raised;
    opt.rect = rect;
    opt.section = index;
    opt.textAlignment = Qt::AlignCenter;
    opt.iconAlignment = Qt::AlignVCenter;
    opt.text = text;
    opt.icon = icon;
    opt.orientation = Qt::Horizontal;
    opt.position = position;
    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
    opt.sortIndicator = QStyleOptionHeader::None;
    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
}
