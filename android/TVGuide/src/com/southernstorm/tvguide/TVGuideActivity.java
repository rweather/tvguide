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
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

public class TVGuideActivity extends Activity implements TvNetworkListener {

    private ListView channelListView;
    private TvChannelListAdapter channelListAdapter;
    private TvRegionListAdapter regionListAdapter;
    private ProgressDialog progressDialog;

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
        channelListAdapter.attach();
        TvChannelCache.getInstance().addNetworkListener(this);
        TvChannelCache.getInstance().addContext(this);

        // If no region selected yet, then populate the initial channel list with regions.
        if (channelListAdapter.getCount() == 0)
            channelListView.setAdapter(regionListAdapter);
        else
            channelListView.setAdapter(channelListAdapter);
    }

    @Override
    protected void onStop() {
        TvChannelCache.getInstance().removeContext(this);
        TvChannelCache.getInstance().removeNetworkListener(this);
        channelListAdapter.detach();
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
        }
        super.onStop();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
        }
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
        TvChannelCache.getInstance().setRegion(regionId);
        channelListView.setAdapter(channelListAdapter);

        SharedPreferences.Editor editor = getPreferences(0).edit();
        editor.putString("region", regionId);
        editor.commit();
    }
    
    private static final int ITEM_CHANGE_REGION = 1;
    private static final int ITEM_ORGANIZE_BOOKMARKS = 2;
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(ITEM_CHANGE_REGION, 0, 0, "Change Region");
        menu.add(ITEM_ORGANIZE_BOOKMARKS, 0, 0, "Organize Bookmarks");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getGroupId() == ITEM_CHANGE_REGION) {
            channelListView.setAdapter(regionListAdapter);
            return true;
        } else if (item.getGroupId() == ITEM_ORGANIZE_BOOKMARKS) {
            Intent intent = new Intent(this, TvBookmarkListActivity.class);
            startActivity(intent);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void setCurrentNetworkRequest(TvChannel channel, Calendar date, Calendar primaryDate) {
    }

    public void setCurrentNetworkIconRequest(TvChannel channel) {
        System.out.println("icon fetch for: " + channel.getName());
        String message = channel.getName();
        if (progressDialog == null) {
            progressDialog = ProgressDialog.show
                (this, "Fetching channel icon", message, true);
        } else {
            progressDialog.setMessage(message);
            progressDialog.show();
        }
    }
    
    public void endNetworkRequests() {
        if (progressDialog != null)
            progressDialog.hide();
    }

    public void dataAvailable(TvChannel channel, Calendar date, Calendar primaryDate) {
    }

    public void requestFailed(TvChannel channel, Calendar date, Calendar primaryDate) {
    }
}
