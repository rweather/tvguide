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

import java.util.Calendar;
import java.util.GregorianCalendar;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

public class TVGuideActivity extends Activity {

    private ListView channelListView;
    private TvChannelListAdapter channelListAdapter;
    private TvRegionListAdapter regionListAdapter;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.channel_list);

        channelListView = (ListView)findViewById(R.id.channelList);
        channelListAdapter = new TvChannelListAdapter(this);
        
        regionListAdapter = new TvRegionListAdapter(this);

        channelListView.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                if (parent.getAdapter() instanceof TvChannelListAdapter) {
                    TvChannel channel = channelListAdapter.getChannel(position);
                    selectChannel(channel);
                } else if (parent.getAdapter() instanceof TvRegionListAdapter) {
                    String regionId = regionListAdapter.getRegionId(position);
                    selectRegion(regionId);
                }
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();

        SharedPreferences prefs = getPreferences(0);
        String region = prefs.getString("region", "");
        if (region != null && !region.equals(""))
            channelListAdapter.setRegion(region);
        
        // If no region selected yet, then populate the initial channel list with regions.
        if (channelListAdapter.getCount() == 0)
            channelListView.setAdapter(regionListAdapter);
        else
            channelListView.setAdapter(channelListAdapter);
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    /**
     * Select a specific channel and display the programmes for today.
     * 
     * @param channel the channel that was selected
     */
    private void selectChannel(TvChannel channel) {
        Calendar date = new GregorianCalendar();
        Intent intent = new Intent(this, TvProgrammeListActivity.class);
        intent.putExtra("date_year", date.get(Calendar.YEAR));
        intent.putExtra("date_month", date.get(Calendar.MONTH));
        intent.putExtra("date_day", date.get(Calendar.DAY_OF_MONTH));
        intent.putExtra("channel", channel.toBundle());
        startActivity(intent);
    }
    
    /**
     * Select a specific region and switch to the channels for that region.
     * 
     * @param regionId the region identifier
     */
    private void selectRegion(String regionId) {
        channelListAdapter.setRegion(regionId);
        channelListView.setAdapter(channelListAdapter);

        SharedPreferences.Editor editor = getPreferences(0).edit();
        editor.putString("region", regionId);
        editor.commit();
    }
    
    private static final int ITEM_CHANGE_REGION = 1;
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(ITEM_CHANGE_REGION, 0, 0, "Change Region");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getGroupId() == ITEM_CHANGE_REGION) {
            channelListView.setAdapter(regionListAdapter);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
