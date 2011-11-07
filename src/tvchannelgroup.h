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

#ifndef _TVCHANNELGROUP_H
#define _TVCHANNELGROUP_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qsettings.h>

class TvChannel;
class TvChannelList;

class TvChannelGroup
{
public:
    explicit TvChannelGroup(TvChannelList *channelList);
    explicit TvChannelGroup(const TvChannelGroup *copyFrom);
    ~TvChannelGroup();

    TvChannelList *channelList() const { return m_channelList; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QStringList channelIds() const { return m_channelIds; }
    void setChannelIds(const QStringList &ids) { m_channelIds = ids; }

    QList<TvChannel *> channels() const;
    QList<TvChannel *> sortedChannels() const;

    void addChannel(TvChannel *channel);
    void removeChannel(TvChannel *channel);

    static QList<TvChannelGroup *> loadSettings
        (TvChannelList *channelList, QSettings *settings);
    static void saveSettings
        (const QList<TvChannelGroup *> &groups, QSettings *settings);

private:
    TvChannelList *m_channelList;
    QString m_name;
    QStringList m_channelIds;
};

#endif
