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

#ifndef _TVPROGRAMMEDELEGATE_H
#define _TVPROGRAMMEDELEGATE_H

#include "tvprogramme.h"
#include <QtGui/qstyleditemdelegate.h>

class TvProgrammeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TvProgrammeDelegate(QWidget *widget, QObject *parent = 0)
        : QStyledItemDelegate(parent), m_widget(widget) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

private:
    QWidget *m_widget;
};

#endif
