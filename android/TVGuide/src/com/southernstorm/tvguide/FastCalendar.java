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

import java.util.Calendar;
import java.util.GregorianCalendar;

/**
 * Faster version of GregorianCalendar that only works for local time and
 * simple equality comparisons.
 */
public class FastCalendar {

    public int year, month, day, hour, minute, second;
    
    public FastCalendar() {
        this(new GregorianCalendar());
    }
    
    public FastCalendar(int year, int month, int day) {
        this.year = year;
        this.month = month;
        this.day = day;
        this.hour = 0;
        this.minute = 0;
        this.second = 0;
    }
    
    public FastCalendar(int year, int month, int day, int hour, int minute, int second) {
        this.year = year;
        this.month = month;
        this.day = day;
        this.hour = hour;
        this.minute = minute;
        this.second = second;
    }

    public FastCalendar(Calendar calendar) {
        this.year = calendar.get(Calendar.YEAR);
        this.month = calendar.get(Calendar.MONTH);
        this.day = calendar.get(Calendar.DAY_OF_MONTH);
        this.hour = calendar.get(Calendar.HOUR_OF_DAY);
        this.minute = calendar.get(Calendar.MINUTE);
        this.second = calendar.get(Calendar.SECOND);
    }
    
    public boolean equals(Object _other) {
        if (!(_other instanceof FastCalendar))
            return false;
        FastCalendar other = (FastCalendar)_other; 
        return this.year == other.year && this.month == other.month &&
               this.day == other.day && this.hour == other.hour &&
               this.minute == other.minute && this.second == other.second;
    }
    
    public int hashCode() {
        return (month * 30 + day) * 24 * 60 + hour * 60 + minute;
    }
    
    public Calendar toCalendar() {
        return new GregorianCalendar(year, month, day, hour, minute, second);
    }
}
