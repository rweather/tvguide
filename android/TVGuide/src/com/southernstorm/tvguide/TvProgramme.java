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

import java.util.List;

import org.xmlpull.v1.XmlPullParser;

import android.text.SpannableString;

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
    public void setDate(String date) { m_date = date; }

    public String getRating() { return m_rating; }
    public void setRating(String rating) { m_rating = rating; }

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

    public SpannableString getShortDescription() {
        RichTextFormatter formatter = new RichTextFormatter();
        formatter.append(m_title);
        formatter.setColor(0xFF606060);
        if (m_date != null && !m_date.equals("0"))
            formatter.append(" (" + m_date + ")");
        if (m_rating != null)
            formatter.append(" (" + m_rating + ")");
        if (m_subTitle != null) {
            formatter.nl();
            formatter.setItalic(true);
            formatter.append(m_subTitle);
            formatter.setItalic(false);
        }
        return formatter.toSpannableString();
    }

    public SpannableString getLongDescription() {
        RichTextFormatter formatter = new RichTextFormatter();
        formatter.setColor(0xFF606060);
        if (m_description != null)
            formatter.append(m_description);
        return formatter.toSpannableString();
    }

    // Internal state.
    private long m_start;
    private long m_stop;
    private String m_title;
    private String m_subTitle;
    private String m_description;
    private String m_date;
    private String m_rating;
    private List<String> m_directors;
    private List<String> m_actors;
    private List<String> m_presenters;
    private List<String> m_categories;
}
