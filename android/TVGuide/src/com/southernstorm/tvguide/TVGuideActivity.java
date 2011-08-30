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

import android.app.Activity;
import android.os.Bundle;
import android.content.SharedPreferences;
import android.widget.ExpandableListView;

public class TVGuideActivity extends Activity {

    private ExpandableListView programmeListView;
    private TvProgrammeListAdapter programmeListAdapter;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

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
            progs.add(prog);
        }

        programmeListAdapter.setProgrammes(progs);
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
}
