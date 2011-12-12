package com.southernstorm.tvguide;

public enum TvBookmarkMatch {
    NoMatch,
    FullMatch,
    Overrun,        // Runs over the stop time
    Underrun,       // Starts before the start time
    TitleMatch,     // Matches only on the title
    ShouldMatch,    // Should match but doesn't (NonMatching option)
    TickMatch       // Matches against the tick list.
}
