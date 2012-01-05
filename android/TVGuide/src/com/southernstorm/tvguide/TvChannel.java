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

package com.southernstorm.tvguide;

import java.io.File;
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

import android.graphics.drawable.Drawable;
import android.os.Bundle;

public class TvChannel implements Comparable<TvChannel> {

    private class DataFor {
        public FastCalendar date;
        public FastCalendar lastModified;
        
        public DataFor(FastCalendar date, FastCalendar lastModified) {
            this.date = date;
            this.lastModified = lastModified;
        }
    }
    
    public static final int NO_NUMBER = 0x7FFFFFFF;
    private String id;
    private String commonId;
    private String name;
    private String region;
    private String numbers;
    private int primaryChannelNumber;
    private int hiddenState;
    private int defaultHiddenState;
    private int iconResource;
    private String iconFile;
    private String iconSource;
    private Drawable iconFileDrawable;
    private boolean convertTimezone;
    private ArrayList<String> baseUrls;
    private Map< Calendar, List<TvProgramme> > programmes;
    private ArrayList<String> otherChannelsList;
    private List<DataFor> dataForList;
    
    public static final int NOT_HIDDEN = 0;
    public static final int HIDDEN = 1;
    public static final int HIDDEN_BY_REGION = 2;

    public TvChannel() {
        this.primaryChannelNumber = TvChannel.NO_NUMBER;
        this.hiddenState = HIDDEN;
        this.defaultHiddenState = HIDDEN;
        this.iconResource = 0;
        this.baseUrls = new ArrayList<String>();
        this.programmes = new TreeMap< Calendar, List<TvProgramme> >();
        this.otherChannelsList = new ArrayList<String>();
        this.dataForList = new ArrayList<DataFor>();
    }

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getCommonId() { return commonId; }
    public void setCommonId(String id) { this.commonId = id; }

    public List<String> getOtherChannelsList() { return otherChannelsList; }
    public void setOtherChannelsList(ArrayList<String> list) { this.otherChannelsList = list; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getRegion() { return region; }
    public void setRegion(String region) { this.region = region; }
    
    public String getNumbers() { return numbers; }
    public void setNumbers(String numbers) { this.numbers = numbers; }

    public int getPrimaryChannelNumber() { return primaryChannelNumber; }
    public void setPrimaryChannelNumber(int number) { this.primaryChannelNumber = number; }
    
    public int getHiddenState() { return hiddenState; }
    public void setHiddenState(int hiddenState) { this.hiddenState = hiddenState; }
    
    public int getDefaultHiddenState() { return defaultHiddenState; }
    public void setDefaultHiddenState(int hiddenState) { this.defaultHiddenState = hiddenState; }
    
    public int getIconResource() { return iconResource; }
    public void setIconResource(int id) { this.iconResource = id; }

    public String getIconFile() { return iconFile; }
    public void setIconFile(String iconFile) { this.iconFile = iconFile; this.iconFileDrawable = null; }

    public String getIconSource() { return iconSource; }
    public void setIconSource(String iconSource) { this.iconSource = iconSource; }
    
    public void clearDataFor() { dataForList.clear(); }
    public void addDataFor(FastCalendar date, FastCalendar lastModified) {
        dataForList.add(new DataFor(date, lastModified));
    }
    
    private static boolean equalsDate(FastCalendar c1, Calendar c2) {
        if (c1.day != c2.get(Calendar.DAY_OF_MONTH))
            return false;
        if (c1.month != c2.get(Calendar.MONTH))
            return false;
        return c1.year == c2.get(Calendar.YEAR);
    }
    
    public boolean hasDataFor() { return !dataForList.isEmpty(); }
    public boolean hasDataFor(Calendar date) {
        for (int index = 0; index < dataForList.size(); ++index) {
            if (equalsDate(dataForList.get(index).date, date))
                return true;
        }
        return false;
    }
    
    public Calendar dayLastModified(Calendar date) {
        for (int index = 0; index < dataForList.size(); ++index) {
            if (equalsDate(dataForList.get(index).date, date))
                return dataForList.get(index).lastModified.toCalendar();
        }
        return null;
    }
    
    public Drawable getIconFileDrawable() {
        if (iconFile == null)
            return null;
        else if (iconFileDrawable != null)
            return iconFileDrawable;
        File file = new File(iconFile);
        if (!file.exists())
            return null;
        iconFileDrawable = Drawable.createFromPath(iconFile);
        return iconFileDrawable;
    }
    
    public boolean iconNeedsFetching() {
        if (iconFile == null || iconFileDrawable != null)
            return false;
        File file = new File(iconFile);
        return !file.exists();
    }
    
    public boolean getConvertTimezone() { return convertTimezone; }
    public void setConvertTimezone(boolean convert) { this.convertTimezone = convert; }
    
    public int compareTo(TvChannel other) {
        // Compares using Australian channel number rules:
        // 2, 20, 21, 3, 30, 31, ..., 9, 90, 91, ..., 10, 11, 1, ...
        int num1 = getPrimaryChannelNumber();
        int num2 = other.getPrimaryChannelNumber();
        if (num1 >= 100 || num2 >= 100) {
            if (num1 < num2)
                return -1;
            else if (num1 > num2)
                return 1;
        } else {
            int high1, high2;
            if (num1 == 1)
                high1 = 11;
            else if (num1 < 10)
                high1 = num1;
            else if (num1 < 20)
                high1 = 10;
            else
                high1 = num1 / 10;
            if (num2 == 1)
                high2 = 11;
            else if (num2 < 10)
                high2 = num2;
            else if (num2 < 20)
                high2 = 10;
            else
                high2 = num2 / 10;
            if (high1 < high2)
                return -1;
            else if (high1 > high2)
                return 1;
            if (num1 < num2)
                return -1;
            else if (num1 > num2)
                return 1;
        }
        return name.compareToIgnoreCase(other.getName());
    }
    
    public List<String> getBaseUrls() {
        return baseUrls;
    }

    public void setBaseUrls(List<String> baseUrls) {
        this.baseUrls.clear();
        this.baseUrls.addAll(baseUrls);
    }

    /**
     * Clears all programmes that were cached for this channel, to save memory.
     */
    public void clearProgrammes() {
        programmes.clear();
    }
    
    /**
     * Loads programme details from an XML input stream.
     *
     * @param date the date the programmes are for
     * @param stream the input stream
     */
    public void loadProgrammesFromXml(Calendar date, InputStream stream) {
        List<TvProgramme> progs = new ArrayList<TvProgramme>();
        Utils.clearTimeZone(); // Force the local timezone to be reloaded between XML files just in case.
        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            parser.setInput(stream, null);
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG &&
                        parser.getName().equals("programme")) {
                    // Parse the contents of a <programme> element.
                    TvProgramme prog = new TvProgramme(this);
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
                Calendar startTime = prog.getStart();
                if (startTime.get(Calendar.HOUR_OF_DAY) >= 6) {
                    // It is possible that timezone adjustment has pushed some of the early
                    // morning programmes into the previous day.
                    if (startTime.get(Calendar.DAY_OF_MONTH) == date.get(Calendar.DAY_OF_MONTH))
                        break;
                    ++start;
                    continue;
                }
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
                if (prog.getStart().get(Calendar.HOUR_OF_DAY) >= 6) {
                    // It is possible that timezone adjustment has pushed some of the early
                    // morning programmes into the previous day.
                    if (prog.getStart().get(Calendar.DAY_OF_MONTH) != date.get(Calendar.DAY_OF_MONTH))
                        break;
                }
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
    
    /**
     * Converts the information in this channel object into a bundle for passing
     * to a new activity.
     * 
     * @return the bundle containing information about this channel
     */
    public Bundle toBundle() {
        Bundle bundle = new Bundle();
        bundle.putString("id", id);
        return bundle;
    }
    
    /**
     * Creates a new channel object from the information in a bundle.
     * 
     * @param bundle the bundle containing the channel information
     * @return the channel object
     */
    public static TvChannel fromBundle(Bundle bundle) {
        return TvChannelCache.getInstance().getChannel(bundle.getString("id"));
    }
    
    /**
     * Determine if this channel has a specific identifier, or if this channel
     * has the same common identifier as another channel with the specific identifier.
     * 
     * @param id the channel identifier to check
     * @return true if the same channel, false if not
     */
    public boolean isSameChannel(String id) {
        if (id == null)
            return false;
        if (this.id != null && this.id.equals(id))
            return true;
        if (this.commonId == null)
            return false;
        return otherChannelsList.contains(id);
    }
    
    /**
     * Determine if there are on-air bookmarks for this channel on a specific day.
     * 
     * Note: bookmarks for "any time" or "any channel" are ignored.
     * 
     * @param weekday the weekday to check
     * @return true if there are bookmarks, false otherwise
     */
    public boolean haveBookmarksForDay(int weekday) {
        return TvBookmarkManager.getInstance().haveBookmarksForDay(this, weekday);
    }
}
