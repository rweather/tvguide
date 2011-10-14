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

#include "categoryselector.h"
#include <QtGui/qpushbutton.h>

CategorySelector::CategorySelector(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(categories, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
            this, SLOT(selectionChanged(QListWidgetItem *)));
    connect(categories, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(doubleClicked(QListWidgetItem *)));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

CategorySelector::~CategorySelector()
{
}

void CategorySelector::setCategories(const QSet<QString> &list)
{
    categories->clear();
    QSet<QString>::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it)
        categories->addItem(*it);
    categories->sortItems();
}

void CategorySelector::selectionChanged(QListWidgetItem *item)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(item != 0);
    if (item != 0)
        m_selection = item->text();
}

void CategorySelector::doubleClicked(QListWidgetItem *item)
{
    m_selection = item->text();
    accept();
}
