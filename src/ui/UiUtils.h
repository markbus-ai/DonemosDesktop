#pragma once

#include <QIcon>
#include <QPushButton>
#include <QString>
#include <QStyle>

/**
 * Mark a button as the primary (brand red) variant.
 *
 * A plain setProperty() is not enough for buttons owned by a QDialogButtonBox:
 * the box already polished them during construction, so the stylesheet rule is
 * not re-evaluated. Repolish explicitly to pick up the `variant` property.
 */
inline void applyPrimaryVariant(QPushButton *button)
{
    button->setProperty("variant", "primary");
    button->style()->unpolish(button);
    button->style()->polish(button);
}

/** Drop the Fusion standard icon a QDialogButtonBox adds to its buttons. */
inline void clearButtonIcon(QPushButton *button)
{
    button->setIcon(QIcon());
}

/** Human-readable label for an aptitud `tipo` code coming from the API. */
inline QString aptitudLabel(const QString &tipo)
{
    if (tipo.isEmpty())
        return QStringLiteral("Sin datos");
    if (tipo == QLatin1String("APTO"))
        return QStringLiteral("Apto");
    if (tipo == QLatin1String("NO_APTO"))
        return QStringLiteral("No apto");
    if (tipo == QLatin1String("APTO_OBSERVACION"))
        return QStringLiteral("Apto con observación");
    return tipo;
}
