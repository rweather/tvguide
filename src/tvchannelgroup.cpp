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

#include "tvchannelgroup.h"
#include "tvchannel.h"
#include "tvchannellist.h"

TvChannelGroup::TvChannelGroup(TvChannelList *channelList)
    : m_channelList(channelList)
{
}

TvChannelGroup::TvChannelGroup(const TvChannelGroup *copyFrom)
    : m_channelList(copyFrom->m_channelList)
    , m_name(copyFrom->m_name)
    , m_channelIds(copyFrom->m_channelIds)
{
}

TvChannelGroup::~TvChannelGroup()
{
}

QList<TvChannel *> TvChannelGroup::channels() const
{
    QList<TvChannel *> chans;
    for (int index = 0; index < m_channelIds.size(); ++index) {
        TvChannel *channel = m_channelList->channel(m_channelIds.at(index));
        if (channel)
            chans.append(channel);
    }
    return chans;
}

static bool sortChannel(TvChannel *c1, TvChannel *c2)
{
    return c1->compare(c2) < 0;
}

QList<TvChannel *> TvChannelGroup::sortedChannels() const
{
    QList<TvChannel *> chans = channels();
    qSort(chans.begin(), chans.end(), sortChannel);
    return chans;
}

void TvChannelGroup::addChannel(TvChannel *channel)
{
    QString id = channel->id();
    if (!m_channelIds.contains(id))
        m_channelIds.append(id);
}

void TvChannelGroup::removeChannel(TvChannel *channel)
{
    m_channelIds.removeAll(channel->id());
}

QList<TvChannelGroup *> TvChannelGroup::loadSettings
    (TvChannelList *channelList, QSettings *settings)
{
    QList<TvChannelGroup *> groups;
    int size = settings->beginReadArray(QLatin1String("groups"));
    for (int index = 0; index < size; ++index) {
        settings->setArrayIndex(index);
        QString name = settings->value(QLatin1String("name")).toString();
        QStringList ids = settings->value(QLatin1String("channels")).toStringList();
        TvChannelGroup *group = new TvChannelGroup(channelList);
        group->setName(name);
        group->setChannelIds(ids);
        groups.append(group);
    }
    settings->endArray();
    return groups;
}

void TvChannelGroup::saveSettings
    (const QList<TvChannelGroup *> &groups, QSettings *settings)
{
    settings->beginWriteArray(QLatin1String("groups"));
    for (int index = 0; index < groups.size(); ++index) {
        TvChannelGroup *group = groups.at(index);
        settings->setArrayIndex(index);
        settings->setValue(QLatin1String("name"), group->name());
        settings->setValue(QLatin1String("channels"), group->channelIds());
    }
    settings->endArray();
}
