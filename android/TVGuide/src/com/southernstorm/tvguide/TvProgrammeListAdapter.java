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
import java.util.Calendar;
import java.util.List;

import android.content.Context;
import android.database.DataSetObserver;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ExpandableListAdapter;
import android.widget.TextView;

public class TvProgrammeListAdapter implements ExpandableListAdapter {

    private Context context;
    private List<TvProgramme> programmes;
    private List<DataSetObserver> observers;
    private LayoutInflater inflater;
    private TvChannel channel;
    private List<Calendar> datesCovered;
    private static final int timeColorMorning = R.drawable.time_color_morning;
    private static final int timeColorAfternoon = R.drawable.time_color_afternoon;
    private static final int timeColorNight = R.drawable.time_color_night;
    private static final int timeColorLateNight = R.drawable.time_color_late_night;
    private static final List<TvProgramme> emptyProgrammes = new ArrayList<TvProgramme>();

    public TvProgrammeListAdapter(Context context) {
        this.context = context;
        programmes = emptyProgrammes;
        observers = new ArrayList<DataSetObserver>();
        inflater = LayoutInflater.from(context);
    }

    public List<TvProgramme> getProgrammes() {
        return programmes;
    }
    
    public void setProgrammes(List<TvProgramme> programmes) {
        if (programmes != null)
            this.programmes = programmes;
        else
            this.programmes = emptyProgrammes;
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }

    /**
     * Gets the channel that is currently being displayed by the programme list's view.
     * 
     * @return the current channel, or null if none
     */
    public TvChannel getChannel() {
        return channel;
    }

    /**
     * Sets the channel to display in this programme list's view.
     * 
     * @param channel the channel
     */
    public void setChannel(TvChannel channel) {
        this.channel = channel;
    }
    
    /**
     * Determine if a channel is covered by this programme list's view.
     * 
     * @param channel the channel
     * @return true if covered, false otherwise
     */
    public boolean isChannelCovered(TvChannel channel) {
        return this.channel == channel;
    }

    /**
     * Gets the list of dates that are covered by the programme list's current view.
     * If there is any change in the data for these dates, the view will be refreshed.
     * 
     * @return the list of dates, or null if none
     */
    public List<Calendar> getDatesCovered() {
        return datesCovered;
    }

    /**
     * Sets the list of dates that are covered by the programme list's current view.
     * If there is any change in the data for these dates, the view will be refreshed.
     * 
     * The first entry in the list must be the "primary date".
     * 
     * @param datesCovered the list of dates
     */
    public void setDatesCovered(List<Calendar> datesCovered) {
        this.datesCovered = datesCovered;
    }

    /**
     * Determine if a date is covered by this view.
     * 
     * @param date the date to test
     * @return true if the date is covered, false otherwise
     */
    public boolean isDateCovered(Calendar date) {
        return datesCovered != null && datesCovered.contains(date);
    }
    
    /**
     * Gets the primary display date for this view.
     * 
     * @return the primary display date
     */
    public Calendar getPrimaryDate() {
        if (datesCovered != null && datesCovered.size() > 0)
            return datesCovered.get(0);
        else
            return null;
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

    /**
     * Updates all programmes in the view.
     */
    public void updateAllProgrammes() {
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }

    private class GroupViewDetails {
        public boolean isExpanded;
        public TextView time;
        public TextView short_desc;
        public TextView long_desc;
    }

    private boolean crossesSixAM(TvProgramme prog) {
        if (prog.getStart().get(Calendar.HOUR_OF_DAY) >= 6)
            return false;
        if (prog.getStop().get(Calendar.HOUR_OF_DAY) >= 6)
            return true;
        return false;
    }

    private TvBookmarkMatch getDisplayMatch(int index) {
        TvProgramme prog = programmes.get(index);
        TvProgramme prev = (index > 0 ? programmes.get(index - 1) : null);
        TvProgramme next = (index < (programmes.size() - 1) ? programmes.get(index + 1) : null);
        TvBookmarkMatch result = prog.getBookmarkMatch();
        TvBookmark bookmark = prog.getBookmark();

        if (result == TvBookmarkMatch.ShouldMatch) {
            // Suppress failed matches either side of a successful match,
            // and remove redundant failed matches.
            if (prev != null && prev.getBookmark() == bookmark) {
                if (prev.getBookmarkMatch() != TvBookmarkMatch.NoMatch &&
                        prev.getBookmarkMatch() != TvBookmarkMatch.TickMatch)
                    result = TvBookmarkMatch.NoMatch;
            } else if (next != null && next.getBookmark() == bookmark) {
                if (next.getBookmarkMatch() != TvBookmarkMatch.ShouldMatch &&
                        next.getBookmarkMatch() != TvBookmarkMatch.NoMatch &&
                        next.getBookmarkMatch() != TvBookmarkMatch.TickMatch)
                    result = TvBookmarkMatch.NoMatch;
            }
        } else if (result == TvBookmarkMatch.TitleMatch) {
            // Partial match immediately before or after a full
            // match is labelled as an underrun or overrun.
            // Probably a double episode where one of the episodes
            // falls outside the normal bookmark range.
            if (prev != null && prev.getStop().equals(prog.getStart()) &&
                    prev.getBookmarkMatch() == TvBookmarkMatch.FullMatch &&
                    prev.getBookmark() == bookmark) {
                result = TvBookmarkMatch.Overrun;
            } else if (next != null && next.getStart().equals(prog.getStop()) &&
                       next.getBookmarkMatch() == TvBookmarkMatch.FullMatch &&
                       next.getBookmark() == bookmark) {
                result = TvBookmarkMatch.Underrun;
            }
        }

        return result;
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
                view.short_desc = (TextView)convertView.findViewById(R.id.progx_short_desc);
                view.long_desc = (TextView)convertView.findViewById(R.id.progx_long_desc);
            } else {
                convertView = inflater.inflate(R.layout.programme, null);
                view = new GroupViewDetails();
                view.isExpanded = isExpanded;
                view.time = (TextView)convertView.findViewById(R.id.prog_time);
                view.short_desc = (TextView)convertView.findViewById(R.id.prog_short_desc);
            }
            convertView.setTag(view);
        }
        TvProgramme prog = programmes.get(position);
        if (position == 0 && crossesSixAM(prog)) {
            view.time.setBackgroundResource(timeColorMorning);
            view.time.setText("  6:00\n (cont)");
        } else {
            Calendar timeval = prog.getStart();
            int hour = timeval.get(Calendar.HOUR_OF_DAY);
            if (hour < 6)
                view.time.setBackgroundResource(timeColorLateNight);
            else if (hour < 12)
                view.time.setBackgroundResource(timeColorMorning);
            else if (hour < 18)
                view.time.setBackgroundResource(timeColorAfternoon);
            else
                view.time.setBackgroundResource(timeColorNight);
            view.time.setText(Utils.formatTimeProgrammeList(timeval));
        }
        view.short_desc.setText(prog.getShortDescription(context, getDisplayMatch(position)));
        if (isExpanded)
            view.long_desc.setText(prog.getLongDescription(context));
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

    private static int getStartTime(TvProgramme prog, int position) {
        int hour = prog.getStart().get(Calendar.HOUR_OF_DAY);
        int minute = prog.getStart().get(Calendar.MINUTE);
        int second = prog.getStart().get(Calendar.SECOND);
        if (hour < 6 && position != 0)
            hour += 24;
        return hour * 60 * 60 + minute * 60 + second;
    }

    /**
     * Gets the start time of the programme at a specific position in the list.
     * 
     * @param position the position of the programme
     * @return the start time, seconds since midnight (>= 24 hours for "next day")
     */
    public int getTimeForPosition(int position) {
        if (position < 0 || position >= programmes.size())
            return 0;
        return getStartTime(programmes.get(position), position);
    }
    
    /**
     * Gets the position within the list that contains a specific start time.
     * 
     * @param time the start time, seconds since midnight (>= 24 hours for "next day")
     * @return the position
     */
    public int getPositionForTime(int time) {
        for (int index = 0; index < programmes.size(); ++index) {
            int t = getStartTime(programmes.get(index), index);
            if (t > time)
                return Math.max(0,  index - 1);
        }
        return Math.max(0, programmes.size() - 1);
    }
}
