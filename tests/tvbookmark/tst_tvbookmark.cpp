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
    void parseSeasons_data();
    void parseSeasons();

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
    QVERIFY(!b.anyTime());
    QVERIFY(!b.color().isValid());
    QVERIFY(b.isOnAir());
    QVERIFY(b.seasons().isEmpty());
    QVERIFY(b.seasonList().isEmpty());
    QVERIFY(b.years().isEmpty());
    QVERIFY(b.yearList().isEmpty());

    QList< QPair<int, int> > list;
    list.append(QPair<int, int>(1, 1));
    list.append(QPair<int, int>(3, 5));
    list.append(QPair<int, int>(7, 0x7fffffff));

    QList< QPair<int, int> > list2;
    list2.append(QPair<int, int>(1991, 1991));
    list2.append(QPair<int, int>(1993, 1995));
    list2.append(QPair<int, int>(1997, 0x7fffffff));

    b.setTitle(QLatin1String("foo"));
    b.setChannelId(QLatin1String("BAR"));
    b.setDayOfWeek(TvBookmark::Wednesday);
    b.setStartTime(QTime(11, 45, 23));
    b.setStopTime(QTime(17, 54, 32));
    b.setAnyTime(true);
    b.setColor(Qt::red);
    b.setOnAir(false);
    b.setSeasons(QLatin1String("1,3-5, 7+"));
    b.setYears(QLatin1String("1991,1993-1995, 1997+"));

    QCOMPARE(b.title(), QLatin1String("foo"));
    QCOMPARE(b.channelId(), QLatin1String("BAR"));
    QVERIFY(b.dayOfWeek() == TvBookmark::Wednesday);
    QCOMPARE(b.dayOfWeekMask(), 0x08);
    QVERIFY(b.startTime() == QTime(11, 45, 23));
    QVERIFY(b.stopTime() == QTime(17, 54, 32));
    QVERIFY(b.anyTime());
    QCOMPARE(b.color().red(), 255);
    QCOMPARE(b.color().blue(), 0);
    QCOMPARE(b.color().green(), 0);
    QVERIFY(!b.isOnAir());
    QCOMPARE(b.seasons(), QLatin1String("1,3-5,7+"));
    QVERIFY(b.seasonList() == list);
    QCOMPARE(b.years(), QLatin1String("1991,1993-1995,1997+"));
    QVERIFY(b.yearList() == list2);

    TvBookmark b2(b);
    QCOMPARE(b2.title(), QLatin1String("foo"));
    QCOMPARE(b2.channelId(), QLatin1String("BAR"));
    QVERIFY(b2.dayOfWeek() == TvBookmark::Wednesday);
    QCOMPARE(b2.dayOfWeekMask(), 0x08);
    QVERIFY(b2.startTime() == QTime(11, 45, 23));
    QVERIFY(b2.stopTime() == QTime(17, 54, 32));
    QVERIFY(b2.anyTime());
    QCOMPARE(b2.color().red(), 255);
    QCOMPARE(b2.color().blue(), 0);
    QCOMPARE(b2.color().green(), 0);
    QVERIFY(!b2.isOnAir());
    QCOMPARE(b2.seasons(), QLatin1String("1,3-5,7+"));
    QVERIFY(b2.seasonList() == list);
    QCOMPARE(b.years(), QLatin1String("1991,1993-1995,1997+"));
    QVERIFY(b.yearList() == list2);
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

void tst_TvBookmark::parseSeasons_data()
{
    QTest::addColumn<QString>("seasons");
    QTest::addColumn<QString>("listStr");
    QTest::addColumn<QString>("formatted");

    QTest::newRow("empty")
        << "" << "" << "";

    QTest::newRow("single1")
        << "1" << "1-1" << "1";
    QTest::newRow("single2")
        << " 142 \t" << "142-142" << "142";

    QTest::newRow("multiple1")
        << "1,3,5-9" << "1-1|3-3|5-9" << "1,3,5-9";
    QTest::newRow("multiple2")
        << "1,3,5+" << "1-1|3-3|5+" << "1,3,5+";
    QTest::newRow("multiple3")
        << "1 3-7 9+" << "1-1|3-7|9+" << "1,3-7,9+";
    QTest::newRow("multiple4")
        << "1 3 - 6 4-4" << "1-1|3-6|4-4" << "1,3-6,4";

    QTest::newRow("error1")
        << "1 x" << "error" << "";
    QTest::newRow("error2")
        << "1,-41" << "error" << "";
    QTest::newRow("error3")
        << "+42" << "error" << "";
    QTest::newRow("error4")
        << "4-" << "error" << "";
    QTest::newRow("error5")
        << "4--6" << "error" << "";
    QTest::newRow("error6")
        << "6-4" << "error" << "";
    QTest::newRow("error7")
        << "0" << "error" << "";
}

void tst_TvBookmark::parseSeasons()
{
    QFETCH(QString, seasons);
    QFETCH(QString, listStr);
    QFETCH(QString, formatted);

    QList< QPair<int, int> > list;
    if (listStr != QLatin1String("error")) {
        QStringList temp = listStr.split('|');
        foreach (QString pair, temp) {
            if (pair.isEmpty())
                continue;
            if (pair.contains(QLatin1Char('+'))) {
                QStringList p = pair.split(QLatin1String("+"));
                list.append(QPair<int, int>(p[0].toInt(), 0x7fffffff));
            } else {
                QStringList p = pair.split(QLatin1String("-"));
                list.append(QPair<int, int>(p[0].toInt(), p[1].toInt()));
            }
        }
        bool ok = false;
        QVERIFY(TvBookmark::parseSeasons(seasons, &ok) == list);
        QVERIFY(ok);

        TvBookmark bookmark;
        bookmark.setSeasons(seasons);
        QCOMPARE(bookmark.seasons(), formatted);
        QVERIFY(bookmark.seasonList() == list);
    } else {
        bool ok = true;
        QVERIFY(TvBookmark::parseSeasons(seasons, &ok).isEmpty());
        QVERIFY(!ok);

        TvBookmark bookmark;
        bookmark.setSeasons(QLatin1String("1,3"));
        QVERIFY(!bookmark.seasonList().isEmpty());
        bookmark.setSeasons(seasons);
        QCOMPARE(bookmark.seasons(), formatted);
        QVERIFY(bookmark.seasonList().isEmpty());
    }
}

QTEST_MAIN(tst_TvBookmark)

#include "tst_tvbookmark.moc"
