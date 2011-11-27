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

import java.io.File;
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
import android.os.Environment;
import android.text.format.DateFormat;
import android.widget.ExpandableListView;

public class TVGuideActivity extends Activity implements TvNetworkListener {

    private ExpandableListView programmeListView;
    private TvProgrammeListAdapter programmeListAdapter;
    private TvChannelCache channelCache;
    private ProgressDialog progressDialog;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        channelCache = new TvChannelCache(this, this);
        channelCache.setServiceName("OzTivo");
        List<String> baseUrls = new ArrayList<String>();
        baseUrls.add("http://www.oztivo.net/xmltv/");
        baseUrls.add("http://xml.oztivo.net/xmltv/");
        channelCache.setBaseUrls(baseUrls);
        channelCache.setDebug(true);

        programmeListView = (ExpandableListView)findViewById(R.id.programmeList);
        programmeListAdapter = new TvProgrammeListAdapter(this);
        programmeListView.setAdapter(programmeListAdapter);

        List<TvProgramme> progs = new ArrayList<TvProgramme>();
        for (int count = 1; count <= 48; ++count) {
            TvProgramme prog = new TvProgramme();
            prog.setTitle("Title " + count);
            prog.setSubTitle("Episode 1." + count);
            prog.setDescription("This is a very long description to check that the text will wrap across lines - " + count);
            prog.setStart((count - 1 + 12) * 30);
            prog.setDate(Integer.toString(1989 + count));
            prog.setRating("PG");
            progs.add(prog);
        }

        programmeListAdapter.setProgrammes(progs);

        //channelCache.fetch("ABC Queensland", "ABC-Qld", new GregorianCalendar());
        //channelCache.fetch("ABC3", "ABC3", new GregorianCalendar());
    }

    @Override
    protected void onStart() {
        super.onStart();
        channelCache.registerReceivers();
    }

    @Override
    protected void onStop() {
        channelCache.unregisterReceivers();
        super.onStop();
    }

    /** Load preferences when the activity is resumed */
    @Override
    protected void onResume() {
        super.onResume();

        SharedPreferences prefs = getPreferences(0);
        // TODO
    }

    /** Save preferences when the activity is paused */
    @Override
    protected void onPause() {
        super.onPause();

        SharedPreferences.Editor editor = getPreferences(0).edit();
        // TODO
        editor.commit();
    }

    public void setCurrentNetworkRequest(String channelName, Calendar date) {
        String message = channelName + " " + DateFormat.format("E, MMM dd", date);
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

    public void dataAvailable(String channelId, Calendar date) {
        // TODO
    }

    public void requestFailed(String channelId, Calendar date) {
        // TODO
    }

    public void optionalRequestFailed(String channelId, Calendar date) {
        // TODO
    }
}
