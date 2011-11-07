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

#ifndef _TVDATETIME_H
#define _TVDATETIME_H

#include <QtCore/qdatetime.h>

// Local time only version of QDateTime that is more efficient
// when performing comparisons.  Qt's comparison operators always
// do a local->UTC conversion, which is expensive when we know
// that all programme data is in local time after being loaded.
class TvDateTime : public QDateTime
{
public:
    TvDateTime() {}
    TvDateTime(const QDate &date, const QTime &time)
        : QDateTime(date, time, Qt::LocalTime) {}
    TvDateTime(const QDateTime &other)
        : QDateTime(other)
    {
        if (timeSpec() != Qt::LocalTime)
            QDateTime::operator=(toLocalTime());
    }
    ~TvDateTime() {}

    TvDateTime &operator=(const TvDateTime &other)
    {
        QDateTime::operator=(other);
        return *this;
    }
    TvDateTime &operator=(const QDateTime &other)
    {
        if (other.timeSpec() == Qt::LocalTime)
            QDateTime::operator=(other);
        else
            QDateTime::operator=(other.toLocalTime());
        return *this;
    }

    bool operator==(const TvDateTime &dt) const
    {
        return date() == dt.date() && time() == dt.time();
    }
    bool operator!=(const TvDateTime &dt) const
    {
        return date() != dt.date() || time() != dt.time();
    }
    bool operator<(const TvDateTime &dt) const
    {
        if (date() != dt.date())
            return date() < dt.date();
        else
            return time() < dt.time();
    }
    bool operator<=(const TvDateTime &dt) const
    {
        if (date() != dt.date())
            return date() < dt.date();
        else
            return time() <= dt.time();
    }
    bool operator>(const TvDateTime &dt) const
    {
        if (date() != dt.date())
            return date() > dt.date();
        else
            return time() > dt.time();
    }
    bool operator>=(const TvDateTime &dt) const
    {
        if (date() != dt.date())
            return date() > dt.date();
        else
            return time() >= dt.time();
    }
};

#endif
