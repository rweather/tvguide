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

package com.southernstorm.tvguide;

import java.lang.String;
import java.util.List;
import org.xmlpull.v1.XmlPullParser;

public class TvProgramme {

    public TvProgramme() {
        m_start = -1;
        m_stop = -1;
    }

    public long getStart() { return m_start; }
    public void setStart(long start) { m_start = start; }

    public long getStop() { return m_stop; }
    public void setStop(long stop) { m_stop = stop; }

    public String getTitle() { return m_title; }
    public void setTitle(String title) { m_title = title; }

    public String getSubTitle() { return m_subTitle; }
    public void setSubTitle(String subTitle) { m_subTitle = subTitle; }

    public String getDescription() { return m_description; }
    public void setDescription(String desc) { m_description = desc; }

    public String getDate() { return m_date; }
    public List<String> getDirectors() { return m_directors; }
    public List<String> getActors() { return m_actors; }
    public List<String> getPresenters() { return m_presenters; }
    public List<String> getCategories() { return m_categories; }

    /** Loads the programme details from an XML input stream.
     *
     * @param parser Pull parser containing the input.  Must be positioned on the programme element.
     *
     * When this method exits, the parser will be positioned just after the programme end element.
     */
    public void load(XmlPullParser parser) {
    }

    // Internal state.
    private long m_start;
    private long m_stop;
    private String m_title;
    private String m_subTitle;
    private String m_description;
    private String m_date;
    private List<String> m_directors;
    private List<String> m_actors;
    private List<String> m_presenters;
    private List<String> m_categories;
}
