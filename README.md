
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
    $ for filepath in $(ls src/*.ui);
      do
        uic "$filepath" > "src/ui_$(basename -s .ui "$filepath").h";
      done
    $ make

Qt Creator can also be used to build the application by
opening the tvguide.pro file.

## Staying up to date

The [Southern Storm](http://southern-storm.blogspot.com/) blog
is the main place to find announcements about progress.
Or contact the author via e-mail (use "git log" to find the
e-mail address).
