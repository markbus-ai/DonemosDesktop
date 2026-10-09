#pragma once

#include <QString>

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
