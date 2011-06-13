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
#include <QtCore/qdebug.h>

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
    m_start = QDateTime::fromString(reader->attributes().value(QLatin1String("start")).toString(), QLatin1String("yyyyMMddHHmmss"));
    m_stop = QDateTime::fromString(reader->attributes().value(QLatin1String("stop")).toString(), QLatin1String("yyyyMMddHHmmss"));
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
            } else {
                qWarning() << "Warning: unknown programme element:" << reader->name();
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader->name() == QLatin1String("programme"))
                break;
        }
    }
}
