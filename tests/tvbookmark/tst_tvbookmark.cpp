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

#include "tvbookmark.h"
#include "tvchannel.h"
#include "tvprogramme.h"
#include <QtTest/QtTest>

class tst_TvBookmark : public QObject
{
    Q_OBJECT
public:
    tst_TvBookmark(QObject *parent = 0) : QObject(parent) {}
    ~tst_TvBookmark() {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    void properties();
    void match_data();
    void match();
    void dayOfWeek_data();
    void dayOfWeek();

private:
    TvChannel *channel1;
    TvChannel *channel2;
};

void tst_TvBookmark::initTestCase()
{
    channel1 = new TvChannel(0);
    channel1->setId("FOO");
    channel1->setName("FooTime");

    channel2 = new TvChannel(0);
    channel2->setId("OFF");
    channel2->setName("TvIsOff");
}

void tst_TvBookmark::cleanupTestCase()
{
    delete channel1;
    delete channel2;
}

void tst_TvBookmark::properties()
{
    TvBookmark b;
    QVERIFY(b.title().isEmpty());
    QVERIFY(b.channelId().isEmpty());
    QVERIFY(b.dayOfWeek() == TvBookmark::AnyDay);
    QCOMPARE(b.dayOfWeekMask(), 0xFE);
    QVERIFY(!b.startTime().isValid());
    QVERIFY(!b.stopTime().isValid());
    QVERIFY(!b.color().isValid());
    QVERIFY(b.isEnabled());

    b.setTitle(QLatin1String("foo"));
    b.setChannelId(QLatin1String("BAR"));
    b.setDayOfWeek(TvBookmark::Wednesday);
    b.setStartTime(QTime(11, 45, 23));
    b.setStopTime(QTime(17, 54, 32));
    b.setColor(Qt::red);
    b.setEnabled(false);

    QCOMPARE(b.title(), QLatin1String("foo"));
    QCOMPARE(b.channelId(), QLatin1String("BAR"));
    QVERIFY(b.dayOfWeek() == TvBookmark::Wednesday);
    QCOMPARE(b.dayOfWeekMask(), 0x08);
    QVERIFY(b.startTime() == QTime(11, 45, 23));
    QVERIFY(b.stopTime() == QTime(17, 54, 32));
    QCOMPARE(b.color().red(), 255);
    QCOMPARE(b.color().blue(), 0);
    QCOMPARE(b.color().green(), 0);
    QVERIFY(!b.isEnabled());

    TvBookmark b2(b);
    QCOMPARE(b2.title(), QLatin1String("foo"));
    QCOMPARE(b2.channelId(), QLatin1String("BAR"));
    QVERIFY(b2.dayOfWeek() == TvBookmark::Wednesday);
    QCOMPARE(b2.dayOfWeekMask(), 0x08);
    QVERIFY(b2.startTime() == QTime(11, 45, 23));
    QVERIFY(b2.stopTime() == QTime(17, 54, 32));
    QCOMPARE(b2.color().red(), 255);
    QCOMPARE(b2.color().blue(), 0);
    QCOMPARE(b2.color().green(), 0);
    QVERIFY(!b2.isEnabled());
}

void tst_TvBookmark::match_data()
{
    QTest::addColumn<QString>("btitle");
    QTest::addColumn<QString>("bchannelId");
    QTest::addColumn<QString>("bstartTime");
    QTest::addColumn<QString>("bstopTime");
    QTest::addColumn<int>("bdayOfWeek");
    QTest::addColumn<QString>("ptitle");
    QTest::addColumn<QString>("pchannelId");
    QTest::addColumn<QString>("pstart");
    QTest::addColumn<QString>("pstop");
    QTest::addColumn<int>("options");
    QTest::addColumn<int>("result");

    QTest::newRow("full-match1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::FullMatch);
    QTest::newRow("full-match2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::MondayToFriday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::FullMatch);
    QTest::newRow("full-match3")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:30"
        << "2011-07-19 00:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::FullMatch);
    QTest::newRow("full-match3")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:31"
        << "2011-07-19 00:29"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::FullMatch);

    QTest::newRow("underrun1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:29"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:29"
        << "2011-07-19 21:29"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun3")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:25"
        << "2011-07-19 21:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun4")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:29"
        << "2011-07-19 00:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun5")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:29"
        << "2011-07-19 00:29"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun6")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:25"
        << "2011-07-19 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);
    QTest::newRow("underrun7")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:25"
        << "2011-07-19 23:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Underrun);

    QTest::newRow("overrun1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("overrun2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:35"
        << "2011-07-19 21:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("overrun3")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:30"
        << "2011-07-19 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("overrun4")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:31"
        << "2011-07-19 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("overrun4")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-20 00:00"
        << "2011-07-20 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);

    QTest::newRow("non-match-day1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Wednesday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);
    QTest::newRow("non-match-day2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::MondayToFriday)
        << "FooTime" << "FOO" << "2011-07-17 20:30"
        << "2011-07-17 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);

    QTest::newRow("non-match-channel1")
        << "FooTime" << "OFF" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);

    QTest::newRow("non-match-title1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTim" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::NoMatch);
    QTest::newRow("non-match-title2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTim" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::NonMatching)
        << int(TvBookmark::ShouldMatch);
    QTest::newRow("non-match-title3")
        << "FooTime" << "OFF" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTim" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(TvBookmark::NonMatching)
        << int(TvBookmark::NoMatch);

    QTest::newRow("non-match-time1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 21:30"
        << "2011-07-19 21:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);
    QTest::newRow("non-match-time2")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 19:30"
        << "2011-07-19 20:30"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);
    QTest::newRow("non-match-time3")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-19 23:25"
        << "2011-07-19 23:28"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);

    QTest::newRow("no-partial-match1")
        << "FooTime" << "FOO" << "20:30" << "21:30"
        << int(TvBookmark::Wednesday)
        << "FooTime" << "FOO" << "2011-07-19 20:30"
        << "2011-07-19 21:30"
        << int(0)
        << int(TvBookmark::NoMatch);

    QTest::newRow("next-day1")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::Tuesday)
        << "FooTime" << "FOO" << "2011-07-20 00:00"
        << "2011-07-20 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("next-day2")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::MondayToFriday)
        << "FooTime" << "FOO" << "2011-07-22 00:00"
        << "2011-07-22 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("next-day3")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::MondayToFriday)
        << "FooTime" << "FOO" << "2011-07-23 00:00"
        << "2011-07-23 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
    QTest::newRow("next-day4")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::MondayToFriday)
        << "FooTime" << "FOO" << "2011-07-24 00:00"
        << "2011-07-24 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::TitleMatch);
    QTest::newRow("next-day5")
        << "FooTime" << "FOO" << "23:30" << "00:30"
        << int(TvBookmark::SaturdayAndSunday)
        << "FooTime" << "FOO" << "2011-07-25 00:00"
        << "2011-07-25 00:35"
        << int(TvBookmark::DefaultOptions)
        << int(TvBookmark::Overrun);
}

void tst_TvBookmark::match()
{
    QFETCH(QString, btitle);
    QFETCH(QString, bchannelId);
    QFETCH(QString, bstartTime);
    QFETCH(QString, bstopTime);
    QFETCH(int, bdayOfWeek);
    QFETCH(QString, ptitle);
    QFETCH(QString, pchannelId);
    QFETCH(QString, pstart);
    QFETCH(QString, pstop);
    QFETCH(int, options);
    QFETCH(int, result);

    TvBookmark b;
    b.setTitle(btitle);
    b.setChannelId(bchannelId);
    b.setStartTime(QTime::fromString(bstartTime, QLatin1String("hh:mm")));
    b.setStopTime(QTime::fromString(bstopTime, QLatin1String("hh:mm")));
    b.setDayOfWeek(bdayOfWeek);

    TvProgramme p(pchannelId == QLatin1String("FOO")
                    ? channel1 : channel2);
    p.setTitle(ptitle);
    p.setStart(QDateTime::fromString(pstart, QLatin1String("yyyy-MM-dd hh:mm")));
    p.setStop(QDateTime::fromString(pstop, QLatin1String("yyyy-MM-dd hh:mm")));

    QCOMPARE(int(b.match(&p, TvBookmark::MatchOptions(options))), result);

    b.setEnabled(false);
    QCOMPARE(int(b.match(&p, TvBookmark::MatchOptions(options))), int(TvBookmark::NoMatch));
}

void tst_TvBookmark::dayOfWeek_data()
{
    QTest::addColumn<int>("day");
    QTest::addColumn<int>("mask");

    QTest::newRow("AnyDay")
        << int(TvBookmark::AnyDay) << 0xFE;
    QTest::newRow("Monday")
        << int(TvBookmark::Monday) << 0x02;
    QTest::newRow("Tuesday")
        << int(TvBookmark::Tuesday) << 0x04;
    QTest::newRow("Wednesday")
        << int(TvBookmark::Wednesday) << 0x08;
    QTest::newRow("Thursday")
        << int(TvBookmark::Thursday) << 0x10;
    QTest::newRow("Friday")
        << int(TvBookmark::Friday) << 0x20;
    QTest::newRow("Saturday")
        << int(TvBookmark::Saturday) << 0x40;
    QTest::newRow("Sunday")
        << int(TvBookmark::Sunday) << 0x80;
    QTest::newRow("MondayToFriday")
        << int(TvBookmark::MondayToFriday) << 0x3E;
    QTest::newRow("SaturdayAndSunday")
        << int(TvBookmark::SaturdayAndSunday) << 0xC0;
    QTest::newRow("WednesdayAndFriday")
        << 0x0128 << 0x28;
}

void tst_TvBookmark::dayOfWeek()
{
    QFETCH(int, day);
    QFETCH(int, mask);

    if (!(day & 0x0100)) {
        TvBookmark b1;
        b1.setDayOfWeek(day);
        QCOMPARE(b1.dayOfWeek(), day);
        QCOMPARE(b1.dayOfWeekMask(), mask);

        TvBookmark b2;
        b2.setDayOfWeekMask(mask);
        QCOMPARE(b2.dayOfWeek(), day);
        QCOMPARE(b2.dayOfWeekMask(), mask);
    } else {
        TvBookmark b3;
        b3.setDayOfWeekMask(day & 0xFE);
        QCOMPARE(b3.dayOfWeek(), int(TvBookmark::Mask));
        QCOMPARE(b3.dayOfWeekMask(), mask);
    }
}

QTEST_MAIN(tst_TvBookmark)

#include "tst_tvbookmark.moc"
