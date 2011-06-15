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

#ifndef _TVPROGRAMME_H
#define _TVPROGRAMME_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qxmlstream.h>

class TvChannel;

class TvProgramme
{
public:
    explicit TvProgramme(TvChannel *channel);
    ~TvProgramme();

    TvChannel *channel() const { return m_channel; }
    QDateTime start() const { return m_start; }
    QDateTime stop() const { return m_stop; }
    QString title() const { return m_title; }
    QString subTitle() const { return m_subTitle; }
    QString description() const { return m_description; }
    QString date() const { return m_date; }
    QStringList directors() const { return m_directors; }
    QStringList actors() const { return m_actors; }
    QStringList categories() const { return m_categories; }
    QString rating() const { return m_rating; }
    QString starRating() const { return m_starRating; }
    QString episodeNumber() const { return m_episodeNumber; }
    QString language() const { return m_language; }
    QString originalLanguage() const { return m_originalLanguage; }
    QString country() const { return m_country; }

    void load(QXmlStreamReader *reader);

    QString shortDescription() const;
    QString longDescription() const;

private:
    TvChannel *m_channel;
    QDateTime m_start;
    QDateTime m_stop;
    QString m_title;
    QString m_subTitle;
    QString m_description;
    QString m_date;
    QStringList m_directors;
    QStringList m_actors;
    QStringList m_categories;
    QString m_rating;
    QString m_starRating;
    QString m_episodeNumber;
    QString m_language;
    QString m_originalLanguage;
    QString m_country;
    mutable QString m_shortDescription;
    mutable QString m_longDescription;
    TvProgramme *m_next;

    friend class TvChannel;
};

#endif
