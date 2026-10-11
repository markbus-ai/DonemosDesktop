#include "ui/components/StatusChip.h"

#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QWidget>

namespace StatusChip {

namespace {
// Chip metrics must match the #Chip rule in resources/style.qss. The label
// width is fixed from font metrics because a QLabel does not include the
// stylesheet padding in its size hint, which clipped long labels inside
// ResizeToContents table columns.
constexpr int kChipFontPx = 11;
constexpr int kChipHorizontalPadding = 26; // 12px per side + small buffer
} // namespace

QLabel *make(const QString &text, const QString &kind, QWidget *parent)
{
    auto *chip = new QLabel(text, parent);
    chip->setObjectName(QStringLiteral("Chip"));
    chip->setProperty("kind", kind);
    chip->setAlignment(Qt::AlignCenter);

    QFont font = chip->font();
    font.setPixelSize(kChipFontPx);
    font.setBold(true);
    chip->setFont(font);

    chip->setFixedHeight(22);
    chip->setFixedWidth(QFontMetrics(font).horizontalAdvance(text) + kChipHorizontalPadding);
    return chip;
}

QWidget *makeCell(const QString &text, const QString &kind, QWidget *parent)
{
    auto *cell = new QWidget(parent);
    cell->setObjectName(QStringLiteral("ChipCell"));
    auto *layout = new QHBoxLayout(cell);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(0);
    layout->addWidget(make(text, kind, cell), 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    return cell;
}

void update(QLabel *chip, const QString &text, const QString &kind)
{
    chip->setText(text);
    chip->setProperty("kind", kind);

    QFont font = chip->font();
    font.setPixelSize(kChipFontPx);
    font.setBold(true);
    chip->setFont(font);
    chip->setFixedWidth(QFontMetrics(font).horizontalAdvance(text) + kChipHorizontalPadding);

    chip->style()->unpolish(chip);
    chip->style()->polish(chip);
}

QString aptitudKind(const QString &tipo)
{
    if (tipo == QLatin1String("APTO"))
        return QStringLiteral("success");
    if (tipo == QLatin1String("APTO_OBSERVACION"))
        return QStringLiteral("warning");
    if (tipo == QLatin1String("NO_APTO"))
        return QStringLiteral("error");
    return QStringLiteral("muted");
}

QString turnoKind(const QString &estado)
{
    if (estado == QLatin1String("PENDIENTE"))
        return QStringLiteral("warning");
    return QStringLiteral("muted");
}

QString turnoLabel(const QString &estado)
{
    if (estado.isEmpty())
        return QStringLiteral("Sin estado");

    QString label = estado;
    label = label.replace(QLatin1Char('_'), QLatin1Char(' ')).toLower();
    if (!label.isEmpty())
        label[0] = label.at(0).toUpper();
    return label;
}

} // namespace StatusChip
