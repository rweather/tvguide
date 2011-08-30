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

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.database.DataSetObserver;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ExpandableListAdapter;
import android.widget.TextView;

public class TvProgrammeListAdapter implements ExpandableListAdapter {

    private List<TvProgramme> programmes;
    private List<DataSetObserver> observers;
    private LayoutInflater inflater;
    private static final int timeColorMorning = R.drawable.time_color_morning;
    private static final int timeColorAfternoon = R.drawable.time_color_afternoon;
    private static final int timeColorNight = R.drawable.time_color_night;
    private static final int timeColorLateNight = R.drawable.time_color_late_night;

    public TvProgrammeListAdapter(Context context) {
        programmes = new ArrayList<TvProgramme>();
        observers = new ArrayList<DataSetObserver>();
        inflater = LayoutInflater.from(context);
    }

    public List<TvProgramme> getProgrammes() {
        return programmes;
    }

    public void setProgrammes(List<TvProgramme> programmes) {
        this.programmes = programmes;
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }

    public Object getChild(int position, int child) {
        return child;
    }

    public long getChildId(int position, int child) {
        return child;
    }

    public long getCombinedChildId(long group, long child) {
        return group * 8 + child + 1;
    }

    public boolean isChildSelectable(int position, int child) {
        return true;
    }

    public int getChildrenCount(int position) {
        // No children yet - we use a different view for the expanded version.
        return 0;
    }

    public int getGroupCount() {
        return programmes.size();
    }

    public Object getGroup(int index) {
        return index;
    }

    public long getGroupId(int position) {
        return position;
    }

    public long getCombinedGroupId(long group) {
        return group * 8;
    }

    public View getChildView(int position, int child, boolean isLastChild, View convertView, ViewGroup parent) {
        return null;
    }

    private class GroupViewDetails {
        public boolean isExpanded;
        public TextView time;
        public TextView line1;
        public TextView line2;
        public TextView line3;
    }

    public View getGroupView(int position, boolean isExpanded, View convertView, ViewGroup parent) {
        GroupViewDetails view = null;
        if (convertView != null) {
            view = (GroupViewDetails)convertView.getTag();
            if (view.isExpanded != isExpanded)
                convertView = null;
        }
        if (convertView == null) {
            if (isExpanded) {
                convertView = inflater.inflate(R.layout.programme_expand, null);
                view = new GroupViewDetails();
                view.isExpanded = isExpanded;
                view.time = (TextView)convertView.findViewById(R.id.progx_time);
                view.line1 = (TextView)convertView.findViewById(R.id.progx_line1);
                view.line2 = (TextView)convertView.findViewById(R.id.progx_line2);
                view.line3 = (TextView)convertView.findViewById(R.id.progx_line3);
            } else {
                convertView = inflater.inflate(R.layout.programme, null);
                view = new GroupViewDetails();
                view.isExpanded = isExpanded;
                view.time = (TextView)convertView.findViewById(R.id.prog_time);
                view.line1 = (TextView)convertView.findViewById(R.id.prog_line1);
                view.line2 = (TextView)convertView.findViewById(R.id.prog_line2);
            }
            convertView.setTag(view);
        }
        TvProgramme prog = programmes.get(position);
        long timeval = prog.getStart();
        int hour = (int)((timeval / 60) % 24);
        int minute = (int)(timeval % 60);
        if (hour < 6)
            view.time.setBackgroundResource(timeColorLateNight);
        else if (hour < 12)
            view.time.setBackgroundResource(timeColorMorning);
        else if (hour < 18)
            view.time.setBackgroundResource(timeColorAfternoon);
        else
            view.time.setBackgroundResource(timeColorNight);
        if (hour > 12)
            hour -= 12;
        if (hour < 10 && minute < 10)
            view.time.setText("  " + hour + ":0" + minute);
        else if (hour < 10)
            view.time.setText("  " + hour + ":" + minute);
        else if (minute < 10)
            view.time.setText(" " + hour + ":0" + minute);
        else
            view.time.setText(" " + hour + ":" + minute);
        view.line1.setText(prog.getTitle());
        view.line2.setText(prog.getSubTitle());
        if (isExpanded)
            view.line3.setText(prog.getDescription());
        return convertView;
    }

    public boolean hasStableIds() {
        return false;
    }

    public boolean isEmpty() {
        return programmes.isEmpty();
    }

    public void registerDataSetObserver(DataSetObserver observer) {
        observers.add(observer);
    }

    public void unregisterDataSetObserver(DataSetObserver observer) {
        observers.remove(observer);

    }

    public boolean areAllItemsEnabled() {
        return true;
    }

    public void onGroupCollapsed(int position) {}
    public void onGroupExpanded(int position) {}
}
