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

#include "tvprogrammedelegate.h"
#include <QtGui/qtextdocument.h>
#include <QtGui/qpainter.h>
#include <QtCore/qdebug.h>

void TvProgrammeDelegate::paint
    (QPainter *painter, const QStyleOptionViewItem &option,
     const QModelIndex &index) const
{
    QVariant data = index.data();
    if (data.userType() == QMetaType::VoidStar) {
        TvProgramme *programme = static_cast<TvProgramme *>
            (data.value<void *>());
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);
        QTextDocument *document = programme->shortDescriptionDocument();
        painter->save();
        painter->translate(opt.rect.x(), opt.rect.y());
        QRect clip(0, 0, opt.rect.width(), opt.rect.height());
        document->drawContents(painter, clip);
        painter->restore();
        return;
    }
    return QStyledItemDelegate::paint(painter, option, index);
}

QSize TvProgrammeDelegate::sizeHint
    (const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant data = index.data();
    if (data.userType() == QMetaType::VoidStar) {
        TvProgramme *programme = static_cast<TvProgramme *>
            (data.value<void *>());
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);
        QTextDocument *document = programme->shortDescriptionDocument();
        document->setDefaultFont(opt.font);
        document->setTextWidth(opt.rect.width());
        return document->size().toSize();
    }
    return QStyledItemDelegate::sizeHint(option, index);
}
