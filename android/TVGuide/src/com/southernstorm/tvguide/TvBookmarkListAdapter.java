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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.content.Context;
import android.database.DataSetObserver;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListAdapter;
import android.widget.TextView;

public class TvBookmarkListAdapter implements ListAdapter, TvBookmarkChangedListener {

    private Context context;
    private List<DataSetObserver> observers;
    private static final List<TvBookmark> emptyBookmarks = new ArrayList<TvBookmark>();
    private List<TvBookmark> bookmarks;

    public TvBookmarkListAdapter(Context context) {
        this.context = context;
        observers = new ArrayList<DataSetObserver>();
        bookmarks = emptyBookmarks;
    }

    public boolean isNewBookmarkItem(int position) {
        return position >= 0 && position < bookmarks.size() && bookmarks.get(position) == null;
    }
    
    public TvBookmark getBookmark(int position) {
        return bookmarks.get(position);
    }

    public int getCount() {
        return bookmarks.size();
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
        if (convertView != null)
            view = (TextView)convertView;
        else
            view = new TextView(context);
        TvBookmark bookmark = bookmarks.get(position);
        if (bookmark != null) {
            view.setText(bookmark.getFormattedDescription(context));
        } else {
            RichTextFormatter formatter = new RichTextFormatter(context);
            formatter.setColor(0xFF000000);
            formatter.setBold(true);
            formatter.append("New Bookmark");
            formatter.nl();
            formatter.append(" ");
            view.setText(formatter.toSpannableString());
        }
        return view;
    }

    public int getViewTypeCount() {
        return 1;
    }

    public boolean hasStableIds() {
        return false;
    }

    public boolean isEmpty() {
        return bookmarks.isEmpty();
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

    public void bookmarksChanged() {
        bookmarks = new ArrayList<TvBookmark>();
        bookmarks.addAll(TvBookmarkManager.getInstance().getBookmarks());
        Collections.sort(bookmarks);
        bookmarks.add(0, null);     // "New Bookmark" item
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }

    public void attach() {
        TvBookmarkManager.getInstance().addChangedListener(this);
        bookmarksChanged();
    }
    
    public void detach() {
        TvBookmarkManager.getInstance().removeChangedListener(this);
        bookmarks = emptyBookmarks;
        for (DataSetObserver observer: observers)
            observer.onChanged();
    }
}
