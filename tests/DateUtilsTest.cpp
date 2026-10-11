#include "util/DateUtils.h"

#include <QtTest>

class DateUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void formatsIsoDateAsDayMonthYear();
    void formatsLeapDay();
    void keepsRawValueWhenNotIso();
    void keepsEmptyValue();
};

void DateUtilsTest::formatsIsoDateAsDayMonthYear()
{
    QCOMPARE(util::prettyDate(QStringLiteral("2024-03-15")), QStringLiteral("15/03/2024"));
    QCOMPARE(util::prettyDate(QStringLiteral("1990-01-02")), QStringLiteral("02/01/1990"));
}

void DateUtilsTest::formatsLeapDay()
{
    QCOMPARE(util::prettyDate(QStringLiteral("2024-02-29")), QStringLiteral("29/02/2024"));
}

void DateUtilsTest::keepsRawValueWhenNotIso()
{
    QCOMPARE(util::prettyDate(QStringLiteral("ayer")), QStringLiteral("ayer"));
    QCOMPARE(util::prettyDate(QStringLiteral("15/03/2024")), QStringLiteral("15/03/2024"));
}

void DateUtilsTest::keepsEmptyValue()
{
    QCOMPARE(util::prettyDate(QString()), QString());
}

QTEST_MAIN(DateUtilsTest)

#include "DateUtilsTest.moc"
