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

#include "dayselectiondialog.h"
#include <QtGui/qpushbutton.h>

DaySelectionDialog::DaySelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(sunday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(monday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(tuesday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(wednesday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(thursday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(friday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));
    connect(saturday, SIGNAL(stateChanged(int)), this, SLOT(dayChanged()));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

DaySelectionDialog::~DaySelectionDialog()
{
}

int DaySelectionDialog::mask() const
{
    int mask = 0;
    if (monday->isChecked())
        mask |= 0x02;
    if (tuesday->isChecked())
        mask |= 0x04;
    if (wednesday->isChecked())
        mask |= 0x08;
    if (thursday->isChecked())
        mask |= 0x10;
    if (friday->isChecked())
        mask |= 0x20;
    if (saturday->isChecked())
        mask |= 0x40;
    if (sunday->isChecked())
        mask |= 0x80;
    return mask;
}

void DaySelectionDialog::setMask(int mask)
{
    monday->setChecked((mask & 0x02) != 0);
    tuesday->setChecked((mask & 0x04) != 0);
    wednesday->setChecked((mask & 0x08) != 0);
    thursday->setChecked((mask & 0x10) != 0);
    friday->setChecked((mask & 0x20) != 0);
    saturday->setChecked((mask & 0x40) != 0);
    sunday->setChecked((mask & 0x80) != 0);
}

void DaySelectionDialog::dayChanged()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(mask() != 0);
}
