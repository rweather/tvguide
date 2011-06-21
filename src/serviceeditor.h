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

#ifndef _SERVICEEDITOR_H
#define _SERVICEEDITOR_H

#include <QtGui/qdialog.h>
#include "ui_serviceeditor.h"

class ServiceEditor : public QDialog, private Ui::ServiceEditor
{
    Q_OBJECT
public:
    explicit ServiceEditor(QWidget *parent = 0);
    ~ServiceEditor();

    QString name() const { return serviceName->text(); }
    void setName(const QString &name) { serviceName->setText(name); }

    QString url() const { return serviceUrl->text(); }
    void setUrl(const QString &url) { serviceUrl->setText(url); }

    int refresh() const;
    void setRefresh(int hours);

private Q_SLOTS:
    void textChanged();
};

#endif
