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

#include "tvprogramme.h"
#include "tvchannel.h"
#include "tvchannellist.h"
#include <QtCore/qdebug.h>
#include <QtGui/qtextdocument.h>

TvProgramme::TvProgramme(TvChannel *channel)
    : m_channel(channel)
    , m_season(0)
    , m_isPremiere(false)
    , m_isRepeat(false)
    , m_isMovie(false)
    , m_suppressed(false)
    , m_ticked(false)
    , m_bookmark(0)
    , m_match(TvBookmark::NoMatch)
    , m_prev(0)
    , m_next(0)
{
}

TvProgramme::~TvProgramme()
{
    if (m_bookmark)
        m_bookmark->removeProgramme(this);
}

int TvProgramme::year() const
{
    bool ok = false;
    int yr = m_date.toInt(&ok);
    if (ok && yr >= 1900)
        return yr;
    else
        return 0;
}

// Episode numbers in the "xmltv_ns" system are of the form
// A.B.C, where each of the numbers is 0-based, not 1-based.
// The numbers could also have the form X/Y for multiple parts.
// Fix the number so it is closer to what the user expects.
static QString fixEpisodeNumber(const QString &str, int *season)
{
    QStringList components = str.split(QLatin1String("."));
    QString result;
    bool needDot = false;
    *season = 0;
    for (int index = 0; index < components.size(); ++index) {
        QString comp = components.at(index);
        int slash = comp.indexOf(QLatin1Char('/'));
        if (slash >= 0)
            comp = comp.left(slash);
        if (comp.isEmpty())
            continue;
        if (needDot) {
            if (index == 2)
                result += QObject::tr(", Part ");
            else
                result += QLatin1Char('.');
        }
        result += QString::number(comp.toInt() + 1);
        if (!needDot)
            *season = comp.toInt() + 1;
        needDot = true;
    }
    return result;
}

void TvProgramme::load(QXmlStreamReader *reader)
{
    // Will leave the XML stream positioned on </programme>.
    // Format details:
    //    http://xmltv.cvs.sourceforge.net/viewvc/xmltv/xmltv/xmltv.dtd
    // The parser is not yet complete - lots of other information
    // may be provided.  This set is reasonably inclusive of the
    // data found in OzTivo listings.  Feel free to add extra fields
    // if you see the warning printed.
    Q_ASSERT(reader->isStartElement());
    Q_ASSERT(reader->name() == QLatin1String("programme"));
    QString start = reader->attributes().value(QLatin1String("start")).toString();
    QString stop = reader->attributes().value(QLatin1String("stop")).toString();
    m_start = TvChannel::stringToDateTime(start, m_channel);
    m_stop = TvChannel::stringToDateTime(stop, m_channel);
    while (!reader->hasError()) {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader->name() == QLatin1String("title")) {
                m_title = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
                m_indexTitle = m_title.toLower();
            } else if (reader->name() == QLatin1String("sub-title")) {
                m_subTitle = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("desc")) {
                m_description = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("date")) {
                m_date = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("director")) {
                m_directors += reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("actor")) {
                m_actors += reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("presenter")) {
                m_presenters += reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("category")) {
                QString category = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
                m_categories += category;
                if (category == QLatin1String("Movie") ||
                        category == QLatin1String("Movies"))
                    m_isMovie = true;   // FIXME: other languages
            } else if (reader->name() == QLatin1String("rating")) {
                m_rating = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("star-rating")) {
                m_starRating = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("episode-num")) {
                QStringRef system = reader->attributes().value(QLatin1String("system"));
                if (system == QLatin1String("xmltv_ns")) {
                    m_episodeNumber = fixEpisodeNumber
                        (reader->readElementText
                            (QXmlStreamReader::IncludeChildElements),
                         &m_season);
                }
            } else if (reader->name() == QLatin1String("language")) {
                m_language = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("orig-language")) {
                m_originalLanguage = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("country")) {
                m_country = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("aspect")) {
                m_aspectRatio = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("premiere")) {
                m_isPremiere = true;
            } else if (reader->name() == QLatin1String("previously-shown")) {
                m_isRepeat = true;
            // The following are container elements that are ignored.
            } else if (reader->name() == QLatin1String("credits") ||
                       reader->name() == QLatin1String("video")) {
            // The following are in the DTD, but not processed yet.
            } else if (reader->name() == QLatin1String("present") ||
                       reader->name() == QLatin1String("quality") ||
                       reader->name() == QLatin1String("audio") ||
                       reader->name() == QLatin1String("stereo") ||
                       reader->name() == QLatin1String("last-chance") ||
                       reader->name() == QLatin1String("new") ||
                       reader->name() == QLatin1String("subtitles") ||
                       reader->name() == QLatin1String("review") ||
                       reader->name() == QLatin1String("url") ||
                       reader->name() == QLatin1String("writer") ||
                       reader->name() == QLatin1String("adapter") ||
                       reader->name() == QLatin1String("producer") ||
                       reader->name() == QLatin1String("composer") ||
                       reader->name() == QLatin1String("editor") ||
                       reader->name() == QLatin1String("commentator") ||
                       reader->name() == QLatin1String("guest") ||
                       reader->name() == QLatin1String("length") ||
                       reader->name() == QLatin1String("icon")) {
                qWarning() << "Warning: unhandled standard programme element:" << reader->name();
            } else {
                qWarning() << "Warning: unknown programme element:" << reader->name();
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("programme"))
                break;
        }
    }
    m_shortDescription = QString();
    m_longDescription = QString();

    // Find the bookmark that best matches the programme.
    if (!m_channel->m_needBookmarkRefresh) {
        TvBookmark *bookmark = 0;
        TvBookmark::Match match =
            m_channel->channelList()->bookmarkList()->match
                (this, &bookmark, TvBookmark::PartialMatches | TvBookmark::NonMatching);
        setBookmark(bookmark, match);
    }
}

void TvProgramme::setBookmark
    (TvBookmark *bookmark, TvBookmark::Match match)
{
    if (m_bookmark) {
        if (m_bookmark != bookmark) {
            m_bookmark->removeProgramme(this);
            if (bookmark)
                bookmark->addProgramme(this);
            m_bookmark = bookmark;
        }
    } else {
        m_bookmark = bookmark;
        if (bookmark)
            bookmark->addProgramme(this);
    }
    if (m_match != match) {
        m_match = match;
        m_shortDescription = QString();
    }
}

void TvProgramme::clearBookmarkMatch()
{
    m_bookmark = 0;
    m_match = TvBookmark::NoMatch;
    m_shortDescription = QString();
}

static bool sortMovedProgrammes(TvProgramme *p1, TvProgramme *p2)
{
    int cmp = p1->channel()->compare(p2->channel());
    if (cmp != 0)
        return cmp < 0;
    return p1->start() < p2->start();
}

QString TvProgramme::shortDescription() const
{
    if (!m_shortDescription.isEmpty())
        return m_shortDescription;
    QString desc;
    bool bold = false;
    TvBookmark::Match match = displayMatch();
    switch (match) {
    case TvBookmark::NoMatch:
    case TvBookmark::ShouldMatch:
    case TvBookmark::TickMatch:
        desc += QLatin1String("<font color=\"#000000\">");
        break;
    case TvBookmark::FullMatch:
        desc += QLatin1String("<font color=\"") +
                m_bookmark->color().name() +
                QLatin1String("\">");
        desc += QLatin1String("<b>");
        bold = true;
        break;
    case TvBookmark::Overrun:
    case TvBookmark::Underrun:
        desc += QLatin1String("<font color=\"") +
                m_bookmark->color().lighter(150).name() +
                QLatin1String("\">");
        desc += QLatin1String("<b>");
        bold = true;
        break;
    case TvBookmark::TitleMatch:
        desc += QLatin1String("<font color=\"") +
                m_bookmark->color().name() +
                QLatin1String("\">");
        break;
    }
    if (m_isMovie)
        desc += QObject::tr("MOVIE: %1").arg(Qt::escape(m_title));
    else
        desc += Qt::escape(m_title);
    if (bold)
        desc += QLatin1String("</b>");
    desc += QLatin1String("</font>");
    desc += QLatin1String("<font color=\"#606060\">");
    if (!m_date.isEmpty() && m_date != QLatin1String("0")) {
        desc += QLatin1String(" (") +
                Qt::escape(m_date) + QLatin1String(")");
    }
    if (!m_rating.isEmpty()) {
        desc += QLatin1String(" (") +
                Qt::escape(m_rating) + QLatin1String(")");
        if (m_isRepeat)
            desc += QObject::tr("(R)");
    } else if (m_isRepeat) {
        desc += QObject::tr(" (R)");
    }
    if (!m_categories.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(m_categories.at(0));
    }
    if (!m_actors.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(m_actors.at(0));
    } else if (!m_presenters.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(m_presenters.at(0));
    } else if (!m_directors.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(QObject::tr("Director: %1").arg(m_directors.at(0)));
    }
    if (!m_starRating.isEmpty()) {
        QStringList list = m_starRating.split(QLatin1String("/"));
        if (list.size() >= 2) {
            int numer = list.at(0).toInt();
            int denom = list.at(1).toInt();
            if (numer > 0 && denom > 0) {
                qreal stars = numer * 5.0f / denom;
                if (stars < 0.0f)
                    stars = 0.0f;
                else if (stars > 5.0f)
                    stars = 5.0f;
                desc += QLatin1Char(' ');
                int istars = int(stars);
                qreal leftOver = stars - istars;
                for (int black = 0; black < istars; ++black)
                    desc += QChar(0x2605);  // Black star.
                if (leftOver >= 0.5f) {
                    desc += QChar(0x272D);  // Black outlined star.
                    ++istars;
                }
                for (int white = istars; white < 5; ++white)
                    desc += QChar(0x2606);  // White star.
            }
        }
    }
    if (!m_subTitle.isEmpty()) {
        desc += QLatin1String("<br><i>") +
                Qt::escape(m_subTitle) +
                QLatin1String("</i>");
    }
    if (!m_episodeNumber.isEmpty()) {
        desc += QObject::tr(" <i>(Episode %1)</i>").arg
            (Qt::escape(m_episodeNumber));
    }
    if (m_isPremiere)
        desc += QObject::tr(", <font color=\"red\"><b>Premiere</b></font>");
    if (match == TvBookmark::ShouldMatch) {
        desc += QLatin1String("<br><s>") +
                Qt::escape(m_bookmark->title()) +
                QLatin1String("</s>");
        QList<TvProgramme *> others = m_bookmark->m_matchingProgrammes;
        bool needComma = false;
        qSort(others.begin(), others.end(), sortMovedProgrammes);
        for (int index = 0; index < others.size(); ++index) {
            TvProgramme *other = others.at(index);
            if (other->match() != TvBookmark::TitleMatch)
                continue;
            if (needComma) {
                desc += QLatin1String("</li><li>");
            } else {
                desc += QObject::tr(" may have moved to:<ul><li>");
                needComma = true;
            }
            desc += other->channel()->name();
            desc += other->start().date().toString(QLatin1String(" dddd, MMMM d, "));
            desc += other->start().time().toString(Qt::LocaleDate);
        }
        if (needComma)
            desc += QLatin1String("</li></ul>");
    }
    if (match != TvBookmark::NoMatch &&
            match != TvBookmark::ShouldMatch &&
            !m_subTitle.isEmpty()) {
        QList<TvProgramme *> others = m_bookmark->m_matchingProgrammes;
        qSort(others.begin(), others.end(), sortMovedProgrammes);
        bool needComma = false;
        for (int index = 0; index < others.size(); ++index) {
            TvProgramme *other = others.at(index);
            if (other == this || other->subTitle() != m_subTitle)
                continue;
            if (needComma) {
                desc += QLatin1String(", ");
            } else {
                desc += QLatin1String("<br>") +
                        QObject::tr("Other showings: ");
                needComma = true;
            }
            if (m_channel != other->channel()) {
                desc += other->channel()->name();
                desc += QLatin1Char(' ');
            }
            if (m_start.date() != other->start().date()) {
                desc += QDate::longDayName(other->start().date().dayOfWeek());
                desc += QLatin1Char(' ');
            }
            desc += other->start().time().toString(Qt::LocaleDate);
        }
    }
    desc += QLatin1String("</font>");
    m_shortDescription = desc;
    return desc;
}

QString TvProgramme::longDescription() const
{
    if (!m_longDescription.isEmpty())
        return m_longDescription;
    QString desc = QLatin1String("<qt><p><i>");
    desc += Qt::escape(m_description) + QLatin1String("</i></p>");
    desc += QObject::tr("<p><b>Duration:</b> %1 minutes</p>")
                .arg(QString::number(m_start.secsTo(m_stop) / 60));
    if (m_categories.size() > 1) {
        desc += QObject::tr("<p><b>Categories:</b> %1</p>")
            .arg(Qt::escape(m_categories.join(QLatin1String(", "))));
    }
    if (!m_actors.isEmpty()) {
        desc += QObject::tr("<p><b>Starring:</b> %1</p>")
            .arg(Qt::escape(m_actors.join(QLatin1String(", "))));
    }
    if (!m_presenters.isEmpty()) {
        desc += QObject::tr("<p><b>Presenter:</b> %1</p>")
            .arg(Qt::escape(m_presenters.join(QLatin1String(", "))));
    }
    if (!m_directors.isEmpty()) {
        desc += QObject::tr("<p><b>Director:</b> %1</p>")
            .arg(Qt::escape(m_directors.join(QLatin1String(", "))));
    }
    if (!m_language.isEmpty() && !m_originalLanguage.isEmpty() &&
            m_language != m_originalLanguage) {
        desc += QObject::tr("<p><b>Language:</b> %1 (original in %2)</p>")
            .arg(Qt::escape(m_language), Qt::escape(m_originalLanguage));
    } else if (!m_language.isEmpty()) {
        desc += QObject::tr("<p><b>Language:</b> %1</p>")
            .arg(Qt::escape(m_language));
    } else if (!m_originalLanguage.isEmpty()) {
        desc += QObject::tr("<p><b>Original Language:</b> %1</p>")
            .arg(Qt::escape(m_originalLanguage));
    }
    if (!m_country.isEmpty()) {
        desc += QObject::tr("<p><b>Country:</b> %1</p>")
            .arg(Qt::escape(m_country));
    }
    if (!m_aspectRatio.isEmpty()) {
        desc += QObject::tr("<p><b>Aspect ratio:</b> %1</p>")
            .arg(Qt::escape(m_aspectRatio));
    }
    desc += QLatin1String("</qt>");
    m_longDescription = desc;
    return desc;
}

TvBookmark::Match TvProgramme::displayMatch() const
{
    TvBookmark::Match result = m_match;

    if (m_match == TvBookmark::ShouldMatch) {
        // Suppress failed matches either side of a successful match,
        // and remove redundant failed matches.
        if (m_prev && m_prev->bookmark() == m_bookmark) {
            if (m_prev->match() != TvBookmark::NoMatch)
                result = TvBookmark::NoMatch;
        } else if (m_next && m_next->bookmark() == m_bookmark) {
            if (m_next->match() != TvBookmark::ShouldMatch &&
                    m_next->match() != TvBookmark::NoMatch)
                result = TvBookmark::NoMatch;
        }
    } else if (m_match == TvBookmark::TitleMatch) {
        // Partial match immediately before or after a full
        // match is labelled as an underrun or overrun.
        // Probably a double episode where one of the episodes
        // falls outside the normal bookmark range.
        if (m_prev && m_prev->stop() == m_start &&
                m_prev->match() == TvBookmark::FullMatch &&
                m_prev->bookmark() == m_bookmark) {
            result = TvBookmark::Overrun;
        } else if (m_next && m_next->start() == m_stop &&
                   m_next->match() == TvBookmark::FullMatch &&
                   m_next->bookmark() == m_bookmark) {
            result = TvBookmark::Underrun;
        }
    }

    return result;
}
