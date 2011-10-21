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

    QListWidget *lists[] = {
        categories,
        abcCategories,
        defCategories,
        ghiCategories,
        jklCategories,
        mnoCategories,
        pqrCategories,
        stuCategories,
        vwCategories,
        xyzCategories,
        0
    };
    for (int index = 0; lists[index] != 0; ++index) {
        connect(lists[index], SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
                this, SLOT(selectionChanged(QListWidgetItem *)));
        connect(lists[index], SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                this, SLOT(doubleClicked(QListWidgetItem *)));
    }

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

CategorySelector::~CategorySelector()
{
}

void CategorySelector::setCategories(const QSet<QString> &list)
{
    categories->clear();
    QSet<QString>::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        QString name = *it;
        if (name.isEmpty())
            continue;
        categories->addItem(name);
        switch (name.at(0).unicode()) {
        case 'A': case 'B': case 'C': case 'a': case 'b': case 'c':
            abcCategories->addItem(name);
            break;
        case 'D': case 'E': case 'F': case 'd': case 'e': case 'f':
            defCategories->addItem(name);
            break;
        case 'G': case 'H': case 'I': case 'g': case 'h': case 'i':
            ghiCategories->addItem(name);
            break;
        case 'J': case 'K': case 'L': case 'j': case 'k': case 'l':
            jklCategories->addItem(name);
            break;
        case 'M': case 'N': case 'O': case 'm': case 'n': case 'o':
            mnoCategories->addItem(name);
            break;
        case 'P': case 'Q': case 'R': case 'p': case 'q': case 'r':
            pqrCategories->addItem(name);
            break;
        case 'S': case 'T': case 'U': case 's': case 't': case 'u':
            stuCategories->addItem(name);
            break;
        case 'V': case 'W': case 'v': case 'w':
            vwCategories->addItem(name);
            break;
        case 'X': case 'Y': case 'Z': case 'x': case 'y': case 'z':
            xyzCategories->addItem(name);
            break;
        default: break;
        }
    }
    categories->sortItems();
    abcCategories->sortItems();
    defCategories->sortItems();
    ghiCategories->sortItems();
    jklCategories->sortItems();
    mnoCategories->sortItems();
    pqrCategories->sortItems();
    stuCategories->sortItems();
    vwCategories->sortItems();
    xyzCategories->sortItems();
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
