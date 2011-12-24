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
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.TreeMap;
import java.util.zip.GZIPInputStream;

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.cookie.DateParseException;
import org.apache.http.impl.cookie.DateUtils;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.XmlResourceParser;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;

/**
 * Cache management and network fetching for channel data.
 */
public class TvChannelCache extends ExternalMediaHandler {

    private String serviceName;
    private File httpCacheDir;
    private File iconCacheDir;
    private Random rand;
    private boolean debug;
    private List<TvChannel> channels;
    private String region;
    private Map< String, List<String> > regionTree;
    private Map< String, ArrayList<String> > commonIds;
    private List<TvNetworkListener> networkListeners;
    private List<TvChannelChangedListener> channelListeners;
    private static TvChannelCache instance = null;

    private TvChannelCache() {
        this.serviceName = "";
        this.rand = new Random(System.currentTimeMillis());
        this.channels = new ArrayList<TvChannel>();
        this.networkListeners = new ArrayList<TvNetworkListener>();
        this.channelListeners = new ArrayList<TvChannelChangedListener>();
    }

    void setDebug(boolean value) {
        debug = value;
    }

    /**
     * Gets the global instance of the channel cache.
     * 
     * @return the global instance
     */
    public static TvChannelCache getInstance() {
        if (instance == null) {
            instance = new TvChannelCache();
            instance.setServiceName("OzTivo");
            instance.setDebug(true);    // FIXME: remove before releasing
        }
        return instance;
    }
    
    /**
     * Gets the current region.
     * 
     * @return the region, or null if none set
     */
    public String getRegion() {
        return region;
    }

    /**
     * Sets the current region.
     * 
     * @param region the region to set
     */
    public void setRegion(String region) {
        if (this.region == null || !this.region.equals(region)) {
            this.region = region;
            
            // Save the region in the settings.
            SharedPreferences.Editor editor = getContext().getSharedPreferences("TVGuideActivity", 0).edit();
            editor.putString("region", region);
            editor.commit();

            // Reload the channel list.
            loadChannels();
        }
    }

    @Override
    public void addContext(Context context) {
        super.addContext(context);
        if (region == null) {
            // Load the region from the settings for the first time.
            SharedPreferences prefs = context.getSharedPreferences("TVGuideActivity", 0);
            region = prefs.getString("region", "");
            if (region != null && region.equals(""))
                region = null;
        }
        if (channels.size() == 0 && region != null)
            loadChannels();
    }

    /**
     * Gets the list of active channels.
     * 
     * @return the channels
     */
    public List<TvChannel> getChannels() {
        return channels;
    }
    
    /**
     * Gets the channel with a specific identifier.
     * 
     * @param id the identifier
     * @return the channel, or null if not found
     */
    public TvChannel getChannel(String id) {
        if (id == null || id.length() == 0)
            return null;
        for (int index = 0; index < channels.size(); ++index) {
            TvChannel channel = channels.get(index);
            if (channel.getId().equals(id))
                return channel;
        }
        return null;
    }
    
    /**
     * Gets the name of the service to cache channel data underneath.
     *
     * @return the service name, or the empty string if no service
     */
    public String getServiceName() {
        return serviceName;
    }

    /**
     * Sets the name of the service to cache channel data underneath.
     *
     * @param serviceName the service name, or the empty string if no service
     */
    public void setServiceName(String serviceName) {
        if (serviceName == null)
            serviceName = "";
        if (!this.serviceName.equals(serviceName)) {
            this.serviceName = serviceName;
            if (isMediaUsable())
                reloadService();
        }
    }

    /**
     * Open the XMLTV data file in the cache for a specific channel and date.
     *
     * The data on the SD card is stored as gzip'ed XML.  The stream returned
     * by this function will unzip the data as it is read.
     *
     * @param channel the channel
     * @param date the date corresponding to the requested data
     * @return an input stream, or null if the data is not present
     */
    public InputStream openChannelData(TvChannel channel, Calendar date) {
        File file = dataFile(channel, date, ".xml.gz");
        if (file == null || !file.exists())
            return null;
        try {
            FileInputStream fileStream = new FileInputStream(file);
            try {
                return new GZIPInputStream(fileStream);
            } catch (IOException e) {
                fileStream.close();
                return null;
            }
        } catch (IOException e) {
            return null;
        }
    }

    /**
     * Gets the last-modified date for a specific channel and date combination.
     * 
     * @param channel the channel
     * @param date the date
     * @return the last-modified date, or null if the data is not in the cache
     */
    public Calendar channelDataLastModified(TvChannel channel, Calendar date) {
        File file = dataFile(channel, date, ".cache");
        if (file == null || !file.exists())
            return null;
        try {
            FileInputStream input = new FileInputStream(file);
            try {
                InputStreamReader reader = new InputStreamReader(input, "utf-8");
                try {
                    String line;
                    while ((line = readLine(reader)) != null) {
                        if (line.startsWith("Last-Modified: ")) {
                            String lastModified = line.substring(15);
                            Date parsedDate = DateUtils.parseDate(lastModified);
                            GregorianCalendar result = new GregorianCalendar();
                            result.setTime(parsedDate);
                            return result;
                        }
                    }
                } catch (DateParseException e) {
                } finally {
                    reader.close();
                }
            } finally {
                input.close();
            }
        } catch (IOException e) {
        }
        return null;
    }
    
    /**
     * Determine if data for a specific channel and date is available in the cache
     * 
     * @param channel the channel
     * @param date the date to look for
     * @return true if data is available, false if not
     */
    public boolean hasChannelData(TvChannel channel, Calendar date) {
        File file = dataFile(channel, date, ".xml.gz");
        if (file == null || !file.exists())
            return false;
        else
            return true;
    }
    
    /**
     * Expire old entries in the cache.
     */
    public void expire() {
        if (httpCacheDir == null)
            return;
        String[] entries = httpCacheDir.list();
        GregorianCalendar today = new GregorianCalendar();
        int todayYear = today.get(Calendar.YEAR);
        int todayMonth = today.get(Calendar.MONTH) + 1;
        int todayDay = today.get(Calendar.DAY_OF_MONTH);
        for (int index = 0; index < entries.length; ++index) {
            // Look for files that end in ".xml.gz" or ".cache".
            String name = entries[index];
            int suffixLength;
            if (name.endsWith(".xml.gz"))
                suffixLength = 7;
            else if (name.endsWith(".cache"))
                suffixLength = 6;
            else
                continue;
            if ((name.length() - suffixLength) < 10)
                continue;

            // Extract the date in the format YYYY-MM-DD from the name
            // and determine if it is less than today.
            int posn = name.length() - suffixLength - 10;
            int year = Utils.parseField(name, posn, 4);
            int month = Utils.parseField(name, posn + 5, 2);
            int day = Utils.parseField(name, posn + 8, 2);
            if (year > todayYear)
                continue;
            if (year == todayYear) {
                if (month > todayMonth)
                    continue;
                if (month == todayMonth) {
                    if (day >= todayDay)
                        continue;
                }
            }

            // Delete the file as it is older than today.
            File file = new File(httpCacheDir, name);
            if (debug)
                System.out.println("expiring " + file.getPath());
            file.delete();
        }
    }
    
    /**
     * Clear the entire contents of the cache.
     */
    public void clear() {
        if (httpCacheDir == null)
            return;
        String[] entries = httpCacheDir.list();
        for (int index = 0; index < entries.length; ++index) {
            // Look for files that end in ".xml.gz" or ".cache".
            String name = entries[index];
            int suffixLength;
            if (name.endsWith(".xml.gz"))
                suffixLength = 7;
            else if (name.endsWith(".cache"))
                suffixLength = 6;
            else
                continue;
            if ((name.length() - suffixLength) < 10)
                continue;

            // Delete the file.
            File file = new File(httpCacheDir, name);
            if (debug)
                System.out.println("deleting " + file.getPath());
            file.delete();
        }
    }

    /**
     * Reloads the service in response to a service name or media change.
     */
    private void reloadService() {
        // If the service name is empty, then there is no need for a cache.
        if (serviceName.length() == 0) {
            unloadService();
            return;
        }

        // Create the cache directory if it doesn't already exist.
        File cacheDir = getCacheDir();
        if (cacheDir == null) {
            unloadService();
            return;
        }
        File serviceCacheDir = new File(cacheDir, serviceName);
        httpCacheDir = new File(serviceCacheDir, "http");
        httpCacheDir.mkdirs();
        if (!httpCacheDir.exists()) {
            unloadService();
            return;
        }
        iconCacheDir = new File(serviceCacheDir, "icons");
        iconCacheDir.mkdirs();
        if (!iconCacheDir.exists())
            iconCacheDir = null;    // We have the http directory, so we can continue.
    }

    /**
     * Unloads the service in response to the media being unmounted, usually
     * because the SD card has been mounted via USB by another computer.
     */
    private void unloadService() {
        httpCacheDir = null;
        iconCacheDir = null;
    }

    @Override
    protected void mediaUsableChanged() {
        if (isMediaUsable())
            reloadService();
        else
            unloadService();
    }

    /**
     * Get the name of the data file corresponding to a particular
     * channel and date.
     *
     * @param channel the channel
     * @param date the date to fetch
     * @param extension the file extension, ".xml.gz" or ".cache"
     * @return the filename encapsulated in a File object, or null if no cache
     */
    private File dataFile(TvChannel channel, Calendar date, String extension) {
        if (httpCacheDir == null)
            return null;
        StringBuilder name = new StringBuilder();
        int year = date.get(Calendar.YEAR);
        int month = date.get(Calendar.MONTH) + 1;
        int day = date.get(Calendar.DAY_OF_MONTH);
        name.append(channel.getId());
        name.append('_');
        name.append((char)('0' + ((year / 1000) % 10)));
        name.append((char)('0' + ((year / 100) % 10)));
        name.append((char)('0' + ((year / 10) % 10)));
        name.append((char)('0' + (year % 10)));
        name.append('-');
        name.append((char)('0' + ((month / 10) % 10)));
        name.append((char)('0' + (month % 10)));
        name.append('-');
        name.append((char)('0' + ((day / 10) % 10)));
        name.append((char)('0' + (day % 10)));
        name.append(extension);
        return new File(httpCacheDir, name.toString());
    }

    private static String readLine(InputStreamReader reader) throws IOException {
        StringBuilder builder = new StringBuilder(1024);
        int ch;
        while ((ch = reader.read()) != -1 && ch != '\n')
            builder.append((char)ch);
        if (ch == -1 && builder.length() == 0)
            return null;
        return builder.toString();
    }
    
    private class RequestInfo {
        public TvChannel channel;
        public Calendar date;
        public Calendar primaryDate;
        public URI uri;
        public File cacheFile;
        public File dataFile;
        public String etag;
        public String lastModified;
        public String userAgent;
        public boolean success;
        public boolean notFound;
        public RequestInfo next;

        public void updateFromResponse(HttpResponse response) {
            Header header = response.getFirstHeader("ETag");
            if (header != null)
                etag = header.getValue();
            header = response.getFirstHeader("Last-Modified");
            if (header != null)
                lastModified = header.getValue();
        }
    };

    private RequestInfo requestQueue;
    private TvChannel currentRequestChannel;
    private Calendar currentRequestDate;
    private Calendar currentRequestPrimaryDate;
    private boolean requestsActive;

    private static Calendar lastRequestTime = null;

    /**
     * Background asynchronous task for downloading data from the Internet.
     */
    private class DownloadAsyncTask extends AsyncTask<RequestInfo, Integer, RequestInfo> {
        private boolean fetch(RequestInfo info) {
            try {
                // OzTivo requires that there be at least 1 second between data requests.
                // Icons are fetched from elsewhere so we can fetch them immediately.
                if (info.date != null) {
                    Calendar currentTime = new GregorianCalendar();
                    if (lastRequestTime == null) {
                        lastRequestTime = currentTime;
                    } else {
                        long diff = currentTime.getTimeInMillis() - lastRequestTime.getTimeInMillis();
                        if (diff >= 0 && diff < 1000) {
                            try {
                                Thread.sleep(diff);
                            } catch (InterruptedException e) {
                            }
                        }
                        lastRequestTime = currentTime;
                    }
                }

                // Start the request.
                DefaultHttpClient client = new DefaultHttpClient();
                HttpGet request = new HttpGet(info.uri);
                request.setHeader("User-Agent", info.userAgent);
                if (info.etag != null)
                    request.setHeader("If-None-Match", info.etag);
                if (info.lastModified != null)
                    request.setHeader("If-Modified-Since", info.lastModified);
                request.setHeader("Accept-Encoding", "gzip");
                HttpResponse response = client.execute(request);
                int status = response.getStatusLine().getStatusCode();
                if (status == HttpStatus.SC_OK) {
                    // Successful response with new data.  Copy it to the cache.
                    info.updateFromResponse(response);
                    HttpEntity entity = response.getEntity();
                    InputStream content = entity.getContent();
                    try {
                        FileOutputStream output = new FileOutputStream(info.dataFile);
                        byte[] buffer = new byte [2048];
                        try {
                            int length;
                            while ((length = content.read(buffer, 0, 2048)) > 0)
                                output.write(buffer, 0, length);
                        } finally {
                            output.close();
                        }
                    } catch (IOException e) {
                        return false;
                    } finally {
                        content.close();
                    }
                    return true;
                } else if (status == HttpStatus.SC_NOT_MODIFIED) {
                    // Data has not changed since the last request.
                    info.updateFromResponse(response);
                    return true;
                } else if (status == HttpStatus.SC_NOT_FOUND) {
                    // Explicit 404 Not Found from the server.
                    info.notFound = true;
                    return false;
                } else {
                    // Request failed for some other reason.
                    return false;
                }
            } catch (UnsupportedEncodingException e) {
                return false;
            } catch (MalformedURLException e) {
                return false;
            } catch (IOException e) {
                return false;
            }
        }

        protected RequestInfo doInBackground(RequestInfo... requests) {
            RequestInfo info = requests[0];
            if (info.cacheFile != null && info.cacheFile.exists()) {
                // Read ETag/Last-Modified data from the ".cache" file.
                try {
                    FileInputStream input = new FileInputStream(info.cacheFile);
                    try {
                        InputStreamReader reader = new InputStreamReader(input, "utf-8");
                        try {
                            String line;
                            while ((line = readLine(reader)) != null) {
                                if (line.startsWith("ETag: "))
                                    info.etag = line.substring(6);
                                else if (line.startsWith("Last-Modified: "))
                                    info.lastModified = line.substring(15);
                            }
                        } finally {
                            reader.close();
                        }
                    } finally {
                        input.close();
                    }
                } catch (IOException e) {
                }
            }
            info.success = fetch(info);
            if (!info.success) {
                // Something failed during the request - delete the cache files
                // before handing the result back to the main thread.
                if (info.cacheFile != null)
                    info.cacheFile.delete();
                info.dataFile.delete();
            } else if (info.cacheFile != null) {
                // Write ETag/Last-Modified data to the ".cache" file.
                try {
                    FileOutputStream output = new FileOutputStream(info.cacheFile);
                    try {
                        OutputStreamWriter writer = new OutputStreamWriter(output, "utf-8");
                        try {
                            if (info.etag != null) { 
                                writer.write("ETag: ");
                                writer.write(info.etag);
                                writer.write("\n");
                            }
                            if (info.lastModified != null) {
                                writer.write("Last-Modified: ");
                                writer.write(info.lastModified);
                                writer.write("\n");
                            }
                        } finally {
                            writer.close();
                        }
                    } finally {
                        output.close();
                    }
                } catch (IOException e) {
                }
            }
            return info;
        }

        protected void onProgressUpdate(Integer... progress) {
            // Progress reporting not used by this task.
        }

        protected void onPostExecute(RequestInfo info) {
            if (debug) {
                if (info.success)
                    System.out.println("fetched to " + info.dataFile.getPath());
                else
                    System.out.println("fetch failed");
            }
            reportRequestResult(info);
            startNextRequest();
        }
    };

    /**
     * Fetch bulk data for all channels.
     * 
     * @param numDays the number of days to fetch
     * @return true if data has been scheduled to be fetched, false if nothing needs downloading
     */
    public boolean bulkFetch(int numDays) {
        boolean fetched = false;
        Calendar today = new GregorianCalendar();
        for (int index = 0; index < channels.size(); ++index) {
            TvChannel channel = channels.get(index);
            for (int day = 0; day < numDays; ++day) {
                Calendar date = (Calendar)today.clone();
                date.add(Calendar.DAY_OF_MONTH, day);
                if (!hasChannelData(channel, date)) {
                    fetch(channel, date, today);
                    fetched = true;
                }
            }
        }
        return fetched;
    }
    
    /**
     * Fetches the guide data for a specific date and time.
     * 
     * @param channel the channel
     * @param date the date to request
     */
    public void fetch(TvChannel channel, Calendar date) {
        fetch(channel, date, date);
    }

    /**
     * Fetches the guide data for a specific date and time, as part of a multi-day request.
     * At least two days worth of data are needed to show 6:00am one day to 6:00am the next.
     * The first day is the "primary" and typically must be fetched from the server.
     * The second day's data is optional and an error will not be reported to the user
     * if it is not available.
     * 
     * @param channel the channel
     * @param date the date to request
     * @param primaryDate the primary date for multi-day requests
     */
    public void fetch(TvChannel channel, Calendar date, Calendar primaryDate) {
        // Bail out if the cache is unusable or there is no network.
        if (httpCacheDir == null || !isNetworkingAvailable())
            return;

        // Determine the base URL to use.  OzTivo rules specify that a
        // url should be chosen randomly from the list of base urls.
        // http://www.oztivo.net/twiki/bin/view/TVGuide/StaticXMLGuideAPI
        List<String> baseUrls = channel.getBaseUrls();
        if (baseUrls.isEmpty())
            return;
        String baseUrl;
        if (baseUrls.size() >= 2)
            baseUrl = baseUrls.get(rand.nextInt(baseUrls.size()));
        else
            baseUrl = baseUrls.get(0);

        // Generate the URI for the request.
        StringBuilder requestUrl = new StringBuilder();
        int year = date.get(Calendar.YEAR);
        int month = date.get(Calendar.MONTH) + 1;
        int day = date.get(Calendar.DAY_OF_MONTH);
        requestUrl.append(baseUrl);
        if (!baseUrl.endsWith("/"))
            requestUrl.append('/');
        requestUrl.append(channel.getId());
        requestUrl.append('_');
        requestUrl.append((char)('0' + ((year / 1000) % 10)));
        requestUrl.append((char)('0' + ((year / 100) % 10)));
        requestUrl.append((char)('0' + ((year / 10) % 10)));
        requestUrl.append((char)('0' + (year % 10)));
        requestUrl.append('-');
        requestUrl.append((char)('0' + ((month / 10) % 10)));
        requestUrl.append((char)('0' + (month % 10)));
        requestUrl.append('-');
        requestUrl.append((char)('0' + ((day / 10) % 10)));
        requestUrl.append((char)('0' + (day % 10)));
        requestUrl.append(".xml.gz");
        URI uri;
        try {
            uri = new URI(requestUrl.toString());
        } catch (URISyntaxException e) {
            return;
        }

        // Create the request info block.
        RequestInfo info = new RequestInfo();
        info.channel  = channel;
        info.date = date;
        info.primaryDate = primaryDate;
        info.uri = uri;
        info.cacheFile = dataFile(channel, date, ".cache");
        info.dataFile = dataFile(channel, date, ".xml.gz");
        info.etag = null;
        info.lastModified = null;
        info.userAgent = getContext().getResources().getString(R.string.user_agent);
        info.success = false;

        // Queue up the request to fetch the data from the network.
        addRequestToQueue(info);
    }

    /**
     * Fetches a channel icon from the network.
     * 
     * @param channel the channel
     * @param uri the URI of the icon's location on the network
     * @param file the local file to cache the icon data in
     */
    private void fetchIcon(TvChannel channel, String uri, File file) {
        // Bail out if the cache is unusable or there is no network.
        if (iconCacheDir == null || !isNetworkingAvailable())
            return;

        // Parse the URI.
        URI uriObject;
        try {
            uriObject = new URI(uri);
        } catch (URISyntaxException e) {
            return;
        }

        // Create the request info block.  Null date indicates that this is an icon fetch.
        RequestInfo info = new RequestInfo();
        info.channel  = channel;
        info.date = null;
        info.primaryDate = null;
        info.uri = uriObject;
        info.cacheFile = null;
        info.dataFile = file;
        info.etag = null;
        info.lastModified = null;
        info.userAgent = getContext().getResources().getString(R.string.user_agent);
        info.success = false;

        // Queue up the request to fetch the data from the network.
        addRequestToQueue(info);
    }

    /**
     * Adds a request to the queue of files to be downloaded.
     *
     * @param info the request to be added
     */
    private void addRequestToQueue(RequestInfo info) {
        // Ignore the request if it is already on the queue.
        RequestInfo current = requestQueue;
        RequestInfo prev = null;
        while (current != null) {
            if (current.channel == info.channel && current.date != null &&
                    info.date != null && current.date.equals(info.date)) {
                // Upgrade the existing request to a primary day request if necessary.
                if (info.primaryDate.equals(info.date))
                    current.primaryDate = current.date;
                return;
            }
            prev = current;
            current = current.next;
        }
        if (currentRequestChannel == info.channel && info.date != null &&
                currentRequestDate != null && currentRequestDate.equals(info.date)) {
            if (info.primaryDate.equals(info.date))
                currentRequestPrimaryDate = currentRequestDate;
            return;
        }

        // Add the request to the end of the queue.
        info.next = null;
        if (prev != null)
            prev.next = info;
        else
            requestQueue = info;

        // If we don't have a request currently in progress, then start it.
        if (currentRequestChannel == null)
            startNextRequest();
    }

    /**
     * Start downloading the next request on the queue.
     */
    private void startNextRequest() {
        for (;;) {
            RequestInfo info = requestQueue;
            if (info == null) {
                currentRequestChannel = null;
                currentRequestDate = null;
                currentRequestPrimaryDate = null;
                if (requestsActive) {
                    requestsActive = false;
                    for (TvNetworkListener listener: networkListeners)
                        listener.endNetworkRequests();
                }
                break;
            }
            requestQueue = info.next;
            info.next = null;
            if (info.date != null && hasChannelData(info.channel, info.date)) {
                // A previous request on the queue already fetched this data.
                for (TvNetworkListener listener: networkListeners)
                    listener.dataAvailable(info.channel, info.date, info.primaryDate);
                continue;
            }
            currentRequestChannel = info.channel;
            currentRequestDate = info.date;
            currentRequestPrimaryDate = info.primaryDate;
            if (debug)
                System.out.println("fetching " + info.uri.toString());
            requestsActive = true;
            for (TvNetworkListener listener: networkListeners) {
                if (info.date != null)
                    listener.setCurrentNetworkRequest(info.channel, info.date, info.primaryDate);
                else
                    listener.setCurrentNetworkIconRequest(info.channel);
            }
            new DownloadAsyncTask().execute(info);
            break;
        }
    }

    private void reportRequestResult(RequestInfo info) {
        for (TvNetworkListener listener: networkListeners) {
            if (info.date == null)
                continue;
            else if (info.success)
                listener.dataAvailable(info.channel, info.date, currentRequestPrimaryDate);
            else
                listener.requestFailed(info.channel, info.date, currentRequestPrimaryDate);
        }
        if (info.date == null && info.success) {
            info.channel.setIconFile(info.dataFile.getPath());
            for (TvChannelChangedListener channelListener: channelListeners)
                channelListener.channelsChanged();
        }
    }

    public void addNetworkListener(TvNetworkListener listener) {
        networkListeners.add(listener);
    }
    
    public void removeNetworkListener(TvNetworkListener listener) {
        networkListeners.remove(listener);
    }

    public void addChannelChangedListener(TvChannelChangedListener listener) {
        channelListeners.add(listener);
    }
    
    public void removeChannelChangedListener(TvChannelChangedListener listener) {
        channelListeners.remove(listener);
    }

    /**
     * Loads channel information from the channels.xml file embedded in the resources.
     */
    private void loadChannels() {
        if (region == null || getContext() == null)
            return;
        channels.clear();
        XmlResourceParser parser = getContext().getResources().getXml(R.xml.channels);
        regionTree = new TreeMap< String, List<String> >();
        commonIds = new TreeMap< String, ArrayList<String> >();
        String id = null;
        String parent;
        try {
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG) {
                    String name = parser.getName();
                    if (name.equals("region")) {
                        // Parse the contents of a <region> element.
                        id = parser.getAttributeValue(null, "id");
                        parent = parser.getAttributeValue(null, "parent");
                        if (id != null && parent != null) {
                            if (!regionTree.containsKey(id))
                                regionTree.put(id, new ArrayList<String>());
                            regionTree.get(id).add(parent);
                        }
                    } else if (name.equals("other-parent")) {
                        // Secondary parent for the current region.
                        parent = Utils.getContents(parser, name);
                        if (!regionTree.containsKey(id))
                            regionTree.put(id, new ArrayList<String>());
                        regionTree.get(id).add(parent);
                    } else if (name.equals("channel")) {
                        // Parse the contents of a <channel> element.
                        TvChannel channel = loadChannel(parser);
                        if (channel != null)
                            channels.add(channel);
                    }
                }
                eventType = parser.next();
            }
        } catch (XmlPullParserException e) {
            // Ignore - just stop parsing at the first error.
        } catch (IOException e) {
        }
        parser.close();
        Collections.sort(channels);
        regionTree = null;
        commonIds = null;
        for (TvChannelChangedListener listener: channelListeners)
            listener.channelsChanged();
    }
    
    private TvChannel loadChannel(XmlPullParser parser) throws XmlPullParserException, IOException {
        TvChannel channel = new TvChannel();
        channel.setId(parser.getAttributeValue(null, "id"));
        String commonId = parser.getAttributeValue(null, "common-id");
        channel.setCommonId(commonId);
        if (commonId != null) {
            // Keep track of all channels with the same common identifier in a shared list.
            // We use this to migrate bookmarks across regions.
            ArrayList<String> list = commonIds.get(commonId);
            if (list != null) {
                list.add(channel.getId());
            } else {
                list = new ArrayList<String>();
                list.add(channel.getId());
                commonIds.put(commonId, list);
            }
            channel.setOtherChannelsList(list);
        }
        String region = parser.getAttributeValue(null, "region");
        if (region == null || !regionMatch(region))
            return null;
        int eventType = parser.next();
        while (eventType != XmlPullParser.END_DOCUMENT) {
            if (eventType == XmlPullParser.START_TAG) {
                String name = parser.getName();
                if (name.equals("display-name")) {
                    channel.setName(Utils.getContents(parser, name));
                } else if (name.equals("icon")) {
                    String src = parser.getAttributeValue(null, "src");
                    if (src != null) {
                        int index = src.lastIndexOf('/');
                        if (index >= 0) {
                            String filename = src.substring(index + 1);
                            int resource = IconFactory.getInstance().getChannelIconResource(filename);
                            if (resource != 0) {
                                channel.setIconResource(resource);
                            } else if (iconCacheDir != null) {
                                String iconFile = iconCacheDir + "/" + filename;
                                File file = new File(iconFile);
                                if (file.exists())
                                    channel.setIconFile(iconFile);
                                else
                                    fetchIcon(channel, src, file); 
                            }
                        }
                    }
                } else if (name.equals("number")) {
                    String system = parser.getAttributeValue(null, "system");
                    String currentNumbers = channel.getNumbers();
                    if (!system.equals("digital")) {
                        if (currentNumbers == null)
                            return null;    // Ignore Pay TV only channels for now.
                    } else {
                        String number = Utils.getContents(parser, name);
                        if (currentNumbers == null) {
                            channel.setNumbers(number);
                            channel.setPrimaryChannelNumber(Integer.valueOf(number));
                        } else {
                            channel.setNumbers(currentNumbers + ", " + number);
                        }
                    }
                }
            } else if (eventType == XmlPullParser.END_TAG && parser.getName().equals("channel")) {
                break;
            }
            eventType = parser.next();
        }
        List<String> baseUrls = new ArrayList<String>();
        baseUrls.add("http://www.oztivo.net/xmltv/");
        baseUrls.add("http://xml.oztivo.net/xmltv/");
        channel.setBaseUrls(baseUrls);
        return channel;
    }
    
    private boolean regionMatch(String r) {
        if (r.equals(region))
            return true;
        List<String> testRegions = new ArrayList<String>();
        testRegions.add(region);
        return regionMatch(r, testRegions);
    }
    
    private boolean regionMatch(String r, List<String> regions) {
        for (String region: regions) {
            if (r.equals(region))
                return true;
            List<String> testRegions = regionTree.get(region);
            if (testRegions != null && regionMatch(r, testRegions))
                return true;
        }
        return false;
    }
    
    /**
     * Determine if networking is available at the present time.
     * 
     * @return true if networking is available, false if not (e.g. airplane mode).
     */
    public boolean isNetworkingAvailable() {
        Context context = getContext();
        if (context == null)
            return false;
        ConnectivityManager manager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (manager == null)
            return false;
        NetworkInfo info = manager.getActiveNetworkInfo();
        if (info == null)
            return false;
        return info.isAvailable() && info.isConnected();
    }
}
