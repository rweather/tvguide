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

#ifndef _HELPBROWSER_H
#define _HELPBROWSER_H

#include <QtGui/qmainwindow.h>
#include "ui_helpbrowser.h"

class HelpBrowser : public QMainWindow, private Ui::HelpBrowser
{
    Q_OBJECT
public:
    explicit HelpBrowser(QWidget *parent = 0);
    ~HelpBrowser();

    void setContextHelp(const QString &name);

    static void showContextHelp(const QString &name, QWidget *widget);

protected:
    void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
    void sourceChanged(const QUrl &url);
};

#endif
