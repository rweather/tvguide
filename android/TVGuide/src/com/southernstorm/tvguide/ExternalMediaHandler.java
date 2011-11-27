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

    private Context context;
    private boolean mounted;
    private boolean readOnly;
    private boolean useInternal;
    private BroadcastReceiver receiver;

    /**
     * Constructs a new external media handler for a context.
     *
     * @param context the application's main context
     */
    protected ExternalMediaHandler(Context context) {
        this.context = context;
        checkMountState();
    }

    public void registerReceivers() {
        // Listen for events indicating that the SD card has been
        // mounted or unmounted.
        IntentFilter filter = new IntentFilter(Intent.ACTION_MEDIA_MOUNTED);
        filter.addDataScheme("file");
        if (receiver == null) {
            receiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    boolean oldMounted = mounted;
                    boolean oldReadOnly = readOnly;
                    boolean oldUseInternal = useInternal;
                    checkMountState();
                    if (oldMounted != mounted || oldReadOnly != readOnly ||
                            oldUseInternal != useInternal)
                        mediaUsableChanged();
                }
            };
        }
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

    public void unregisterReceivers() {
        context.unregisterReceiver(receiver);
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
     * Returns the context associated with this external media handler.
     *
     * @return the context
     */
    public Context getContext() {
        return context;
    }

    /**
     * Determine if the external media (SD card usually) is currently
     * mounted and usable.
     *
     * @return true if the media is mounted and writable, false otherwise.
     */
    public boolean isMediaUsable() {
        return mounted && !readOnly;
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
            return context.getCacheDir();
        else
            return context.getExternalCacheDir();
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
            return context.getFilesDir();
        else
            return context.getExternalFilesDir(null);
    }
}
