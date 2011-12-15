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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.util.Xml;

/**
 * This class manages the global list of bookmarks and ticks in the TV guide.
 */
public class TvBookmarkManager {

    private static TvBookmarkManager instance = null;
    private List<TvBookmark> bookmarks;
    private List<TvTick> ticks;
    private List<Context> contexts;
    private BookmarkMediaHandler mediaHandler;
    private boolean isLoaded;
    private List<TvBookmarkChangedListener> listeners;
    
    private class BookmarkMediaHandler extends ExternalMediaHandler {

        protected BookmarkMediaHandler(Context context) {
            super(context);
        }

        @Override
        protected void mediaUsableChanged() {
            mediaChanged();
        }
    }

    private TvBookmarkManager() {
        bookmarks = new ArrayList<TvBookmark>();
        ticks = new ArrayList<TvTick>();
        contexts = new ArrayList<Context>();
        mediaHandler = null;
        isLoaded = false;
        listeners = new ArrayList<TvBookmarkChangedListener>();
    }

    /**
     * Retrieves the global bookmark manager instance.
     * 
     * @return the bookmark manager
     */
    public static TvBookmarkManager getInstance() {
        if (instance == null)
            instance = new TvBookmarkManager();
        return instance;
    }

    /**
     * Adds a context to this manager, for saving bookmarks when they change.
     * If this is the first context that was added, the bookmarks will be loaded.
     * 
     * @param context the context to add
     */
    public void addContext(Context context) {
        for (int index = 0; index < contexts.size(); ++index) {
            if (contexts.get(index) == context)
                return;
        }
        contexts.add(context);
        if (mediaHandler == null) {
            mediaHandler = new BookmarkMediaHandler(context);
            mediaHandler.registerReceivers();
        }
        if (!isLoaded && mediaHandler.isMediaUsable()) {
            isLoaded = true;
            load();
        }
    }

    /**
     * Removes a context from this manager.
     * 
     * @param context the context to remove
     */
    public void removeContext(Context context) {
        if (mediaHandler != null && mediaHandler.getContext() == context) {
            mediaHandler.unregisterReceivers();
            mediaHandler = null;
        }
        for (int index = 0; index < contexts.size(); ++index) {
            if (contexts.get(index) == context) {
                contexts.remove(index);
                break;
            }
        }
        if (mediaHandler == null && contexts.size() > 0) {
            context = contexts.get(0);
            mediaHandler = new BookmarkMediaHandler(context);
            mediaHandler.registerReceivers();
        }
        if (mediaHandler == null) {
            // No more contexts, so mark the data to be reloaded the next time
            // we have a context just in case the SD card contents was changed.
            isLoaded = false;
        }
    }

    /**
     * Gets the list of bookmarks that are active on this manager.
     * 
     * @return the bookmark list
     */
    public List<TvBookmark> getBookmarks() {
        return bookmarks;
    }
    
    /**
     * Adds a new bookmark to the list.
     * 
     * @param bookmark the bookmark to add
     */
    public void addBookmark(TvBookmark bookmark) {
        bookmarks.add(bookmark);
        save();
    }

    /**
     * Removes a bookmark from the list.  The item is searched for based on
     * strict object identity.
     * 
     * @param bookmark the bookmark to remove
     */
    public void removeBookmark(TvBookmark bookmark) {
        for (int index = 0; index < bookmarks.size(); ++index) {
            if (bookmarks.get(index) == bookmark) {
                bookmarks.remove(index);
                save();
                break;
            }
        }
    }

    /**
     * Notify the manager that a specific bookmark in the list has been updated.
     * 
     * @param bookmark the bookmark
     */
    public void updateBookmark(TvBookmark bookmark) {
        save();
    }
    
    /**
     * Finds a bookmark by its internal identifier.
     * 
     * @param id the bookmark's internal identifier
     * @return the bookmark object, or null if not found
     */
    public TvBookmark findBookmarkById(long id) {
        for (int index = 0; index < bookmarks.size(); ++index) {
            TvBookmark bookmark = bookmarks.get(index);
            if (bookmark.getInternalId() == id)
                return bookmark;
        }
        return null;
    }
    
    /**
     * Gets the list of ticks that are active on this manager.
     * 
     * @return the tick list
     */
    public List<TvTick> getTicks() {
        return ticks;
    }
    
    /**
     * Adds a tick object to the tick list.
     * 
     * @param tick the tick object to add
     */
    public void addTick(TvTick tick) {
        ticks.add(tick);
        save();
    }
    
    /**
     * Removes a tick object from the tick list.  The object is searched for
     * using equals() rather than strict object identity.
     * 
     * @param tick the tick object to remove
     */
    public void removeTick(TvTick tick) {
        for (int index = 0; index < ticks.size(); ++index) {
            if (ticks.get(index).equals(tick)) {
                ticks.remove(index);
                save();
                break;
            }
        }
    }

    private File getBookmarksFile() {
        return new File(mediaHandler.getFilesDir(), "bookmarks.xml");
    }

    /**
     * Loads the bookmarks from external SD card storage.
     */
    private void load() {
        bookmarks.clear();
        ticks.clear();
        File file = getBookmarksFile();
        if (!file.exists())
            return;
        Calendar today = new GregorianCalendar();
        try {
            FileInputStream fileStream = new FileInputStream(file);
            try {
                XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
                XmlPullParser parser = factory.newPullParser();
                parser.setInput(fileStream, null);
                int eventType = parser.getEventType();
                while (eventType != XmlPullParser.END_DOCUMENT) {
                    if (eventType == XmlPullParser.START_TAG &&
                            parser.getName().equals("bookmark")) {
                        // Parse the contents of a <bookmark> element.
                        TvBookmark bookmark = new TvBookmark();
                        bookmark.loadFromXml(parser);
                        bookmarks.add(bookmark);
                        eventType = parser.getEventType();
                    } else if (eventType == XmlPullParser.START_TAG &&
                                parser.getName().equals("tick")) {
                        // Parse the contents of a <tick> element.
                        TvTick tick = new TvTick();
                        tick.loadFromXml(parser);
                        long diff = (today.getTimeInMillis() - tick.getTimestamp().getTimeInMillis());
                        if (diff < 30 * 24 * 60 * 60 * 1000) // ticks expire after 30 days
                            ticks.add(tick);
                        eventType = parser.getEventType();
                    } else {
                        // Skip unknown element.
                        eventType = parser.next();
                    }
                }
            } catch (XmlPullParserException e) {
                // Ignore - just stop parsing at the first error.
            } catch (IOException e) {
            }
        } catch (IOException e) {
        }
    }

    /**
     * Saves the bookmarks to external SD card storage.
     */
    private void save() {
        if (mediaHandler == null || !mediaHandler.isMediaUsable())
            return;
        File file = getBookmarksFile();
        try {
            FileOutputStream fileStream = new FileOutputStream(file);
            XmlSerializer serializer = Xml.newSerializer();
            serializer.setOutput(fileStream, "UTF-8");
            serializer.startDocument(null, Boolean.valueOf(true));
            serializer.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output", true);
            serializer.startTag(null, "bookmarks");
            for (TvBookmark bookmark: bookmarks)
                bookmark.saveToXml(serializer);
            for (TvTick tick: ticks)
                tick.saveToXml(serializer);
            serializer.endTag(null, "bookmarks");
            serializer.endDocument();
            fileStream.close();
        } catch (IOException e) {
        }
        notifyChanged();
    }

    /**
     * Reload the bookmarks if the SD card media has been re-inserted,
     * or invalidate the bookmarks if the SD card media has been removed.
     */
    private void mediaChanged() {
        if (mediaHandler.isMediaUsable()) {
            isLoaded = true;
            load();
        } else {
            isLoaded = false;
            bookmarks.clear();
            ticks.clear();
        }
        notifyChanged();
    }
    
    private void notifyChanged() {
        for (TvBookmarkChangedListener listener: listeners)
            listener.bookmarksChanged();
    }

    public void addChangedListener(TvBookmarkChangedListener listener) {
        listeners.add(listener);
    }
    
    public void removeChangedListener(TvBookmarkChangedListener listener) {
        listeners.remove(listener);
    }
    
    /**
     * Match a programme against the bookmarks and ticks in this manager.
     * 
     * @param prog the programme
     */
    public void matchProgramme(TvProgramme prog) {
        TvBookmarkMatch result = TvBookmarkMatch.NoMatch;
        TvBookmark bookmark = null;
        
        // Check the list of ticked programmes first as ticking takes
        // precedence over bookmark matching.
        // TODO: tick indexing
        for (TvTick tick: ticks) {
            if (tick.match(prog)) {
                prog.setBookmark(null, TvBookmarkMatch.TickMatch);
                return;
            }
        }
        
        // Look for a bookmark match.
        // TODO: bookmark indexing
        List<TvBookmark> candidates = bookmarks;
        for (TvBookmark bm: candidates) {
            TvBookmarkMatch match = bm.match(prog);
            if (match != TvBookmarkMatch.NoMatch) {
                if (match == TvBookmarkMatch.ShouldMatch) {
                    if (result != TvBookmarkMatch.TitleMatch) {
                        bookmark = bm;
                        result = TvBookmarkMatch.ShouldMatch;
                    }
                } else if (match != TvBookmarkMatch.TitleMatch) {
                    bookmark = bm;
                    result = match;
                    break;
                } else {
                    bookmark = bm;
                    result = TvBookmarkMatch.TitleMatch;
                }
            }
        }

        // Update the programme's bookmark details.
        prog.setBookmark(bookmark, result);
    }
    
    /**
     * Match all programmes in a list against the bookmarks and ticks in this manager.
     * 
     * @param programmes the list of programmes
     */
    public void matchProgrammes(List<TvProgramme> programmes) {
        for (TvProgramme prog: programmes)
            matchProgramme(prog);
    }
}
