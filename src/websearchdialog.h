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

#ifndef _WEBSEARCHDIALOG_H
#define _WEBSEARCHDIALOG_H

#include <QtWidgets/qdialog.h>
#include <QtCore/qurl.h>
#include "ui_websearchdialog.h"

class WebSearchDialog : public QDialog, private Ui::WebSearchDialog
{
    Q_OBJECT
public:
    explicit WebSearchDialog(QWidget *parent = 0);
    ~WebSearchDialog();

    QString searchText() const { return searchFor->currentText(); }
    void setSearchText(const QString &text) { searchFor->setEditText(text); }

    void addSearchItem(const QString &text);
    void addSearchItems(const QStringList &list);

    QUrl url() const;

    void saveSettings();

private Q_SLOTS:
    void titleChanged(const QString &text);
    void help();
};

#endif
