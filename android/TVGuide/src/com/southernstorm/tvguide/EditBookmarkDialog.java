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

import android.app.Dialog;
import android.content.Context;
import android.content.SharedPreferences;

import android.view.View;
import android.view.ViewGroup.LayoutParams;

import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TimePicker;

public class EditBookmarkDialog extends Dialog {

    private EditText title;
    private Spinner channel;
    private TvChannelListAdapter channelAdapter;
    private Spinner dayOfWeek;
    private EditBookmarkDayOfWeekAdapter dayOfWeekAdapter;
    private Spinner color;
    private EditBookmarkColorAdapter colorAdapter;
    private TimePicker startTime;
    private TimePicker stopTime;
    private CheckBox anyTime;
    private EditText season;
    private EditText year;
    private RadioButton onAir;
    private RadioButton offAir;
    private Button saveButton;
    private Button cancelButton;
    private boolean cancelSet;
    
    public EditBookmarkDialog(Context context) {
        super(context);
        cancelSet = false;
        
        setContentView(R.layout.edit_bookmark_dialog);
        setTitle(context.getResources().getString(R.string.edit_bookmark_window_title));
        getWindow().setLayout(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
        
        title = (EditText)findViewById(R.id.edit_bookmark_title);
        channel = (Spinner)findViewById(R.id.edit_bookmark_channel);
        dayOfWeek = (Spinner)findViewById(R.id.edit_bookmark_day);
        color = (Spinner)findViewById(R.id.edit_bookmark_color);
        startTime = (TimePicker)findViewById(R.id.edit_bookmark_start_time);
        stopTime = (TimePicker)findViewById(R.id.edit_bookmark_stop_time);
        anyTime = (CheckBox)findViewById(R.id.edit_bookmark_any_time);
        season = (EditText)findViewById(R.id.edit_bookmark_season);
        year = (EditText)findViewById(R.id.edit_bookmark_year);
        onAir = (RadioButton)findViewById(R.id.edit_bookmark_on_air);
        offAir = (RadioButton)findViewById(R.id.edit_bookmark_off_air);
        saveButton = (Button)findViewById(R.id.edit_bookmark_save);
        cancelButton = (Button)findViewById(R.id.edit_bookmark_cancel);
        
        channelAdapter = new TvChannelListAdapter(context);
        SharedPreferences prefs = context.getSharedPreferences("TVGuideActivity", 0);
        String region = prefs.getString("region", "");
        if (region != null && !region.equals(""))
            channelAdapter.setRegion(region);
        channelAdapter.addAnyChannel();
        channel.setAdapter(channelAdapter);
        
        dayOfWeekAdapter = new EditBookmarkDayOfWeekAdapter(context);
        dayOfWeek.setAdapter(dayOfWeekAdapter);
        
        colorAdapter = new EditBookmarkColorAdapter(context);
        color.setAdapter(colorAdapter);
        
        anyTime.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                startTime.setEnabled(!isChecked);
                stopTime.setEnabled(!isChecked);
            }
        });
        
        saveButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                dismiss();
            }
        });

        cancelButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                setCancel(true);
                cancel();
            }
        });
    }
    
    public void copyFromBookmark(TvBookmark bookmark) {
        title.setText(bookmark.getTitle());
        int position = channelAdapter.getPositionForChannel(bookmark.getChannelId());
        if (position < 0)
            position = channelAdapter.addOtherChannel(bookmark.getChannelId());
        channel.setSelection(position);
        dayOfWeek.setSelection(dayOfWeekAdapter.getPositionForMask(bookmark.getDayOfWeekMask()));
        color.setSelection(colorAdapter.getPositionForColor(bookmark.getColor()));
        int time = bookmark.getStartTime();
        startTime.setCurrentHour(time / (60 * 60));
        startTime.setCurrentMinute((time / 60) % 60);
        time = bookmark.getStopTime();
        stopTime.setCurrentHour(time / (60 * 60));
        stopTime.setCurrentMinute((time / 60) % 60);
        boolean any = bookmark.getAnyTime();
        anyTime.setChecked(any);
        startTime.setEnabled(!any);
        stopTime.setEnabled(!any);
        season.setText(bookmark.getSeasons());
        year.setText(bookmark.getYears());
        if (bookmark.isOnAir())
            onAir.setChecked(true);
        else
            offAir.setChecked(true);
        cancelSet = false;
    }
    
    public void setCancel(boolean flag) {
        cancelSet = flag;
    }
    
    public void copyToBookmark(TvBookmark bookmark) {
        if (cancelSet)
            return;
        bookmark.setTitle(title.getText().toString());
        TvChannel channel = channelAdapter.getChannel(this.channel.getSelectedItemPosition());
        if (channel == null || channel.getId().equals(""))
            bookmark.setChannelId(null);
        else
            bookmark.setChannelId(channel.getId());
        bookmark.setDayOfWeekMask(dayOfWeekAdapter.getMaskForPosition(dayOfWeek.getSelectedItemPosition()));
        bookmark.setColor(colorAdapter.getColorForPosition(color.getSelectedItemPosition()));
        bookmark.setStartTime(startTime.getCurrentHour() * 60 * 60 + startTime.getCurrentMinute() * 60);
        bookmark.setStopTime(stopTime.getCurrentHour() * 60 * 60 + stopTime.getCurrentMinute() * 60);
        bookmark.setAnyTime(anyTime.isChecked());
        bookmark.setSeasons(season.getText().toString());
        bookmark.setYears(year.getText().toString());
        bookmark.setOnAir(onAir.isChecked());
    }
}
