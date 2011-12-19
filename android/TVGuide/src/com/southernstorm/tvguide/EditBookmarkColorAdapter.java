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

public class EditBookmarkColorAdapter implements ListAdapter, SpinnerAdapter {

    private LayoutInflater inflater;
    private List<Integer> colorValues;
    private List<Integer> colorResources;
    private List<String> colorNames;
    
    public EditBookmarkColorAdapter(Context context) {
        this.inflater = LayoutInflater.from(context);
        
        colorValues = new ArrayList<Integer>();
        colorResources = new ArrayList<Integer>();
        colorNames = new ArrayList<String>();
        
        colorValues.add(0xFFFF0000);
        colorResources.add(R.drawable.color_red);
        colorNames.add("Red");
        
        colorValues.add(0xFF00AA00);
        colorResources.add(R.drawable.color_green);
        colorNames.add("Green");
        
        colorValues.add(0xFF0000FF);
        colorResources.add(R.drawable.color_blue);
        colorNames.add("Blue");
        
        colorValues.add(0xFFFFAA00);
        colorResources.add(R.drawable.color_orange);
        colorNames.add("Orange");
        
        colorValues.add(0xFFFF007F);
        colorResources.add(R.drawable.color_purple);
        colorNames.add("Purple");
    }

    private static long colorDistance(int c1, int c2) {
        long distance;
        int diff = ((c1 >> 16) & 0xFF) - ((c2 >> 16) & 0xFF);
        distance = diff * diff;
        diff = ((c1 >> 8) & 0xFF) - ((c2 >> 8) & 0xFF);
        distance += diff * diff;
        diff = (c1 & 0xFF) - (c2 & 0xFF);
        distance += diff * diff;
        return distance;
    }

    public int getPositionForColor(int color) {
        long distance = colorDistance(color, colorValues.get(0));
        int match = 0;
        for (int index = 1; index < colorValues.size(); ++index) {
            long newdist = colorDistance(color, colorValues.get(index));
            if (newdist < distance) {
                distance = newdist;
                match = index;
            }
        }
        return match;
    }

    public int getColorForPosition(int position) {
        return colorValues.get(position);
    }

    public int getCount() {
        return colorValues.size();
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

    private class ViewDetails {
        public ImageView icon;
        public TextView name;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewDetails view = null;
        if (convertView != null)
            view = (ViewDetails)convertView.getTag();
        if (view == null) {
            convertView = inflater.inflate(R.layout.color, null);
            view = new ViewDetails();
            view.icon = (ImageView)convertView.findViewById(R.id.color_icon);
            view.name = (TextView)convertView.findViewById(R.id.color_name);
            convertView.setTag(view);
        }
        view.icon.setImageResource(colorResources.get(position));
        view.name.setText(colorNames.get(position));
        return convertView;
    }

    public int getViewTypeCount() {
        return 1;
    }

    public boolean hasStableIds() {
        return false;
    }

    public boolean isEmpty() {
        return colorValues.isEmpty();
    }

    public void registerDataSetObserver(DataSetObserver observer) {
    }

    public void unregisterDataSetObserver(DataSetObserver observer) {
    }

    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        return getView(position, convertView, parent);
    }

    public boolean areAllItemsEnabled() {
        return true;
    }

    public boolean isEnabled(int position) {
        return true;
    }
}
