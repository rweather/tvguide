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

import java.io.InputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import android.app.ProgressDialog;
import android.app.TabActivity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.text.format.DateFormat;

import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewParent;

import android.widget.ExpandableListView;
import android.widget.ExpandableListView.ExpandableListContextMenuInfo;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.TabHost.TabContentFactory;

public class TvProgrammeListActivity extends TabActivity implements TvNetworkListener {

    private static final int NUM_DAYS = 5;
    
    private View[] tabViews;
    private ExpandableListView[] programmeListViews;
    private TvProgrammeListAdapter[] programmeListAdapters;
    private TvChannelCache channelCache;
    private ProgressDialog progressDialog;
    private TvChannel channel;
    private LayoutInflater inflater;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tabbed_view);
       
        inflater = LayoutInflater.from(this);

        channelCache = new TvChannelCache(this, this);
        channelCache.setServiceName("OzTivo");
        channelCache.setDebug(true);
        channelCache.expire();
        
        tabViews = new View[NUM_DAYS];
        programmeListViews = new ExpandableListView[NUM_DAYS];
        programmeListAdapters = new TvProgrammeListAdapter[NUM_DAYS];
    }

    @Override
    protected void onStart() {
        super.onStart();
        channelCache.registerReceivers();
        
        // Unpack the channel and date from the intent.
        Intent intent = getIntent();
        int year = intent.getIntExtra("date_year", 2000);
        int month = intent.getIntExtra("date_month", Calendar.JANUARY);
        int day = intent.getIntExtra("date_day", 1);
        Calendar date = new GregorianCalendar(year, month, day);
        channel = TvChannel.fromBundle(intent.getBundleExtra("channel"));

        // Show the channel name in the title bar.
        setTitle(channel.getName());

        // Create the tabs for the next 5 days.
        TabHost tabHost = getTabHost();
        tabHost.clearAllTabs();
        for (day = 0; day < 5; ++day) {
            Calendar tabDate = (Calendar)date.clone(); 
            tabDate.add(Calendar.DAY_OF_MONTH, day);
            final int dayNum = day;
            final Calendar dayDate = tabDate;
            TabHost.TabSpec spec = tabHost.newTabSpec("day" + day)
                    .setIndicator(DateFormat.format("E\nMMM dd", tabDate))
                    .setContent(new TabContentFactory() {
                        public View createTabContent(String tab) {
                            return createTabView(dayNum, dayDate);
                        }
                    });
            tabHost.addTab(spec);
            
            // Make the tab's title use multiple lines of text.
            TextView title = (TextView)tabHost.getTabWidget().getChildAt(day).findViewById(android.R.id.title);
            title.setSingleLine(false);
            title.setGravity(0x11);     // Center the text in both directions.
        }
    }

    /**
     * Creates the programme list view for a specific tab.
     * 
     * @param day the day index for the tab
     * @param date the date on the tab
     * @return the view to display in the tab
     */
    private View createTabView(int day, Calendar date) {
        // Return the existing view if we have one.
        if (tabViews[day] != null)
            return tabViews[day];
        
        // Create a new view.
        View view = inflater.inflate(R.layout.programme_list, null);
        ExpandableListView listView = (ExpandableListView)view.findViewById(R.id.programmeList);
        TvProgrammeListAdapter adapter = new TvProgrammeListAdapter(this);
        listView.setAdapter(adapter);
        tabViews[day] = view;
        programmeListViews[day] = listView;
        programmeListAdapters[day] = adapter;
        registerForContextMenu(listView);
        
        // Fetch data from the network to fill the view.
        selectDate(day, date);
        
        // Return the tab's view.
        return view;
    }

    @Override
    protected void onStop() {
        channelCache.unregisterReceivers();
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
        }
        for (int day = 0; day < NUM_DAYS; ++day) {
            tabViews[day] = null;
            programmeListViews[day] = null;
            programmeListAdapters[day] = null;
        }
        super.onStop();
    }

    /** Load preferences when the activity is resumed */
    @Override
    protected void onResume() {
        super.onResume();

        //SharedPreferences prefs = getPreferences(0);
        // TODO
    }

    /** Save preferences when the activity is paused */
    @Override
    protected void onPause() {
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
        }

        super.onPause();

        SharedPreferences.Editor editor = getPreferences(0).edit();
        // TODO
        editor.commit();
    }

    private void selectDate(int day, Calendar date) {
        // Set up the programme list view with the covered channels and dates.
        Calendar tomorrow = (Calendar)date.clone(); 
        tomorrow.add(Calendar.DAY_OF_MONTH, 1);
        List<Calendar> datesCovered = new ArrayList<Calendar>();
        datesCovered.add(date);
        datesCovered.add(tomorrow);
        TvProgrammeListAdapter adapter = programmeListAdapters[day];
        adapter.setChannel(channel);
        adapter.setDatesCovered(datesCovered);
        
        // Fetch the selected date and the next.
        fetch(channel, date, date);
        fetch(channel, tomorrow, date);
    }

    public void setCurrentNetworkRequest(TvChannel channel, Calendar date, Calendar primaryDate) {
        String message = channel.getName(); // + " " + DateFormat.format("E, MMM dd", primaryDate);
        if (progressDialog == null) {
            progressDialog = ProgressDialog.show
                (this, "Fetching guide data", message, true);
        } else {
            progressDialog.setMessage(message);
            progressDialog.show();
        }
    }

    public void endNetworkRequests() {
        if (progressDialog != null)
            progressDialog.hide();
    }

    private void fetch(TvChannel channel, Calendar date, Calendar primaryDate) {
        InputStream stream = channelCache.openChannelData(channel, date);
        if (stream != null)
            parseProgrammes(channel, date, stream);
        else
            channelCache.fetch(channel, date, primaryDate);
    }
    
    public void dataAvailable(TvChannel channel, Calendar date, Calendar primaryDate) {
        InputStream stream = channelCache.openChannelData(channel, date);
        if (stream != null)
            parseProgrammes(channel, date, stream);
    }
    
    private void parseProgrammes(TvChannel channel, Calendar date, InputStream stream) {
        channel.loadProgrammesFromXml(date, stream);
        try {
            stream.close();
        } catch (IOException e) {
        }
        for (int day = 0; day < NUM_DAYS; ++day) {
            TvProgrammeListAdapter adapter = programmeListAdapters[day];
            if (adapter != null && adapter.isChannelCovered(channel) && adapter.isDateCovered(date))
                adapter.setProgrammes(channel.programmesForDay(adapter.getPrimaryDate()));
        }
    }

    public void requestFailed(TvChannel channel, Calendar date, Calendar primaryDate) {
        // TODO
    }

    private static final int ITEM_WEB_SEARCH = 1;
    private static final int ITEM_WEB_SEARCH_GOOGLE = 2;
    private static final int ITEM_WEB_SEARCH_IMDB = 3;
    private static final int ITEM_WEB_SEARCH_EPGUIDES = 4;
    private static final int ITEM_WEB_SEARCH_WIKIPEDIA = 5;
    
    // Workaround for the lack of expandable list item ID's on submenu items.
    private ContextMenuInfo savedMenuInfo = null;
    
    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        menu.setHeaderTitle("Programme Menu");
        SubMenu searchMenu = menu.addSubMenu("Web Search");
        searchMenu.add(ITEM_WEB_SEARCH, ITEM_WEB_SEARCH_GOOGLE, 0, "Google");
        searchMenu.add(ITEM_WEB_SEARCH, ITEM_WEB_SEARCH_IMDB, 0, "IMDb");
        searchMenu.add(ITEM_WEB_SEARCH, ITEM_WEB_SEARCH_EPGUIDES, 0, "epguides");
        searchMenu.add(ITEM_WEB_SEARCH, ITEM_WEB_SEARCH_WIKIPEDIA, 0, "Wikipedia");
        savedMenuInfo = menuInfo;
    }
    
    @Override
    public void onContextMenuClosed(Menu menu) {
        savedMenuInfo = null;
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        ExpandableListContextMenuInfo info = (ExpandableListContextMenuInfo)item.getMenuInfo();
        if (info == null)
            info = (ExpandableListContextMenuInfo)savedMenuInfo;
        int type = ExpandableListView.getPackedPositionType(info.packedPosition);
        int groupId = -1;
        if (type == ExpandableListView.PACKED_POSITION_TYPE_CHILD ||
                type == ExpandableListView.PACKED_POSITION_TYPE_GROUP) {
            groupId = ExpandableListView.getPackedPositionGroup(info.packedPosition);
        }
        if (groupId == -1)
            return false;
        TvProgramme prog = programmeForMenuItem(info, groupId);
        if (prog == null)
            return false;
        if (item.getGroupId() == ITEM_WEB_SEARCH) {
            switch (item.getItemId()) {
            case ITEM_WEB_SEARCH_GOOGLE:
                webSearch(prog, "http://www.google.com/search?q=");
                break;
            case ITEM_WEB_SEARCH_IMDB:
                webSearch(prog, "http://www.imdb.com/find?q=");
                break;
            case ITEM_WEB_SEARCH_EPGUIDES:
                webSearch(prog, "http://www.google.com/search?q=allintitle:+site:epguides.com+");
                break;
            case ITEM_WEB_SEARCH_WIKIPEDIA:
                webSearch(prog, "http://en.wikipedia.org/wiki/Special:Search?search=");
                break;
            }
        }
        return false;
    }
    
    private TvProgramme programmeForMenuItem(ExpandableListContextMenuInfo info, int groupId) {
        ViewParent view = info.targetView.getParent();
        while (view != null && !(view instanceof ExpandableListView))
            view = view.getParent();
        if (view == null)
            return null;
        TvProgrammeListAdapter adapter = (TvProgrammeListAdapter)(((ExpandableListView)view).getExpandableListAdapter());
        return adapter.getProgrammes().get(groupId);
    }

    /**
     * Search the web for a programme's title.
     * 
     * @param prog the programme details
     * @param urlPrefix prefix for the search engine to use
     */
    private void webSearch(TvProgramme prog, String urlPrefix) {
        try {
            String encoded = URLEncoder.encode(prog.getTitle(), "utf-8");
            Uri uri = Uri.parse(urlPrefix + encoded);
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            startActivity(intent);
        } catch (UnsupportedEncodingException e) {
            // Ignore - shouldn't happen.
        }
    }
}
