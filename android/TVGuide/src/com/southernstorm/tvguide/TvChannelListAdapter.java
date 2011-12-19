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
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.content.Context;
import android.content.res.XmlResourceParser;
import android.database.DataSetObserver;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

public class TvChannelListAdapter implements ListAdapter, SpinnerAdapter {

    private Context context;
    private List<TvChannel> channels;
    private List<DataSetObserver> observers;
    private LayoutInflater inflater;
    private String region;
    private Map< String, List<String> > regionTree;
    private Map< String, ArrayList<String> > commonIds;
    
    public TvChannelListAdapter(Context context) {
        this.context = context;
        this.channels = new ArrayList<TvChannel>();
        this.observers = new ArrayList<DataSetObserver>();
        this.inflater = LayoutInflater.from(context);
        this.region = null;
    }

    public String getRegion() {
        return region;
    }
    
    public void setRegion(String region) {
        if (this.region == null || !this.region.equals(region)) {
            this.region = region;
            loadChannels();
            for (DataSetObserver observer: observers)
                observer.onChanged();
        }
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

    /**
     * Loads channel information from the channels.xml file embedded in the resources.
     */
    private void loadChannels() {
        channels.clear();
        if (region == null)
            return;
        XmlResourceParser parser = context.getResources().getXml(R.xml.channels);
        regionTree = new TreeMap< String, List<String> >();
        commonIds = new TreeMap< String, ArrayList<String> >();
        String id = null;
        String parent;
        try {
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG) {
                    String name = parser.getName();
                    if (name.equals("region")) {
                        // Parse the contents of a <region> element.
                        id = parser.getAttributeValue(null, "id");
                        parent = parser.getAttributeValue(null, "parent");
                        if (id != null && parent != null) {
                            if (!regionTree.containsKey(id))
                                regionTree.put(id, new ArrayList<String>());
                            regionTree.get(id).add(parent);
                        }
                    } else if (name.equals("other-parent")) {
                        // Secondary parent for the current region.
                        parent = Utils.getContents(parser, name);
                        if (!regionTree.containsKey(id))
                            regionTree.put(id, new ArrayList<String>());
                        regionTree.get(id).add(parent);
                    } else if (name.equals("channel")) {
                        // Parse the contents of a <channel> element.
                        TvChannel channel = loadChannel(parser);
                        if (channel != null)
                            channels.add(channel);
                    }
                }
                eventType = parser.next();
            }
        } catch (XmlPullParserException e) {
            // Ignore - just stop parsing at the first error.
        } catch (IOException e) {
        }
        parser.close();
        Collections.sort(channels);
        regionTree = null;
        commonIds = null;
    }
    
    private TvChannel loadChannel(XmlPullParser parser) throws XmlPullParserException, IOException {
        TvChannel channel = new TvChannel();
        channel.setId(parser.getAttributeValue(null, "id"));
        String commonId = parser.getAttributeValue(null, "common-id");
        channel.setCommonId(commonId);
        if (commonId != null) {
            // Keep track of all channels with the same common identifier in a shared list.
            // We use this to migrate bookmarks across regions.
            ArrayList<String> list = commonIds.get(commonId);
            if (list != null) {
                list.add(channel.getId());
            } else {
                list = new ArrayList<String>();
                list.add(channel.getId());
                commonIds.put(commonId, list);
            }
            channel.setOtherChannelsList(list);
        }
        String region = parser.getAttributeValue(null, "region");
        if (region == null || !regionMatch(region))
            return null;
        int eventType = parser.next();
        while (eventType != XmlPullParser.END_DOCUMENT) {
            if (eventType == XmlPullParser.START_TAG) {
                String name = parser.getName();
                if (name.equals("display-name")) {
                    channel.setName(Utils.getContents(parser, name));
                } else if (name.equals("icon")) {
                    String src = parser.getAttributeValue(null, "src");
                    if (src != null) {
                        int index = src.lastIndexOf('/');
                        if (index >= 0)
                            channel.setIconResource(IconFactory.getInstance().getChannelIconResource(src.substring(index + 1)));
                    }
                } else if (name.equals("number")) {
                    String system = parser.getAttributeValue(null, "system");
                    String currentNumbers = channel.getNumbers();
                    if (!system.equals("digital")) {
                        if (currentNumbers == null)
                            return null;    // Ignore Pay TV only channels for now.
                    } else {
                        String number = Utils.getContents(parser, name);
                        if (currentNumbers == null) {
                            channel.setNumbers(number);
                            channel.setPrimaryChannelNumber(Integer.valueOf(number));
                        } else {
                            channel.setNumbers(currentNumbers + ", " + number);
                        }
                    }
                }
            } else if (eventType == XmlPullParser.END_TAG && parser.getName().equals("channel")) {
                break;
            }
            eventType = parser.next();
        }
        List<String> baseUrls = new ArrayList<String>();
        baseUrls.add("http://www.oztivo.net/xmltv/");
        baseUrls.add("http://xml.oztivo.net/xmltv/");
        channel.setBaseUrls(baseUrls);
        return channel;
    }
    
    private boolean regionMatch(String r) {
        if (r.equals(region))
            return true;
        List<String> testRegions = new ArrayList<String>();
        testRegions.add(region);
        return regionMatch(r, testRegions);
    }
    
    private boolean regionMatch(String r, List<String> regions) {
        for (String region: regions) {
            if (r.equals(region))
                return true;
            List<String> testRegions = regionTree.get(region);
            if (testRegions != null && regionMatch(r, testRegions))
                return true;
        }
        return false;
    }
}
