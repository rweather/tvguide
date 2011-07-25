
XMLTV Guide Browser
===================

This application provides support for directly browsing online
XMLTV guides such as OzTivo and tv.swedb.se:

    http://www.oztivo.net/
    http://tv.swedb.se/content/view/55/48/

## Obtaining the sources

The sources are available from the project
[git repository](https://github.com/rweather/tvguide).

## License

The application is distributed under the terms of the
GNU General Public License, version 3.  A copy of the
license can be found in the COPYING file within the sources.

## Dependencies

It is recommended to use Qt 4.7 or later.  It may build with
Qt 4.6, but this has not been tested.  It definitely will not
work with Qt 4.5 and earlier due to missing features in Qt that
the application relies upon.

## Building

Once you have checked out the sources from the repository,
you can build it from the command-line as follows:

    $ qmake tvguide.pro
    $ make

Qt Creator can also be used to build the application by
opening the tvguide.pro file.

## Staying up to date

The [Southern Storm](http://southern-storm.blogspot.com/) blog
is the main place to find announcements about progress.
Or contact the author via e-mail (use "git log" to find the
e-mail address).

## Change log

### 0.0.4

* Region selection on the "Edit Channels" dialog for OzTivo data.
  Note: Pay TV channels will not be affected by region changes,
  so use "Hide All" to hide everything before selecting a region.
* Checkbox to enable/disable individual bookmarks.  If a bookmark
  is unchecked then it will be ignored for matching.  Useful for a
  show that is known not to be on at the moment, and the user does
  not care about partial matches.
* Zoom in/out on View menu for adjusting the font size in the
  programme view (laptop connected to TV use case).
* Move bookmark operations to a new "Bookmarks" menu.
* "Show partial matches" option on the "Bookmarks" menu to control
  whether partial title-only matches are shown or not.  On by default.
* "Show failed matches" option on the "Bookmarks" menu that causes
  failed bookmark matches to show with a strike-through of the
  bookmark's title.  Implements the "show me when a show is no
  longer on or it has moved" use case.  Off by default.
* Some performance improvements to parsing of XML guide data.

### 0.0.3

* Pressing "Add Bookmark" when a bookmarked program is selected
  will prompt the user to ask if they wish to edit the existing
  bookmark or add a new one.
* Enable the "wrapping" option on time edit widgets.
* Multi-channel view mode which shows all programmes on all
  channels that start at a particular time.  Combine with
  7 Day Outlook to see all bookmark matches for all channels
  and all days.
* Show channel numbers next to the names and sort the list
  on ascending order of channel number, with some grouping
  of ABC, SBS, etc channels; e.g. 2, 20, 21, ..., 3, 30, 31, ...

### 0.0.2

* Fix caching of channel index - it was attempting to refresh
  the data from the server too often.
* Prepend "MOVIE:" to the title of movie programmes.
* Add episode numbers to the summary description.
* If a show overruns or underruns its bookmarked timeslot, then
  use a lighter shade for the highlight color; i.e. reset your
  recorder because the show isn't in its usual timeslot.
* If a bookmark matches on title only, but not channel, day, or
  time, then use a darker shade of the highlight color.  This
  indicates where the show has been moved to another channel/day
  or repeat episodes.
* Fix bookmark sort order to default to Day/Ascending, not
  Day/Descending.
* Searching for a show by name on google, imdb, or epguides.
* Fix flickering of Reload/Stop buttons in long-running
  netwok requests; especially for the 7 Day Outlook.
* The busy indicator and progress bar were not being displayed
  when the channel list was first loaded.  Fix this.
* Make the status bar show the channel/date that is being fetched
  whenever a network request is performed.

### 0.0.1

* Initial version.
* Basic channel/day selection and programme browsing.
* Bookmarking for favourite shows.
