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
    , m_tickIcon(QLatin1String(":/images/tick.png"))
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

QModelIndex TvProgrammeModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || row >= m_programmes.size())
        return QModelIndex();
    if (column < 0 || column >= ColumnCount)
        return QModelIndex();
    return createIndex(row, column, m_programmes.at(row));
}

QModelIndex TvProgrammeModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int TvProgrammeModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
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
    int prevrow = index.row() - 1;
    if (role == Qt::DisplayRole) {
        if (index.column() == ColumnTime) {
            QTime time = prog->start().time();
            if (prevrow < 0 ||
                    m_programmes.at(prevrow)->start().time() != time)
                return time.toString(Qt::LocaleDate);
            if (prog->start().date().dayOfWeek() !=
                    m_programmes.at(prevrow)->start().date().dayOfWeek())
                return time.toString(Qt::LocaleDate);
        } else if (index.column() == ColumnTitle) {
            return QVariant::fromValue<void *>
                (static_cast<void *>(prog));
        } else if (index.column() == ColumnDay) {
            QDate date = prog->start().date();
            if (prevrow < 0 ||
                    m_programmes.at(prevrow)->start().date() != date) {
                return prog->start().date().toString
                    (QLatin1String("dddd\nMMMM d"));
            }
        } else if (index.column() == ColumnChannel) {
            return prog->channel()->name();
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColumnTime)
            return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::ToolTipRole) {
        if (index.column() == ColumnTitle)
            return prog->longDescription();
    } else if (role == Qt::DecorationRole) {
        if (index.column() == ColumnTime) {
            if (prog->isTicked())
                return m_tickIcon;
            if (prog->color().isValid())
                return m_bookmarkIcon;
        }
    } else if (role == Qt::BackgroundRole) {
        // Paint the times for different parts of the day
        // in different colors to make it easier to find
        // things like Morning, Afternoon, Night, etc.
        if (index.column() == ColumnDay ||
                index.column() == ColumnTime) {
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
        if (section == ColumnDay)
            return tr("Day");
        if (section == ColumnTime)
            return tr("Time");
        if (section == ColumnTitle) {
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
        if (section == ColumnChannel)
            return tr("Channel");
    }
    return QVariant();
}

void TvProgrammeModel::updateTick(int row)
{
    QModelIndex idx = index(row, ColumnTime, QModelIndex());
    emit dataChanged(idx, idx);
}
