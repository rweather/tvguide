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

#ifndef _TVPROGRAMMEFILTER_H
#define _TVPROGRAMMEFILTER_H

#include "tvprogramme.h"

class TvProgrammeFilter
{
public:
    enum CombineMode
    {
        CombineAnd,
        CombineOr
    };

    TvProgrammeFilter() : m_combineMode(CombineAnd), m_moviesOnly(false) {}
    ~TvProgrammeFilter() {}

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString episodeName() const { return m_episodeName; }
    void setEpisodeName(const QString &episodeName) { m_episodeName = episodeName; }

    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    QString credit() const { return m_credit; }
    void setCredit(const QString &credit) { m_credit = credit; }

    QString category() const { return m_category; }
    void setCategory(const QString &category) { m_category = category; }

    CombineMode combineMode() const { return m_combineMode; }
    void setCombineMode(CombineMode combineMode) { m_combineMode = combineMode; }

    bool moviesOnly() const { return m_moviesOnly; }
    void setMoviesOnly(bool moviesOnly) { m_moviesOnly = moviesOnly; }

    bool match(const TvProgramme *prog) const;

    bool isDefault() const;

private:
    QString m_title;
    QString m_episodeName;
    QString m_description;
    QString m_credit;
    QString m_category;
    CombineMode m_combineMode;
    bool m_moviesOnly;
};

#endif
