#pragma once

#include <QString>

class QLabel;
class QWidget;

/**
 * Status chips: small pill labels colored by semantics. Kinds map to the brand
 * palette via the `kind` dynamic property (`success`, `warning`, `error`,
 * `muted`) styled in resources/style.qss.
 */
namespace StatusChip {

/** Chip label with the given text and semantic kind. */
QLabel *make(const QString &text, const QString &kind, QWidget *parent = nullptr);

/**
 * Transparent cell wrapper holding a left-aligned chip. Use it as a table cell
 * widget so row selection stays visible around the pill.
 */
QWidget *makeCell(const QString &text, const QString &kind, QWidget *parent = nullptr);

/** Semantic kind for an aptitud code (APTO / APTO_OBSERVACION / NO_APTO). */
QString aptitudKind(const QString &tipo);

/** Semantic kind for an appointment state (PENDIENTE = warning, else muted). */
QString turnoKind(const QString &estado);

/** Human-readable label for an appointment state code. */
QString turnoLabel(const QString &estado);

} // namespace StatusChip
