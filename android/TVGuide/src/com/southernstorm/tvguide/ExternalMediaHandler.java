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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Environment;

/**
 * External media handler for the SD card - keeps track of when the SD card is
 * mounted and unmounted so the application knows when it can be used.
 *
 * If there is no SD card in the device, then internal storage is used instead.
 */
public abstract class ExternalMediaHandler {

    private class ContextInfo {
        public Context context;
        public BroadcastReceiver receiver;
    }
    
    private List<ContextInfo> contexts;
    private boolean mounted;
    private boolean readOnly;
    private boolean useInternal;
    private boolean firstCheck;

    /**
     * Constructs a new external media handler.  Should be followed by a call
     * to addContext() to associate the handler with an activity.
     */
    protected ExternalMediaHandler() {
        this.contexts = new ArrayList<ContextInfo>();
        this.firstCheck = true;
    }

    /**
     * Adds an activity's context to this media handler.
     * 
     * @param context the context to add
     */
    public void addContext(Context context) {
        for (int index = 0; index < contexts.size(); ++index) {
            if (contexts.get(index).context == context)
                return;
        }
        ContextInfo info = new ContextInfo();
        info.context = context;
        contexts.add(info);
        registerReceivers(info);
        if (contexts.size() == 1)
            updateMountState();
    }
    
    /**
     * Removes an activity's context from this media handler.
     * 
     * @param context the context to add
     */
    public void removeContext(Context context) {
        for (int index = 0; index < contexts.size(); ++index) {
            ContextInfo info = contexts.get(index);
            if (info.context == context) {
                contexts.remove(index);
                unregisterReceivers(info);
                break;
            }
        }
    }
    
    private void registerReceivers(ContextInfo info) {
        // Listen for events indicating that the SD card has been
        // mounted or unmounted.
        IntentFilter filter = new IntentFilter(Intent.ACTION_MEDIA_MOUNTED);
        filter.addDataScheme("file");
        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                updateMountState();
            }
        };
        info.receiver = receiver;
        Context context = info.context;
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_BAD_REMOVAL);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_CHECKING);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_EJECT);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_NOFS);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_REMOVED);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_SHARED);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_UNMOUNTABLE);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
        filter = new IntentFilter(Intent.ACTION_MEDIA_UNMOUNTED);
        filter.addDataScheme("file");
        context.registerReceiver(receiver, filter);
    }

    private void unregisterReceivers(ContextInfo info) {
        info.context.unregisterReceiver(info.receiver);
    }

    private void updateMountState() {
        boolean oldMounted = mounted;
        boolean oldReadOnly = readOnly;
        boolean oldUseInternal = useInternal;
        checkMountState();
        if (firstCheck || oldMounted != mounted || oldReadOnly != readOnly ||
                oldUseInternal != useInternal)
            mediaUsableChanged();
        firstCheck = false;
    }

    private void checkMountState() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            mounted = true;
            readOnly = false;
            useInternal = false;
        } else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            mounted = true;
            readOnly = true;
            useInternal = false;
        } else if (Environment.MEDIA_REMOVED.equals(state)) {
            // No SD card at all, so act as though it is mounted
            // but use the internal storage instead.
            mounted = true;
            readOnly = false;
            useInternal = true;
        } else {
            mounted = false;
            readOnly = true;
            useInternal = false;
        }
    }

    /**
     * Returns the first activity context associated with this external
     * media handler, or null if no activities are using this handler at present.
     *
     * @return the context
     */
    public Context getContext() {
        if (contexts.size() > 0)
            return contexts.get(0).context;
        else
            return null;
    }

    /**
     * Determine if the external media (SD card usually) is currently
     * mounted and usable.
     *
     * @return true if the media is mounted and writable, false otherwise.
     */
    public boolean isMediaUsable() {
        return !contexts.isEmpty() && mounted && !readOnly;
    }

    /**
     * Called when isMediaUsable() changes state.
     */
    protected abstract void mediaUsableChanged();

    /**
     * Gets the File object corresponding to the root directory of the cache
     * for this application.
     *
     * @return the root of the cache, or null if the media is unusable
     */
    public File getCacheDir() {
        if (!isMediaUsable())
            return null;
        else if (useInternal)
            return getContext().getCacheDir();
        else
            return getContext().getExternalCacheDir();
    }

    /**
     * Gets the File object corresponding to the root directory of the
     * persistent application data.
     *
     * @return the root of the file space, or null if the media is unusable
     */
    public File getFilesDir() {
        if (!isMediaUsable())
            return null;
        else if (useInternal)
            return getContext().getFilesDir();
        else
            return getContext().getExternalFilesDir(null);
    }
}
