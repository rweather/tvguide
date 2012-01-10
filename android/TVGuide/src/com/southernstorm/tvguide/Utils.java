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

import java.io.IOException;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;

import android.graphics.Color;

public class Utils {

    /**
     * Format a time value, including the AM/PM indicator.
     * 
     * @param time the time to format
     * @return the formatted time
     */
    public static String formatTime(Calendar time) {
        // TODO: 24 hour clock support
        int hour = time.get(Calendar.HOUR_OF_DAY);
        int minute = time.get(Calendar.MINUTE);
        String ampm = (hour < 12 ? " AM" : " PM");
        if (hour == 0)
            hour = 12;
        else if (hour > 12)
            hour -= 12;
        if (minute < 10)
            return hour + ":0" + minute + ampm;
        else
            return hour + ":" + minute + ampm;
    }

    /**
     * Format a time value, including the AM/PM indicator.
     * 
     * @param time the time to format, as the number of seconds since midnight
     * @return the formatted time
     */
    public static String formatTime(int time) {
        // TODO: 24 hour clock support
        int hour = time / (60 * 60);
        int minute = (time / 60) % 60;
        String ampm = (hour < 12 ? " AM" : " PM");
        if (hour == 0)
            hour = 12;
        else if (hour > 12)
            hour -= 12;
        if (minute < 10)
            return hour + ":0" + minute + ampm;
        else
            return hour + ":" + minute + ampm;
    }

    /**
     * Format a time value for the programme list.
     * 
     * @param time the time to format
     * @return the formatted time
     */
    public static String formatTimeProgrammeList(Calendar time) {
        // TODO: 24 hour clock support
        int hour = time.get(Calendar.HOUR_OF_DAY);
        int minute = time.get(Calendar.MINUTE);
        String ampm = (hour < 12 ? "\n   AM" : "\n   PM");
        if (hour == 0)
            hour = 12;
        else if (hour > 12)
            hour -= 12;
        if (hour < 10 && minute < 10)
            return "  " + hour + ":0" + minute + ampm;
        else if (hour < 10)
            return "  " + hour + ":" + minute + ampm;
        else if (minute < 10)
            return " " + hour + ":0" + minute + ampm;
        else
            return " " + hour + ":" + minute + ampm;
    }

    /**
     * Parses a fixed-length numeric field out of a string.
     * 
     * @param str the string
     * @param posn the starting position within the string
     * @param length the length of the field
     * @return the numeric value
     */
    public static int parseField(String str, int posn, int length) {
        int value = 0;
        while (length-- > 0) {
            if (posn >= str.length())
                break;
            char ch = str.charAt(posn++);
            if (ch >= '0' && ch <= '9')
                value = value * 10 + ch - '0';
        }
        return value;
    }

    private static TimeZone cachedTimeZone = null;
    private static int cachedOffset = 0;
    
    private static int parseTZField(String str, int posn) {
        while (posn < str.length() && str.charAt(posn) == ' ') {
            ++posn;
        }
        if (posn >= str.length())
            return cachedOffset;
        boolean positive;
        if (str.charAt(posn) == '+') {
            positive = true;
            ++posn;
        } else if (str.charAt(posn) == '-') {
            positive = false;
            ++posn;
        } else {
            return cachedOffset;
        }
        int raw = parseField(str, posn, 4);
        int value = ((raw / 100) * 60 + (raw % 100)) * (60 * 1000);
        if (positive)
            return value;
        else
            return -value;
    }
    
    /**
     * Parses a date/time string from within an XMLTV stream.  Only the local
     * time part of the value is parsed. Timezone specifications are ignored.
     * 
     * @param str the string
     * @param convertTimezone true to convert date/time values to local time
     * @return the date/time value as a Calendar object
     */
    public static Calendar parseDateTime(String str, boolean convertTimezone) {
        // Format looks like: 20111209060000 +1100
        if (str == null)
            return null;
        int year = parseField(str, 0, 4);
        int month = parseField(str, 4, 2);
        int day = parseField(str, 6, 2);
        int hour = parseField(str, 8, 2);
        int minute = parseField(str, 10, 2);
        int second = parseField(str, 12, 2);
        if (convertTimezone) {
            if (cachedTimeZone == null) {
                cachedTimeZone = TimeZone.getDefault();
                cachedOffset = cachedTimeZone.getOffset(System.currentTimeMillis());
            }
            int tz = parseTZField(str, 14);
            if (tz != cachedOffset) {
                Calendar calendar = new GregorianCalendar(year, month - 1, day, hour, minute, second);
                calendar.add(Calendar.MILLISECOND, cachedOffset - tz);
                return calendar;
            }
        }
        return new GregorianCalendar(year, month - 1, day, hour, minute, second);
    }

    public static void clearTimeZone() {
        cachedTimeZone = null;
    }

    /**
     * Parses a date/time string from within an XMLTV stream.  Only the local
     * time part of the value is parsed. Timezone specifications are ignored.
     * 
     * @param str the string
     * @return the date/time value as a FastCalendar object
     */
    public static FastCalendar parseDateTimeFast(String str) {
        // Format looks like: 20111209060000 +1100
        if (str == null)
            return null;
        int year = parseField(str, 0, 4);
        int month = parseField(str, 4, 2);
        int day = parseField(str, 6, 2);
        int hour = parseField(str, 8, 2);
        int minute = parseField(str, 10, 2);
        int second = parseField(str, 12, 2);
        return new FastCalendar(year, month - 1, day, hour, minute, second);
    }

    private static void appendField(StringBuilder builder, int value, int digits) {
        int divider = 1;
        while (digits-- > 1)
            divider *= 10;
        while (divider != 0) {
            builder.append((char)('0' + (value / divider) % 10));
            divider /= 10;
        }
    }

    /**
     * Formats a date/time value as a string.
     * 
     * @param date the date to format
     * @return the formatted date
     */
    public static String formatDateTime(Calendar date) {
        StringBuilder builder = new StringBuilder(16);
        appendField(builder, date.get(Calendar.YEAR), 4);
        appendField(builder, date.get(Calendar.MONTH) + 1, 2);
        appendField(builder, date.get(Calendar.DAY_OF_MONTH), 2);
        appendField(builder, date.get(Calendar.HOUR_OF_DAY), 2);
        appendField(builder, date.get(Calendar.MINUTE), 2);
        appendField(builder, date.get(Calendar.SECOND), 2);
        return builder.toString();
    }

    /**
     * Gets the full text contents of an XML element.  The parser is assumed to be
     * positions on the start element.  Upon exit, the parser will be positioned
     * on the end element.
     * 
     * @param parser the XML parser to read from
     * @param name the name of the start element
     * @return the full text contents, or null if empty
     * @throws XmlPullParserException error in xml data
     * @throws IOException error reading the xml data
     */
    public static String getContents(XmlPullParser parser, String name) throws XmlPullParserException, IOException {
        String str = null;
        int eventType = parser.next();
        while (eventType != XmlPullParser.END_DOCUMENT) {
            if (eventType == XmlPullParser.TEXT) {
                String nextStr = parser.getText();
                if (str == null)
                    str = nextStr;
                else
                    str = str + nextStr;
            } else if (eventType == XmlPullParser.END_TAG && parser.getName().equals(name)) {
                break;
            }
            eventType = parser.next();
        }
        if (str != null && str.length() == 0)
            str = null;
        return str;
    }
    
    public static void writeContents(XmlSerializer serializer, String name, String contents) throws IOException {
        if (contents == null)
            return;
        serializer.startTag(null, name);
        serializer.text(contents);
        serializer.endTag(null, name);
    }
    
    public static void writeEmptyTag(XmlSerializer serializer, String name) throws IOException {
        serializer.startTag(null, name);
        serializer.endTag(null, name);
    }
    
    /**
     * Null-safe version of string equality.
     * 
     * @param s1 the first string
     * @param s2 the second string
     * @return true if equal, false if not
     */
    public static boolean stringEquals(String s1, String s2) {
        if (s1 == null || s2 == null)
            return s1 == s2;
        else
            return s1.equals(s2);
    }
    
    /**
     * Null-safe version of string comparison.
     * 
     * @param s1 the first string
     * @param s2 the second string
     * @return the comparison result
     */
    public static int stringCompareIgnoreCase(String s1, String s2) {
        if (s1 == null)
            return (s2 == null ? 0 : -1);
        else if (s2 == null)
            return 1;
        else
            return s1.compareToIgnoreCase(s2);
    }
    
    /**
     * Null-safe version of date equality.
     * 
     * @param c1 the first date
     * @param c2 the second date
     * @return true if equal, false if not
     */
    public static boolean dateEquals(Calendar c1, Calendar c2) {
        if (c1 == null || c2 == null)
            return c1 == c2;
        else
            return c1.equals(c2);
    }
    
    /**
     * Lighten a color by a specific factor.
     * 
     * @param color the color to lighten
     * @param factor the factor to apply; e.g. 150 makes the color 50% brighter, 50
     * makes the color 50% darker, 100 makes no change.
     * 
     * @return the lightened version of the color
     */
    public static int lightenColor(int color, int factor) {
        float[] hsv = new float [3];
        Color.colorToHSV(color, hsv);
        float s = hsv[1];
        float v = hsv[2] * factor / 100.0f;
        if (v > 1.0f) {
            // Value has overflowed, so adjust the saturation instead.
            s -= v - 1.0f;
            if (s < 0)
                s = 0;
            v = 1.0f;
        }
        hsv[1] = s;
        hsv[2] = v;
        return Color.HSVToColor(hsv);
    }
}
