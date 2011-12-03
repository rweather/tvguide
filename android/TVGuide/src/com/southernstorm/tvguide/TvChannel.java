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

import java.io.InputStream;
import java.io.IOException;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

public class TvChannel {

    private String id;
    private String name;
    private boolean convertTimezone;
    private List<String> baseUrls;
    private Map< Calendar, List<TvProgramme> > programmes;

    public TvChannel() {
        this.programmes = new TreeMap< Calendar, List<TvProgramme> >();
    }

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public boolean getConvertTimezone() { return convertTimezone; }
    public void setConvertTimezone(boolean convert) { this.convertTimezone = convert; }
    
    public List<String> getBaseUrls() {
        return baseUrls;
    }

    public void setBaseUrls(List<String> baseUrls) {
        this.baseUrls = baseUrls;
    }

    /**
     * Loads programme details from an XML input stream.
     *
     * @param date the date the programmes are for
     * @param stream the input stream
     */
    public void loadProgrammesFromXml(Calendar date, InputStream stream) {
        List<TvProgramme> progs = new ArrayList<TvProgramme>();
        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            parser.setInput(stream, null);
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG &&
                        parser.getName().equals("programme")) {
                    // Parse the contents of a <programme> element.
                    eventType = parser.getEventType();
                    TvProgramme prog = new TvProgramme();
                    prog.load(parser, convertTimezone);
                    progs.add(prog);
                    eventType = parser.getEventType();
                } else {
                    // Skip unknown element.
                    eventType = parser.next();
                }
            }
        } catch (XmlPullParserException e) {
            // Ignore - just stop parsing at the first error.
        } catch (IOException e) {
        }
        this.programmes.put(date, progs);
    }
    
    /**
     * Gets the list of programmes for a specific day, from 6:00 AM one day to 6:00 AM the next.
     * 
     * @param date the date to fetch
     * @return the list of programmes, or null if none available
     */
    public List<TvProgramme> programmesForDay(Calendar date) {
        Calendar nextDay = (Calendar)date.clone();
        nextDay.add(Calendar.DAY_OF_MONTH, 1);
        List<TvProgramme> dayProgs = programmes.get(date);
        List<TvProgramme> nextDayProgs = programmes.get(nextDay);
        List<TvProgramme> progs = null;
        if (dayProgs != null) {
            int start = 0;
            while (start < dayProgs.size()) {
                TvProgramme prog = dayProgs.get(start);
                if (prog.getStart().get(Calendar.HOUR_OF_DAY) >= 6)
                    break;
                if (prog.getStop().get(Calendar.HOUR_OF_DAY) > 6)
                    break;
                if (prog.getStop().get(Calendar.HOUR_OF_DAY) == 6 && prog.getStop().get(Calendar.MINUTE) > 0)
                    break;
                ++start;
            }
            if (start < dayProgs.size()) {
                progs = new ArrayList<TvProgramme>();
                progs.addAll(dayProgs.subList(start, dayProgs.size()));
            }
        }
        if (nextDayProgs != null) {
            int end = 0;
            while (end < nextDayProgs.size()) {
                TvProgramme prog = nextDayProgs.get(end);
                if (prog.getStart().get(Calendar.HOUR_OF_DAY) >= 6)
                    break;
                ++end;
            }
            if (end > 0) {
                List<TvProgramme> prefix = nextDayProgs.subList(0, end);
                if (progs == null)
                    progs = new ArrayList<TvProgramme>();
                progs.addAll(prefix);
            }
        }
        return progs;
    }
}
