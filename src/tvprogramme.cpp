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
    , m_next(0)
{
}

TvProgramme::~TvProgramme()
{
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
    m_start = TvChannel::stringToDateTime(start);
    m_stop = TvChannel::stringToDateTime(stop);
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
}

void TvProgramme::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        m_shortDescription = QString();
    }
}

void TvProgramme::setBookmark
    (TvBookmark *bookmark, TvBookmark::Match match)
{
    m_bookmark = bookmark;
    m_match = match;
    if (match == TvBookmark::NoMatch || match == TvBookmark::ShouldMatch || match == TvBookmark::TickMatch)
        setColor(QColor());
    else if (match == TvBookmark::FullMatch)
        setColor(bookmark->color());
    else if (match == TvBookmark::TitleMatch)
        setColor(bookmark->color().darker(300));
    else
        setColor(bookmark->color().lighter(150));

    QString title;
    if (match == TvBookmark::ShouldMatch) {
        title = bookmark->title();
        m_bookmark = 0;
    }
    if (m_nonMatchingTitle != title) {
        m_nonMatchingTitle = title;
        m_shortDescription = QString();
    }
}

QString TvProgramme::shortDescription() const
{
    if (!m_shortDescription.isEmpty())
        return m_shortDescription;
    QString desc;
    if (m_color.isValid()) {
        desc += QLatin1String("<font color=\"") +
                m_color.name() +
                QLatin1String("\">");
    }
    desc += QLatin1String("<b>");
    if (m_isMovie)
        desc += QObject::tr("MOVIE: %1").arg(Qt::escape(m_title));
    else
        desc += Qt::escape(m_title);
    desc += QLatin1String("</b>");
    if (m_color.isValid())
        desc += QLatin1String("</font>");
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
    if (!m_nonMatchingTitle.isEmpty()) {
        desc += QLatin1String("<br><s>") +
                Qt::escape(m_nonMatchingTitle) +
                QLatin1String("</s>");
    }
    if (!m_otherShowings.isEmpty()) {
        desc += QLatin1String("<br>") +
                QObject::tr("Other showings:");
        for (int index = 0; index < m_otherShowings.size(); ++index) {
            TvProgramme *other = m_otherShowings.at(index);
            if (index)
                desc += QLatin1String(", ");
            else
                desc += QLatin1Char(' ');
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

void TvProgramme::clearOtherShowings()
{
    if (!m_otherShowings.isEmpty()) {
        m_otherShowings.clear();
        m_shortDescription = QString();
    }
}

void TvProgramme::addOtherShowing(TvProgramme *programme)
{
    m_otherShowings.append(programme);
    m_shortDescription = QString();
}

void TvProgramme::moveShowings(TvProgramme *from)
{
    m_shortDescription = QString();
    m_otherShowings.clear();
    m_otherShowings.append(from);
    m_otherShowings.append(from->m_otherShowings);
    from->m_otherShowings.clear();
    from->m_shortDescription = QString();
}
