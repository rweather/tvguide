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

import java.io.IOException;
import java.util.Calendar;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;

/**
 * Record of a single show that is "ticked" to remind the user to watch it.
 * 
 * Ticks differ from bookmarks in that a tick applies to a single showing of a programme.
 * It is intended for one-off shows like movies and documentaries.
 * 
 * Ticks automatically expire after 30 days.
 */
public class TvTick {

    private String title;
    private String channelId;
    private Calendar startTime;
    private Calendar timestamp;

    /**
     * Constructs a new tick object.
     */
    public TvTick() {
    }

    /**
     * Gets the title of the programme to match with this tick object.
     * 
     * @return the programme title
     */
    public String getTitle() {
        return title;
    }
    
    /**
     * Sets the title of the programme to match with this tick object.
     * Case is significant when matching against programmes.
     * 
     * @param title the programme title
     */
    public void setTitle(String title) {
        this.title = title;
    }

    /**
     * Gets the identifier of the channel to match with this tick object.
     * 
     * @return the channel identifier
     */
    public String getChannelId() {
        return channelId;
    }
    
    /**
     * Sets the identifier of the channel to match with this tick object.
     * 
     * @param channelId the channel identifier
     */
    public void setChannelId(String channelId) {
        this.channelId = channelId;
    }
    
    /**
     * Gets the starting time of the programme to match against.
     * 
     * @return the start time
     */
    public Calendar getStartTime() {
        return startTime;
    }
    
    /**
     * Sets the starting time of the programme to match against.
     * 
     * @param startTime the start time
     */
    public void setStartTime(Calendar startTime) {
        this.startTime = startTime;
    }
    
    /**
     * Gets the timestamp of when this tick object was created.  This is used to
     * determine when the tick object should expire.
     * 
     * @return the timestamp
     */
    public Calendar getTimestamp() {
        return timestamp;
    }
    
    /**
     * Sets the timestamp of when this tick object was created.
     * 
     * @param timestamp the timestamp
     */
    public void setTimestamp(Calendar timestamp) {
        this.timestamp = timestamp;
    }

    /**
     * Determine if this tick object matches a specific programme.
     * 
     * @param programme the programme to match against
     * @return true if the programme matches, false if not
     */
    public boolean match(TvProgramme programme) {
        if (!startTime.equals(programme.getStart()))
            return false;
        if (!channelId.equals(programme.getChannel().getId()))
            return false;
        return title.equals(programme.getTitle());
    }

    /**
     * Loads the tick details from an XML input stream.
     *
     * When this method exits, the parser will be positioned just after
     * the tick end element.
     *
     * @param parser Pull parser containing the input.  Must be positioned
     * on the tick element.
     * @throws XmlPullParserException error in xml data
     * @throws IOException error reading the xml data
     */
    public void loadFromXml(XmlPullParser parser) throws XmlPullParserException, IOException {
        title = null;
        channelId = null;
        startTime = null;
        timestamp = null;
        int eventType = parser.next();
        while (eventType != XmlPullParser.END_DOCUMENT) {
            if (eventType == XmlPullParser.START_TAG) {
                String name = parser.getName();
                if (name.equals("title")) {
                    title = Utils.getContents(parser, name);
                } else if (name.equals("channel-id")) {
                    channelId = Utils.getContents(parser, name);
                } else if (name.equals("start-time")) {
                    startTime = Utils.parseDateTime(Utils.getContents(parser, name), false);
                } else if (name.equals("timestamp")) {
                    timestamp = Utils.parseDateTime(Utils.getContents(parser, name), false);
                }
            } else if (eventType == XmlPullParser.END_TAG && parser.getName().equals("tick")) {
                break;
            }
            eventType = parser.next();
        }
    }

    /**
     * Saves this tick to an XML stream.
     * 
     * @param serializer the serializer for the XML stream
     * @throws IOException failed to write to the XML stream
     */
    public void saveToXml(XmlSerializer serializer) throws IOException {
        serializer.startTag(null, "tick");
        Utils.writeContents(serializer, "title", title);
        Utils.writeContents(serializer, "channel-id", channelId);
        Utils.writeContents(serializer, "start-time", Utils.formatDateTime(startTime));
        Utils.writeContents(serializer, "timestamp", Utils.formatDateTime(timestamp));
        serializer.endTag(null, "tick");
    }
}
