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

#include "tvprogrammefilter.h"

bool TvProgrammeFilter::match(const TvProgramme *prog) const
{
    bool matchTitle = (m_combineMode == CombineAnd);
    bool matchEpisodeName = (m_combineMode == CombineAnd);
    bool matchDescription = (m_combineMode == CombineAnd);
    bool matchCredit = (m_combineMode == CombineAnd);
    bool matchCategory = (m_combineMode == CombineAnd);
    bool haveCriteria = false;
    if (!m_title.isEmpty()) {
        matchTitle = prog->containsSearchString(m_title, TvProgramme::SearchTitle);
        haveCriteria = true;
    }
    if (!m_episodeName.isEmpty()) {
        matchEpisodeName = prog->containsSearchString(m_episodeName, TvProgramme::SearchEpisodeName);
        haveCriteria = true;
    }
    if (!m_description.isEmpty()) {
        matchDescription = prog->containsSearchString(m_description, TvProgramme::SearchDescription);
        haveCriteria = true;
    }
    if (!m_credits.isEmpty()) {
        matchCredit = prog->containsSearchString(m_credits, TvProgramme::SearchCredits);
        haveCriteria = true;
    }
    if (!m_categories.isEmpty()) {
        matchCategory = prog->containsSearchString(m_categories, TvProgramme::SearchCategories);
        haveCriteria = true;
    }
    if (m_moviesOnly) {
        if (!prog->isMovie())
            return false;
    }
    if (!haveCriteria)
        return true;        // No criteria always means "match all".
    if (m_combineMode == CombineAnd) {
        return matchTitle &&
               matchEpisodeName &&
               matchDescription &&
               matchCredit &&
               matchCategory;
    } else {
        return matchTitle ||
               matchEpisodeName ||
               matchDescription ||
               matchCredit ||
               matchCategory;
    }
}

bool TvProgrammeFilter::isDefault() const
{
    return m_title.isEmpty() &&
           m_episodeName.isEmpty() &&
           m_description.isEmpty() &&
           m_credits.isEmpty() &&
           m_categories.isEmpty() &&
           !m_moviesOnly;
}

QStringList TvProgrammeFilter::splitAndTrim(const QString &str)
{
    QStringList list = str.split(QLatin1Char(','));
    QStringList list2;
    foreach (QString s, list) {
        QString s2 = s.trimmed();
        if (!s2.isEmpty())
            list2.append(s2);
    }
    return list2;
}

void TvProgrammeFilter::setCredit(const QString &credit)
{
    m_credit = credit;
    m_credits = splitAndTrim(credit);
}

void TvProgrammeFilter::setCategory(const QString &category)
{
    m_category = category;
    m_categories = splitAndTrim(category);
}
