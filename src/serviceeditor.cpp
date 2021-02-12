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

#include "serviceeditor.h"
#include "helpbrowser.h"
#include <QtWidgets/qpushbutton.h>

ServiceEditor::ServiceEditor(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(serviceName, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(serviceUrl, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
}

ServiceEditor::~ServiceEditor()
{
}

int ServiceEditor::refresh() const
{
    switch (refreshTime->currentIndex()) {
    case 0:     return 3;
    case 1:     return 6;
    case 2:     return 12;
    case 3:     return 24;
    case 4:     return 48;
    }
    return 48;
}

void ServiceEditor::setRefresh(int hours)
{
    if (hours <= 3)
        refreshTime->setCurrentIndex(0);
    else if (hours <= 6)
        refreshTime->setCurrentIndex(1);
    else if (hours <= 12)
        refreshTime->setCurrentIndex(2);
    else if (hours <= 24)
        refreshTime->setCurrentIndex(3);
    else
        refreshTime->setCurrentIndex(4);
}

void ServiceEditor::textChanged()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled
        (!serviceName->text().isEmpty() &&
         !serviceUrl->text().isEmpty());
}

void ServiceEditor::help()
{
    HelpBrowser::showContextHelp(QLatin1String("select_guide.html"), this);
}
