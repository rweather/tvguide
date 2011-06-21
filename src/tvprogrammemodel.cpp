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

#include "tvprogrammemodel.h"
#include "tvchannel.h"
#include <QtCore/qdebug.h>
#include <QtGui/qfont.h>
#include <QtGui/qwidget.h>

TvProgrammeModel::TvProgrammeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_channel(0)
    , m_bookmarkIcon(QLatin1String(":/images/bookmark.png"))
{
}

TvProgrammeModel::~TvProgrammeModel()
{
}

void TvProgrammeModel::clear()
{
    if (!m_programmes.isEmpty()) {
        m_programmes.clear();
        m_channel = 0;
        m_date = QDate();
        reset();
    }
}

void TvProgrammeModel::setProgrammes(const QList<TvProgramme *> &programmes, TvChannel *channel, const QDate &date)
{
    m_programmes = programmes;
    m_channel = channel;
    m_date = date;
    reset();
}

#define MODEL_COLUMNS       3
#define MODEL_COL_DAY       0
#define MODEL_COL_TIME      1
#define MODEL_COL_TITLE     2

QModelIndex TvProgrammeModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || row >= m_programmes.size())
        return QModelIndex();
    if (column < 0 || column >= MODEL_COLUMNS)
        return QModelIndex();
    return createIndex(row, column, m_programmes.at(row));
}

QModelIndex TvProgrammeModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvProgrammeModel::columnCount(const QModelIndex &) const
{
    return MODEL_COLUMNS;
}

int TvProgrammeModel::rowCount(const QModelIndex &) const
{
    return m_programmes.size();
}

QVariant TvProgrammeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    TvProgramme *prog = static_cast<TvProgramme *>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        if (index.column() == MODEL_COL_TIME) {
            QTime time = prog->start().time();
            return time.toString(Qt::LocaleDate);
        } else if (index.column() == MODEL_COL_TITLE) {
            return QVariant::fromValue<void *>
                (static_cast<void *>(prog));
        } else if (index.column() == MODEL_COL_DAY) {
            return QDate::longDayName(prog->start().date().dayOfWeek());
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == MODEL_COL_TIME)
            return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::ToolTipRole) {
        if (index.column() == MODEL_COL_TITLE)
            return prog->longDescription();
    } else if (role == Qt::DecorationRole) {
        if (index.column() == MODEL_COL_TIME) {
            if (prog->color().isValid())
                return m_bookmarkIcon;
        }
    } else if (role == Qt::BackgroundRole) {
        // Paint the times for different parts of the day
        // in different colors to make it easier to find
        // things like Morning, Afternoon, Night, etc.
        if (index.column() == MODEL_COL_DAY ||
                index.column() == MODEL_COL_TIME) {
            int hour = prog->start().time().hour();
            if (hour < 6)
                return QBrush(Qt::gray);
            else if (hour < 12)
                return QBrush(QColor(0, 192, 64));
            else if (hour < 18)
                return QBrush(QColor(192, 192, 64));
            else
                return QBrush(Qt::yellow);
        }
    }
    return QVariant();
}

QVariant TvProgrammeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == MODEL_COL_DAY)
            return tr("Day");
        if (section == MODEL_COL_TIME)
            return tr("Time");
        if (section == MODEL_COL_TITLE) {
            if (m_date.isValid()) {
                QString title;
                if (m_channel) {
                    title += m_channel->name();
                    title += QLatin1String(" - ");
                }
                title += m_date.toString(Qt::DefaultLocaleLongDate);
                return title;
            } else {
                return tr("Description");
            }
        }
    }
    return QVariant();
}
