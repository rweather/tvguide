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
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.format.DateFormat;
import android.widget.ExpandableListView;

public class TVGuideActivity extends Activity implements TvNetworkListener {

    private ExpandableListView programmeListView;
    private TvProgrammeListAdapter programmeListAdapter;
    private TvChannelCache channelCache;
    private ProgressDialog progressDialog;
    private TvChannel channel;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        channelCache = new TvChannelCache(this, this);
        channelCache.setServiceName("OzTivo");
        channelCache.setDebug(true);
        channelCache.expire();

        programmeListView = (ExpandableListView)findViewById(R.id.programmeList);
        programmeListAdapter = new TvProgrammeListAdapter(this);
        programmeListView.setAdapter(programmeListAdapter);

        channel = new TvChannel();
        //channel.setId("ABC-Qld");
        //channel.setName("ABC Queensland");
        channel.setId("Sci-Fi");
        channel.setName("Sci Fi");
        List<String> baseUrls = new ArrayList<String>();
        baseUrls.add("http://www.oztivo.net/xmltv/");
        baseUrls.add("http://xml.oztivo.net/xmltv/");
        channel.setBaseUrls(baseUrls);
        
        Calendar date = new GregorianCalendar();
        Calendar tomorrow = (Calendar)date.clone(); 
        tomorrow.add(Calendar.DAY_OF_MONTH, 1);
        List<Calendar> datesCovered = new ArrayList<Calendar>();
        datesCovered.add(date);
        datesCovered.add(tomorrow);
        
        programmeListAdapter.setChannel(channel);
        programmeListAdapter.setDatesCovered(datesCovered);
        
        fetch(channel, date, date);
        fetch(channel, tomorrow, date);
    }

    @Override
    protected void onStart() {
        super.onStart();
        channelCache.registerReceivers();
    }

    @Override
    protected void onStop() {
        channelCache.unregisterReceivers();
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
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

    public void setCurrentNetworkRequest(TvChannel channel, Calendar date, Calendar primaryDate) {
        String message = channel.getName() + " " + DateFormat.format("E, MMM dd", primaryDate);
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
        if (programmeListAdapter.isChannelCovered(channel) && programmeListAdapter.isDateCovered(date)) {
            programmeListAdapter.setProgrammes
                (channel.programmesForDay(programmeListAdapter.getPrimaryDate()));
        }
    }

    public void requestFailed(TvChannel channel, Calendar date, Calendar primaryDate) {
        // TODO
    }
}
