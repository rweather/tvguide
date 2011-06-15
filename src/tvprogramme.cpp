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
    , m_next(0)
{
}

TvProgramme::~TvProgramme()
{
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
            } else if (reader->name() == QLatin1String("category")) {
                m_categories += reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("rating")) {
                m_rating = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("star-rating")) {
                m_starRating = reader->readElementText
                    (QXmlStreamReader::IncludeChildElements);
            } else if (reader->name() == QLatin1String("episode-num")) {
                QStringRef system = reader->attributes().value(QLatin1String("system"));
                if (system == QLatin1String("xmltv_ns")) {
                    m_episodeNumber = reader->readElementText
                        (QXmlStreamReader::IncludeChildElements);
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
            } else if (reader->name() != QLatin1String("credits")) {
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

QString TvProgramme::shortDescription() const
{
    if (!m_shortDescription.isEmpty())
        return m_shortDescription;
    QString desc = QLatin1String("<b>") +
                   Qt::escape(m_title) +
                   QLatin1String("</b>");
    if (!m_date.isEmpty() && m_date != QLatin1String("0")) {
        desc += QLatin1String(" (") +
                Qt::escape(m_date) + QLatin1String(")");
    }
    if (!m_rating.isEmpty()) {
        desc += QLatin1String(" (") +
                Qt::escape(m_rating) + QLatin1String(")");
    }
    if (!m_categories.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(m_categories.at(0));
    }
    if (!m_actors.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(m_actors.at(0));
    } else if (!m_directors.isEmpty()) {
        desc += QLatin1String(", ") +
                Qt::escape(QObject::tr("Director: %1").arg(m_directors.at(0)));
    }
    if (!m_subTitle.isEmpty()) {
        desc += QLatin1String("<br><i>") +
                Qt::escape(m_subTitle) +
                QLatin1String("</i>");
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
    if (m_categories.size() > 1) {
        desc += QObject::tr("<p><b>Categories:</b> %1</p>")
            .arg(Qt::escape(m_categories.join(QLatin1String(", "))));
    }
    if (m_actors.size() > 1) {
        desc += QObject::tr("<p><b>Starring:</b> %1</p>")
            .arg(Qt::escape(m_actors.join(QLatin1String(", "))));
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
    desc += QLatin1String("</qt>");
    m_longDescription = desc;
    return desc;
}
