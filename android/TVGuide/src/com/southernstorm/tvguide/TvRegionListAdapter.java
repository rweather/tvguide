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
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListAdapter;
import android.widget.TextView;

public class TvRegionListAdapter implements ListAdapter {

    private class Region implements Comparable<Region> {
        public String id;
        public String name;
        public List<Region> parents;
        public boolean selectable;
        
        public int compareTo(Region other) {
            return name.compareToIgnoreCase(other.name);
        }
    }

    private Context context;
    private Map<String, Region> regions;
    private List<Region> selectableRegions;
    
    public TvRegionListAdapter(Context context) {
        this.context = context;
        loadRegions(context);
    }
    
    private void loadRegions(Context context) {
        XmlResourceParser parser = context.getResources().getXml(R.xml.channels);
        regions = new TreeMap<String, Region>();
        Region currentRegion = null;
        String id;
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
                        Region parentRegion = (parent != null ? regions.get(parent) : null);
                        currentRegion = new Region();
                        currentRegion.id = id;
                        currentRegion.name = id;
                        currentRegion.parents = new ArrayList<Region>();
                        if (parentRegion != null)
                            currentRegion.parents.add(parentRegion);
                        currentRegion.selectable = false;
                        regions.put(id, currentRegion);
                    } else if (name.equals("display-name") && currentRegion != null) {
                        // Human-readable name of the region.
                        currentRegion.name = Utils.getContents(parser, name);
                    } else if (name.equals("selectable") && currentRegion != null) {
                        // This region can be selected in the UI.
                        currentRegion.selectable = true;
                    } else if (name.equals("other-parent") && currentRegion != null) {
                        // Secondary parent for the current region.
                        parent = Utils.getContents(parser, name);
                        Region parentRegion = regions.get(parent);
                        currentRegion.parents.add(parentRegion);
                    }
                } else if (eventType == XmlPullParser.END_TAG && parser.getName().equals("region")) {
                    currentRegion = null;
                }
                eventType = parser.next();
            }
        } catch (XmlPullParserException e) {
            // Ignore - just stop parsing at the first error.
        } catch (IOException e) {
        }
        parser.close();
        selectableRegions = new ArrayList<Region>();
        for (Region region: regions.values()) {
            if (region.selectable)
                selectableRegions.add(region);
        }
        Collections.sort(selectableRegions);
    }

    public int getCount() {
        return selectableRegions.size();
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
        if (convertView instanceof TextView)
            view = (TextView)convertView;
        else
            view = new TextView(context);
        view.setText(selectableRegions.get(position).name);
        view.setTextColor(0xFF000000);
        view.setLines(2);
        view.setGravity(0x10);  // center_vertical
        return view;
    }

    public int getViewTypeCount() {
        return 1;
    }

    public boolean hasStableIds() {
        return true;
    }

    public boolean isEmpty() {
        return selectableRegions.isEmpty();
    }

    public void registerDataSetObserver(DataSetObserver observer) {
        // List is never updated after construction, so no need to notify anyone.
    }

    public void unregisterDataSetObserver(DataSetObserver observer) {
    }

    public boolean areAllItemsEnabled() {
        return true;
    }

    public boolean isEnabled(int position) {
        return true;
    }
    
    public String getRegionId(int position) {
        return selectableRegions.get(position).id;
    }
}
