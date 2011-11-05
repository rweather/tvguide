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

#ifndef _PROGRAMMEVIEW_H
#define _PROGRAMMEVIEW_H

#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qstyleoption.h>
#include <QtCore/qvector.h>
#include "tvprogramme.h"

class ProgrammeHeaderView;

class ProgrammeView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit ProgrammeView(QWidget *parent = 0);
    ~ProgrammeView();

    void setProgrammes(const QList<TvProgramme *> &programmes);
    void setMultiChannelProgrammes(const QList<TvProgramme *> &programmes);

    TvProgramme *selectedProgramme() const { return m_selection.prog; }
    void updateSelection();

    void scrollToTime(const QTime &time);

Q_SIGNALS:
    void selectionChanged();

protected:
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
            {}
        TvProgramme *prog;
        QTextDocument *document;
        QRectF rect;
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
    QSize m_timeSize;
    QRectF m_totalRect;
    bool m_is24HourClock;
    int m_columnWidth;
    int m_columnSpacing;
    int m_options;
    ProgrammeHeaderView *m_headerView;
    QImage m_tickIcon;
    QImage m_returnedIcon;
    Selection m_selection;
    TvProgramme *m_prevSelection;

    void writeColumn(ColumnInfo *column, int columnIndex);
    void writeProgramme(ProgInfo *info, int index);

    void clearColumns();
    void layout();
    void layoutColumns();
    void layoutHeaderView();
    void levelHours();
    void updateScrollBars();

    static int adjustTimeSpan(QTime *start, QTime *stop, int posn);
    void drawTimeSpan(QPainter *painter, const QRectF &rect, const QTime &start, const QTime &stop, int posn);
    static qreal yOffset(const ColumnInfo *column, int timeValue);

    Selection programmeAtPosition(const QPoint &pos, bool includeTime) const;
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
};

#endif
