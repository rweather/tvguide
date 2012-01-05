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

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.database.DataSetObserver;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

public class EditBookmarkDayOfWeekAdapter implements SpinnerAdapter {
    
    private Context context;
    private List<Integer> dayMasks;
    private List<String> dayNames;
    private List<DataSetObserver> observers;
    
    private static final int[] standardMasks = {
        TvBookmark.ANY_DAY_MASK,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80,
        TvBookmark.MON_TO_FRI_MASK,
        TvBookmark.SAT_AND_SUN_MASK
    };

    public EditBookmarkDayOfWeekAdapter(Context context) {
        this.context = context;
        dayMasks = new ArrayList<Integer>();
        dayNames = new ArrayList<String>();
        for (int index = 0; index < standardMasks.length; ++index) {
            dayMasks.add(standardMasks[index]);
            dayNames.add(TvBookmark.getDayOfWeekMaskName(standardMasks[index], true));
        }
        observers = new ArrayList<DataSetObserver>();
    }

    public int getMaskForPosition(int position) {
        if (position >= 0 && position < dayMasks.size())
            return dayMasks.get(position);
        else
            return TvBookmark.ANY_DAY_MASK;
    }
    
    public int getPositionForMask(int mask) {
        for (int index = 0; index < dayMasks.size(); ++index) {
            if (dayMasks.get(index) == mask)
                return index;
        }
        dayMasks.add(mask);
        dayNames.add(TvBookmark.getDayOfWeekMaskName(mask, true));
        for (DataSetObserver observer: observers)
            observer.onChanged();
        return dayMasks.size() - 1;
    }
    
    public int getCount() {
        return dayMasks.size();
    }

    public Object getItem(int position) {
        return position;
    }

    public long getItemId(int position) {
        return position;
    }

    public int getItemViewType(int position) {
        return 0;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        TextView view;
        if (convertView != null)
            view = (TextView)convertView;
        else
            view = new TextView(context);
        view.setText(dayNames.get(position));
        view.setTextColor(0xFF000000);
        return view;
    }

    public int getViewTypeCount() {
        return 1;
    }

    public boolean hasStableIds() {
        return false;
    }

    public boolean isEmpty() {
        return dayMasks.isEmpty();
    }

    public void registerDataSetObserver(DataSetObserver observer) {
        observers.add(observer);
    }

    public void unregisterDataSetObserver(DataSetObserver observer) {
        observers.remove(observer);
    }

    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        return getView(position, convertView, parent);
    }
}
