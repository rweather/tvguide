## Change log

### 0.0.7

* Add Wikipedia to the list of search engines for Web Search.
* Season number matching in bookmarks.
* Remove redundant failed matches before and after a successful match.
* Combine multiple showings of the same episode into a single
  entry in the 7-day outlook.  The extra showings are displayed
  as "Other showings".
* Temporary "tick" bookmarks for marking one-off shows such as movies.
* Matching bookmarks on "Any time".  Makes it easier to watch for
  future shows with "Any channel, Any day, and Any time".
* Option on the "Tools" menu to clear the disk cache.
* Load channel icons.
* Handle the XMLTV "presenter" tag.

### 0.0.6

* Fix a web search problem related to non-alphanumeric characters
  like colons and exclaimation marks in show names.
* Add a drop-down list of actors, directors, and the episode title
  when searching for a show in the "Web Search" dialog.
* 7 Day Outlook and All Channels state is saved on exit.
* Save the previous search engine from the "Web Search" dialog.

### 0.0.5

* Unit tests for bookmark class, to assist with refactoring.
* Add "Saturday and Sunday" as a valid bookmark matching option.
* "Other" button added to bookmark dialog to select arbitrary
  combinations of days for bookmark matching.
* Double-clicking a bookmark in "Organize Bookmarks" will Edit it.
* Display the first column of the 7 day view as "Monday July 25"
  rather than just "Monday".  Also suppress repeated displays
  of the same date to delineate the days better.
* Imparja no longer appears in capital cities, only regional areas.
* Add "Help" buttons to the dialogs with context-sensitive help.
* Sort "One" to appear after "Ten".

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
