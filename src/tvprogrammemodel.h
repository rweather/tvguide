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

#ifndef _TVPROGRAMMEMODEL_H
#define _TVPROGRAMMEMODEL_H

#include "tvprogramme.h"
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qicon.h>

class TvProgrammeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TvProgrammeModel(QObject *parent = 0);
    ~TvProgrammeModel();

    static const int ColumnCount        = 4;
    static const int ColumnDay          = 0;
    static const int ColumnTime         = 1;
    static const int ColumnChannel      = 2;
    static const int ColumnTitle        = 3;

    void clear();
    void setProgrammes(const QList<TvProgramme *> &programmes, TvChannel *channel, const QDate &date);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void updateTick(int row);
    void updateIcons();

    QString filter() const { return m_filter; }
    void setFilter(const QString &str);

    int filterOptions() const { return m_filterOptions; }
    void setFilterOptions(int options);

private:
    QList<TvProgramme *> m_unfilteredProgrammes;
    QList<TvProgramme *> m_programmes;
    TvChannel *m_channel;
    QDate m_date;
    QPixmap m_bookmarkIcon;
    QPixmap m_tickIcon;
    QPixmap m_returnedIcon;
    QString m_filter;
    int m_filterOptions;

    void updateFilter();
};

#endif
