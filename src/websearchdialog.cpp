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

#include "websearchdialog.h"
#include "helpbrowser.h"
#include <QtGui/qpushbutton.h>

WebSearchDialog::WebSearchDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(titleEdit, SIGNAL(textChanged(QString)), this, SLOT(titleChanged(QString)));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
}

WebSearchDialog::~WebSearchDialog()
{
}

QUrl WebSearchDialog::url() const
{
    QList<QRadioButton *> buttons = groupBox->findChildren<QRadioButton *>();
    for (int index = 0; index < buttons.size(); ++index) {
        if (buttons.at(index)->isChecked()) {
            QString url = buttons.at(index)->property("url").toString();
            url += QString::fromLatin1
                (QUrl::toPercentEncoding(title(), "/?:"));
            return QUrl(url);
        }
    }
    return QUrl();
}

void WebSearchDialog::titleChanged(const QString &text)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void WebSearchDialog::help()
{
    HelpBrowser::showContextHelp(QLatin1String("searching.html"), this);
}
