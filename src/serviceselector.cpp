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

#include "serviceselector.h"
#include "serviceeditor.h"
#include "helpbrowser.h"
#include <QtCore/qsettings.h>

#define ROLE_SERVICE_ID         Qt::UserRole
#define ROLE_SERVICE_URL        (Qt::UserRole + 1)
#define ROLE_SERVICE_REFRESH    (Qt::UserRole + 2)

ServiceSelector::ServiceSelector(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(serviceList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectionChanged()));

    connect(editButton, SIGNAL(clicked()), this, SLOT(editService()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteService()));
    connect(newButton, SIGNAL(clicked()), this, SLOT(newService()));

    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));

    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    serviceList->setSortingEnabled(true);

    connect(serviceList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(accept()));

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("Service"));
    QString current = settings.value(QLatin1String("id")).toString();
    int size = settings.beginReadArray(QLatin1String("service"));
    if (size) {
        for (int index = 0; index < size; ++index) {
            settings.setArrayIndex(index);
            QListWidgetItem *item =
                new QListWidgetItem(settings.value(QLatin1String("name")).toString());
            item->setData(ROLE_SERVICE_ID, settings.value(QLatin1String("id")).toString());
            item->setData(ROLE_SERVICE_URL, settings.value(QLatin1String("url")).toString());
            item->setData(ROLE_SERVICE_REFRESH, settings.value(QLatin1String("refresh")).toInt());
            serviceList->addItem(item);
            if (item->data(ROLE_SERVICE_ID).toString() == current)
                serviceList->setCurrentItem(item);
        }
        if (!serviceList->currentItem())
            serviceList->setCurrentRow(0);
    } else {
        // No services registered, so pre-populate the list.
        QListWidgetItem *item =
            new QListWidgetItem(QLatin1String("OzTivo - Australian Community-Driven TV Guide"));
        item->setData(ROLE_SERVICE_ID, QLatin1String("OzTivo"));
        item->setData(ROLE_SERVICE_URL, QLatin1String("http://xml.oztivo.net/xmltv/datalist.xml.gz"));
        item->setData(ROLE_SERVICE_REFRESH, 24);
        serviceList->addItem(item);
        serviceList->setCurrentItem(item);
    }
    settings.endArray();
    settings.endGroup();
}

ServiceSelector::~ServiceSelector()
{
}

void ServiceSelector::accept()
{
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("Service"));
    QListWidgetItem *item = serviceList->selectedItems().at(0);
    settings.setValue(QLatin1String("id"),
                      item->data(ROLE_SERVICE_ID).toString());
    settings.setValue(QLatin1String("name"), item->text());
    settings.setValue(QLatin1String("url"),
                      item->data(ROLE_SERVICE_URL).toString());
    settings.setValue(QLatin1String("refresh"),
                      item->data(ROLE_SERVICE_REFRESH).toInt());
    settings.beginWriteArray(QLatin1String("service"));
    for (int index = 0; index < serviceList->count(); ++index) {
        item = serviceList->item(index);
        settings.setArrayIndex(index);
        settings.setValue(QLatin1String("name"), item->text());
        settings.setValue(QLatin1String("id"),
                          item->data(ROLE_SERVICE_ID).toString());
        settings.setValue(QLatin1String("url"),
                          item->data(ROLE_SERVICE_URL).toString());
        settings.setValue(QLatin1String("refresh"),
                          item->data(ROLE_SERVICE_REFRESH).toInt());
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
    QDialog::accept();
}

void ServiceSelector::editService()
{
    QList<QListWidgetItem *> items = serviceList->selectedItems();
    if (items.size() < 1)
        return;
    QListWidgetItem *item = items.at(0);
    ServiceEditor editor(this);
    editor.setName(item->text());
    editor.setUrl(item->data(ROLE_SERVICE_URL).toString());
    editor.setRefresh(item->data(ROLE_SERVICE_REFRESH).toInt());
    if (editor.exec() == QDialog::Accepted) {
        item->setText(editor.name());
        item->setData(ROLE_SERVICE_URL, editor.url());
        item->setData(ROLE_SERVICE_REFRESH, editor.refresh());
    }
}

void ServiceSelector::deleteService()
{
    int row = serviceList->currentRow();
    if (row < 0 || row >= serviceList->count())
        return;
    delete serviceList->takeItem(row);
}

void ServiceSelector::newService()
{
    ServiceEditor editor(this);
    editor.setWindowTitle(tr("New Service"));
    if (editor.exec() == QDialog::Accepted) {
        QString id = createServiceId(editor.name());
        QListWidgetItem *item = new QListWidgetItem(editor.name());
        item->setData(ROLE_SERVICE_ID, id);
        item->setData(ROLE_SERVICE_URL, editor.url());
        item->setData(ROLE_SERVICE_REFRESH, editor.refresh());
        serviceList->addItem(item);
    }
}

void ServiceSelector::selectionChanged()
{
    if (serviceList->selectedItems().size() >= 1) {
        editButton->setEnabled(true);
        deleteButton->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
        editButton->setEnabled(false);
        deleteButton->setEnabled(false);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

QString ServiceSelector::createServiceId(const QString &name) const
{
    QString id;
    for (int index = 0; index < name.length(); ++index) {
        uint ch = name.at(index).unicode();
        if ((ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
            id += QChar(ch);
        else if (ch == ' ' && index > 0)
            break;
        else
            id += QLatin1Char('_');
    }
    if (!hasServiceId(id))
        return id;
    int value = 1;
    QString idnum;
    for (;;) {
        idnum = id + QString::number(value);
        if (!hasServiceId(idnum))
            break;
        ++value;
    }
    return idnum;
}

bool ServiceSelector::hasServiceId(const QString &id) const
{
    for (int index = 0; index < serviceList->count(); ++index) {
        if (id == serviceList->item(index)->data(ROLE_SERVICE_ID).toString())
            return true;
    }
    return false;
}

void ServiceSelector::help()
{
    HelpBrowser::showContextHelp(QLatin1String("select_guide.html"), this);
}
