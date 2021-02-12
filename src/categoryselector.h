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

#ifndef _CATEGORYSELECTOR_H
#define _CATEGORYSELECTOR_H

#include <QtWidgets/qdialog.h>
#include <QtCore/qset.h>
#include "ui_categoryselector.h"

class CategorySelector : public QDialog, private Ui::CategorySelector
{
    Q_OBJECT
public:
    explicit CategorySelector(QWidget *parent = 0);
    ~CategorySelector();

    void setCategories(const QSet<QString> &list);

    QString selectedCategory() const { return m_selection; }

private Q_SLOTS:
    void selectionChanged(QListWidgetItem *item);
    void doubleClicked(QListWidgetItem *item);

private:
    QString m_selection;
};

#endif
