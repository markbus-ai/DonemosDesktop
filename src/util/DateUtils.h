#pragma once

#include <QDate>
#include <QString>

namespace util {

/**
 * Render an ISO date (yyyy-MM-dd) as dd/MM/yyyy for display.
 *
 * Returns the input unchanged when it is not a valid ISO date, so raw or
 * legacy values still surface instead of an empty cell.
 */
inline QString prettyDate(const QString &iso)
{
    const QDate date = QDate::fromString(iso, Qt::ISODate);
    return date.isValid() ? date.toString(QStringLiteral("dd/MM/yyyy")) : iso;
}

} // namespace util
