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

#include "bookmarklisteditor.h"
#include "bookmarkitemeditor.h"
#include "tvchannellist.h"
#include "helpbrowser.h"
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

BookmarkListEditor::BookmarkListEditor(TvChannelList *channelList, QWidget *parent)
    : QDialog(parent)
    , m_channelList(channelList)
{
    setupUi(this);

    setWindowModality(Qt::WindowModal);

    m_model = new TvBookmarkModel(channelList, this);

    m_proxy = new BookmarkListSortModel(m_model, this);
    m_proxy->setDynamicSortFilter(true);

    bookmarkView->setModel(m_proxy);
    bookmarkView->verticalHeader()->hide();
    bookmarkView->horizontalHeader()->setStretchLastSection(true);
    bookmarkView->setSelectionBehavior(QTableView::SelectRows);

    connect(bookmarkView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex)));
    connect(bookmarkView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(editBookmark()));

    connect(editButton, SIGNAL(clicked()), this, SLOT(editBookmark()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteBookmark()));
    connect(newButton, SIGNAL(clicked()), this, SLOT(newBookmark()));

    editButton->setEnabled(false);
    deleteButton->setEnabled(false);

    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("BookmarkSort"));
    int column = settings.value(QLatin1String("column"), 1).toInt();
    Qt::SortOrder order = Qt::SortOrder(settings.value(QLatin1String("order"), int(Qt::AscendingOrder)).toInt());
    if (column < 0 || column >= 6)
        column = 1;
    if (order != Qt::AscendingOrder && order != Qt::DescendingOrder)
        order = Qt::AscendingOrder;
    settings.endGroup();

    bookmarkView->setSortingEnabled(true);
    bookmarkView->horizontalHeader()->setSortIndicator(column, order);

    QStyle *style = bookmarkView->style();
    if (!style)
        style = QApplication::style();
    QStyleOptionButton opt;
    QSize size = style->sizeFromContents(QStyle::CT_CheckBox, &opt, QSize(3, 1), bookmarkView);
    bookmarkView->setColumnWidth(0, size.width());

    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
}

BookmarkListEditor::~BookmarkListEditor()
{
    QSettings settings(QLatin1String("Southern Storm"),
                       QLatin1String("qtvguide"));
    settings.beginGroup(QLatin1String("BookmarkSort"));
    settings.setValue(QLatin1String("column"),
                      bookmarkView->horizontalHeader()->sortIndicatorSection());
    settings.setValue(QLatin1String("order"),
                      int(bookmarkView->horizontalHeader()->sortIndicatorOrder()));
    settings.endGroup();
    settings.sync();
}

void BookmarkListEditor::accept()
{
    m_channelList->bookmarkList()->replaceBookmarks(m_model->detachBookmarks());
    QDialog::accept();
}

void BookmarkListEditor::editBookmark()
{
    QModelIndex index = bookmarkView->selectionModel()->currentIndex();
    index = m_proxy->mapToSource(index);
    if (!index.isValid())
        return;
    TvBookmark *bookmark = m_model->bookmarkAt(index.row());

    BookmarkItemEditor bookmarkDlg(m_channelList, this);
    bookmarkDlg.copyFromBookmark(bookmark);

    if (bookmarkDlg.exec() == QDialog::Accepted) {
        bookmarkDlg.copyToBookmark(bookmark);
        m_model->updateBookmark(index.row());
        bookmarkView->sortByColumn
            (bookmarkView->horizontalHeader()->sortIndicatorSection(),
             bookmarkView->horizontalHeader()->sortIndicatorOrder());
    }
}

void BookmarkListEditor::deleteBookmark()
{
    QModelIndex index = bookmarkView->selectionModel()->currentIndex();
    index = m_proxy->mapToSource(index);
    if (!index.isValid())
        return;
    m_model->removeBookmark(index.row());
}

void BookmarkListEditor::newBookmark()
{
    BookmarkItemEditor bookmarkDlg(m_channelList, this);
    bookmarkDlg.setWindowTitle(tr("New Bookmark"));
    if (bookmarkDlg.exec() == QDialog::Accepted) {
        TvBookmark *bookmark = new TvBookmark();
        bookmarkDlg.copyToBookmark(bookmark);
        m_model->addBookmark(bookmark);
        bookmarkView->sortByColumn
            (bookmarkView->horizontalHeader()->sortIndicatorSection(),
             bookmarkView->horizontalHeader()->sortIndicatorOrder());
    }
}

void BookmarkListEditor::currentChanged(const QModelIndex &index)
{
    if (index.isValid()) {
        editButton->setEnabled(true);
        deleteButton->setEnabled(true);
    } else {
        editButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }
}

void BookmarkListEditor::help()
{
    HelpBrowser::showContextHelp(QLatin1String("bookmarks.html"), this);
}
