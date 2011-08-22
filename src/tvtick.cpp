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

#include "tvtick.h"
#include "tvprogramme.h"
#include "tvchannel.h"

TvTick::TvTick()
{
}

TvTick::~TvTick()
{
}

bool TvTick::match(const TvProgramme *programme) const
{
    if (programme->start() != m_start)
        return false;
    if (programme->channel()->id() != m_channelId)
        return false;
    return programme->title() == m_title;
}

void TvTick::load(QSettings *settings)
{
    m_title = settings->value(QLatin1String("title")).toString();
    m_channelId = settings->value(QLatin1String("channelId")).toString();
    m_start = QDateTime::fromString(settings->value(QLatin1String("start")).toString(), Qt::ISODate);
    m_timestamp = QDateTime::fromString(settings->value(QLatin1String("timestamp")).toString(), Qt::ISODate);
}

void TvTick::save(QSettings *settings)
{
    settings->setValue(QLatin1String("title"), m_title);
    settings->setValue(QLatin1String("channelId"), m_channelId);
    settings->setValue(QLatin1String("start"), m_start.toString(Qt::ISODate));
    settings->setValue(QLatin1String("timestamp"), m_timestamp.toString(Qt::ISODate));
}
