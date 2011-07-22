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

#include "helpbrowser.h"
#include "mainwindow.h"
#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>

HelpBrowser::HelpBrowser(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);

    connect(actionBack, SIGNAL(triggered()),
            textBrowser, SLOT(backward()));
    connect(actionForward, SIGNAL(triggered()),
            textBrowser, SLOT(forward()));
    connect(actionHome, SIGNAL(triggered()),
            textBrowser, SLOT(home()));
    connect(actionClose, SIGNAL(triggered()),
            this, SLOT(close()));

    connect(textBrowser, SIGNAL(backwardAvailable(bool)),
            actionBack, SLOT(setEnabled(bool)));
    connect(textBrowser, SIGNAL(forwardAvailable(bool)),
            actionForward, SLOT(setEnabled(bool)));
    connect(textBrowser, SIGNAL(sourceChanged(QUrl)),
            this, SLOT(sourceChanged(QUrl)));

    actionBack->setEnabled(textBrowser->isBackwardAvailable());
    actionForward->setEnabled(textBrowser->isForwardAvailable());

    QUrl homeUrl(QLatin1String("qrc:/help/en/index.html"));
    textBrowser->setSource(homeUrl);
}

HelpBrowser::~HelpBrowser()
{
}

void HelpBrowser::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}

void HelpBrowser::sourceChanged(const QUrl &)
{
    setWindowTitle(textBrowser->documentTitle());
}

void HelpBrowser::setContextHelp(const QString &name)
{
    QUrl url(QLatin1String("qrc:/help/en/") + name);
    textBrowser->setSource(url);
}

void HelpBrowser::showContextHelp(const QString &name, QWidget *widget)
{
    MainWindow *mw = 0;
    while (widget != 0) {
        mw = qobject_cast<MainWindow *>(widget);
        if (mw)
            break;
        widget = widget->parentWidget();
    }
    if (mw)
        mw->showContextHelp(name);
}
