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

import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

public class TvChannelListAdapter implements ListAdapter, SpinnerAdapter, TvChannelChangedListener {

    private List<TvChannel> channels;
    private List<DataSetObserver> observers;
    private LayoutInflater inflater;
    
    public TvChannelListAdapter(Context context) {
        this.channels = new ArrayList<TvChannel>();
        this.channels.addAll(TvChannelCache.getInstance().getChannels());
        this.observers = new ArrayList<DataSetObserver>();
        this.inflater = LayoutInflater.from(context);
    }

    public void attach() {
        TvChannelCache.getInstance().addChannelChangedListener(this);
    }
    
    public void detach() {
        TvChannelCache.getInstance().removeChannelChangedListener(this);
    }
    
    public void addAnyChannel() {
        TvChannel channel = new TvChannel();
        channel.setName("Any channel");
        channel.setId("");
        channels.add(0, channel);
    }
    
    public int addOtherChannel(String channelId) {
        TvChannel channel = new TvChannel();
        channel.setName(channelId);
        channel.setId(channelId);
        channels.add(channel);
        return channels.size() - 1;
    }

    public int getCount() {
        return channels.size();
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

    public TvChannel getChannel(int position) {
        return channels.get(position);
    }
    
    public int getPositionForChannel(String channelId) {
        if (channelId == null)
            channelId = "";
        for (int index = 0; index < channels.size(); ++index) {
            if (channels.get(index).getId().equals(channelId))
                return index;
        }
        return -1;
    }
    
    private class ViewDetails {
        public ImageView icon;
        public TextView name;
        public TextView numbers;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewDetails view = null;
        if (convertView != null)
            view = (ViewDetails)convertView.getTag();
        if (view == null) {
            convertView = inflater.inflate(R.layout.channel, null);
            view = new ViewDetails();
            view.icon = (ImageView)convertView.findViewById(R.id.channel_icon);
            view.name = (TextView)convertView.findViewById(R.id.channel_name);
            view.numbers = (TextView)convertView.findViewById(R.id.channel_numbers);
            convertView.setTag(view);
        }
        TvChannel channel = channels.get(position);
        view.icon.setImageResource(channel.getIconResource());
        view.name.setText(channel.getName());
        String numbers = channel.getNumbers();
        if (numbers == null)
        	numbers = "";
        view.numbers.setText(numbers);
        return convertView;
    }

    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        return getView(position, convertView, parent);
    }

    public int getViewTypeCount() {
        return 1;
    }

    public boolean hasStableIds() {
        return false;
    }

    public boolean isEmpty() {
        return channels.isEmpty();
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

    public boolean isEnabled(int position) {
        return true;
    }

    public void channelsChanged() {
        channels.clear();
        this.channels.addAll(TvChannelCache.getInstance().getChannels());
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }
}
