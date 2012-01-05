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

public enum TvScrollTime {
    NOW,
    MORNING,
    AFTERNOON,
    NIGHT,
    LATE_NIGHT;
    
    public int toTime() {
        switch (this) {
        case MORNING: return 6 * 60 * 60;
        case AFTERNOON: return 12 * 60 * 60;
        case NIGHT: return 18 * 60 * 60;
        case LATE_NIGHT: return 24 * 60 * 60;
        default: break;
        }
        GregorianCalendar calendar = new GregorianCalendar();
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        if (hour < 5)
            hour += 24;     // Assume we want to see "late night" rather than "early morning"
        int minute = calendar.get(Calendar.MINUTE);
        if (minute < 15)    // Align on next lower 15 minute boundary.
            minute = 0;
        else if (minute < 30)
            minute = 15;
        else if (minute < 45)
            minute = 30;
        else
            minute = 45;
        return hour * 60 * 60 + minute * 60;
    }
}
