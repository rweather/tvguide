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

#ifndef _SERVICESELECTOR_H
#define _SERVICESELECTOR_H

#include <QtGui/qdialog.h>
#include "ui_serviceselector.h"

class ServiceSelector : public QDialog, private Ui::ServiceSelector
{
    Q_OBJECT
public:
    explicit ServiceSelector(QWidget *parent = 0);
    ~ServiceSelector();

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void editService();
    void deleteService();
    void newService();
    void selectionChanged();

private:
    QString createServiceId(const QString &name) const;
    bool hasServiceId(const QString &id) const;
};

#endif
