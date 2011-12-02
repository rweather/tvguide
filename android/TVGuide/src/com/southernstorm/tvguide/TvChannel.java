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
import java.util.List;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

public class TvChannel {

    private String id;
    private String name;
    private boolean convertTimezone;

    public TvChannel() {
    }

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public boolean getConvertTimezone() { return convertTimezone; }
    public void setConvertTimezone(boolean convert) { this.convertTimezone = convert; }
    
    /**
     * Loads programme details from an XML input stream.
     *
     * @param stream the input stream
     */
    public List<TvProgramme> loadProgrammes(InputStream stream) {
        List<TvProgramme> programmes = new ArrayList<TvProgramme>();
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
                    programmes.add(prog);
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
        return programmes;
    }
}
