#include "ui/dialogs/DonacionDetalleDialog.h"

#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"
#include "ui/components/DataTable.h"
#include "ui/components/StatusChip.h"
#include "util/DateUtils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <cmath>

namespace {

/** Human-readable rendering of a JSON scalar, with an em dash for nulls. */
QString display(const QJsonValue &value)
{
    if (value.isNull() || value.isUndefined())
        return QStringLiteral("—");
    if (value.isBool())
        return value.toBool() ? QStringLiteral("Sí") : QStringLiteral("No");
    if (value.isDouble()) {
        const double number = value.toDouble();
        if (std::floor(number) == number)
            return QString::number(static_cast<qlonglong>(number));
        return QString::number(number);
    }
    const QString text = value.toString();
    return text.isEmpty() ? QStringLiteral("—") : text;
}

QString prettyDate(const QString &iso)
{
    return iso.isEmpty() ? QStringLiteral("—") : util::prettyDate(iso);
}

void addRow(QFormLayout *form, const QString &label, const QString &value)
{
    auto *valueLabel = new QLabel(value);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);
    form->addRow(label, valueLabel);
}

} // namespace

DonacionDetalleDialog::DonacionDetalleDialog(const QJsonObject &donation, QWidget *parent)
    : QDialog(parent)
    , m_descartadaChip(nullptr)
    , m_autoexclusionChip(nullptr)
{
    const QString numero = donation.value(QStringLiteral("numero_donacion")).toString();
    setWindowTitle(QStringLiteral("Donación %1").arg(numero));
    setMinimumSize(720, 640);
    setModal(true);

    const QJsonObject paciente = donation.value(QStringLiteral("paciente")).toObject();
    const QJsonObject sede = donation.value(QStringLiteral("sede")).toObject();
    const QJsonObject tipo = donation.value(QStringLiteral("tipo")).toObject();

    auto *title = new QLabel(QStringLiteral("Donación Nº %1").arg(numero), this);
    title->setObjectName(QStringLiteral("PageTitle"));

    const QString pacienteName = QStringLiteral("%1 %2")
                                     .arg(paciente.value(QStringLiteral("nombre")).toString(),
                                          paciente.value(QStringLiteral("apellido")).toString())
                                     .trimmed();
    QStringList meta;
    meta << prettyDate(donation.value(QStringLiteral("fecha")).toString());
    if (!pacienteName.isEmpty())
        meta << QStringLiteral("%1 (DNI %2)").arg(pacienteName, paciente.value(QStringLiteral("dni")).toString());
    if (!sede.value(QStringLiteral("nombre")).toString().isEmpty())
        meta << QStringLiteral("Sede %1").arg(sede.value(QStringLiteral("nombre")).toString());
    auto *subtitle = new QLabel(meta.join(QStringLiteral("  ·  ")), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));
    subtitle->setWordWrap(true);

    // --- State chips ---------------------------------------------------------
    const bool descartada = donation.value(QStringLiteral("descartada")).toBool();
    m_descartadaChip = StatusChip::make(descartada ? QStringLiteral("Descartada") : QStringLiteral("No descartada"),
                                        descartada ? QStringLiteral("error") : QStringLiteral("muted"), this);

    const QJsonObject autoexclusion = donation.value(QStringLiteral("autoexclusion")).toObject();
    auto *chipsRow = new QHBoxLayout;
    chipsRow->setSpacing(Theme::SpacingSm);
    chipsRow->addWidget(m_descartadaChip);
    if (!autoexclusion.isEmpty()) {
        m_autoexclusionChip = StatusChip::make(QStringLiteral("Autoexclusión"), QStringLiteral("warning"), this);
        chipsRow->addWidget(m_autoexclusionChip);
    }
    chipsRow->addStretch();

    // --- Bag / clinical cards -----------------------------------------------
    auto *bagForm = new QFormLayout;
    bagForm->setSpacing(Theme::SpacingSm);
    addRow(bagForm, QStringLiteral("Tipo de bolsa"), display(donation.value(QStringLiteral("tipo_bolsa"))));
    addRow(bagForm, QStringLiteral("Anticoagulante"), display(donation.value(QStringLiteral("anticoagulante"))));
    addRow(bagForm, QStringLiteral("Lote"), display(donation.value(QStringLiteral("lote"))));
    addRow(bagForm, QStringLiteral("Tubuladura"), display(donation.value(QStringLiteral("tubuladura"))));
    addRow(bagForm, QStringLiteral("Brazo"), display(donation.value(QStringLiteral("brazo"))));
    addRow(bagForm, QStringLiteral("Dificultad"), display(donation.value(QStringLiteral("dificultad"))));
    addRow(bagForm, QStringLiteral("Doble etiqueta"), display(donation.value(QStringLiteral("doble_etiqueta"))));
    auto *bagCard = new Card(this);
    auto *bagTitle = new QLabel(QStringLiteral("Datos de la bolsa"), bagCard);
    bagTitle->setObjectName(QStringLiteral("CardTitle"));
    bagCard->addWidget(bagTitle);
    bagCard->addLayout(bagForm);

    auto *clinicalForm = new QFormLayout;
    clinicalForm->setSpacing(Theme::SpacingSm);
    addRow(clinicalForm, QStringLiteral("Hemoglobina"), display(donation.value(QStringLiteral("hemoglobina"))));
    addRow(clinicalForm, QStringLiteral("Hematocrito"), display(donation.value(QStringLiteral("hematocrito"))));
    addRow(clinicalForm, QStringLiteral("Plaquetas"), display(donation.value(QStringLiteral("plaquetas"))));
    addRow(clinicalForm, QStringLiteral("Presión arterial"), display(donation.value(QStringLiteral("presion_arterial"))));
    addRow(clinicalForm, QStringLiteral("Frecuencia cardíaca"), display(donation.value(QStringLiteral("frecuencia_cardiaca"))));
    addRow(clinicalForm, QStringLiteral("Peso del donante"), display(donation.value(QStringLiteral("peso_donante"))));
    auto *clinicalCard = new Card(this);
    auto *clinicalTitle = new QLabel(QStringLiteral("Signos clínicos"), clinicalCard);
    clinicalTitle->setObjectName(QStringLiteral("CardTitle"));
    clinicalCard->addWidget(clinicalTitle);
    clinicalCard->addLayout(clinicalForm);

    auto *cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(Theme::SpacingMd);
    cardsRow->addWidget(bagCard, 1);
    cardsRow->addWidget(clinicalCard, 1);

    // --- Donation cycle ------------------------------------------------------
    auto *cycleCard = new Card(this);
    auto *cycleTitle = new QLabel(QStringLiteral("Ciclo de donaciones"), cycleCard);
    cycleTitle->setObjectName(QStringLiteral("CardTitle"));

    auto *cycleRow = new QHBoxLayout;
    cycleRow->setSpacing(Theme::SpacingXl);
    const auto addCycleField = [cycleCard, cycleRow](const QString &label, const QJsonValue &value) {
        auto *column = new QVBoxLayout;
        column->setSpacing(Theme::SpacingXs);
        auto *fieldLabel = new QLabel(label, cycleCard);
        fieldLabel->setObjectName(QStringLiteral("FieldLabel"));
        auto *fieldValue = new QLabel(display(value), cycleCard);
        fieldValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
        column->addWidget(fieldLabel);
        column->addWidget(fieldValue);
        cycleRow->addLayout(column);
    };
    addCycleField(QStringLiteral("Ciclo"), donation.value(QStringLiteral("ciclo")));
    addCycleField(QStringLiteral("Serie"), donation.value(QStringLiteral("serie")));
    addCycleField(QStringLiteral("Donación"), donation.value(QStringLiteral("donacion")));
    cycleRow->addStretch();

    cycleCard->addWidget(cycleTitle);
    cycleCard->addLayout(cycleRow);

    // --- Components ----------------------------------------------------------
    auto *componentsCard = new Card(this);
    auto *componentsTitle = new QLabel(QStringLiteral("Componentes"), componentsCard);
    componentsTitle->setObjectName(QStringLiteral("CardTitle"));
    auto *componentsTable = new DataTable(componentsCard);
    componentsTable->setHeaders({
        QStringLiteral("Tipo"),
        QStringLiteral("Vencimiento"),
        QStringLiteral("Peso"),
        QStringLiteral("Descartado"),
        QStringLiteral("Motivo"),
    });

    const QJsonArray componentes = donation.value(QStringLiteral("componentes")).toArray();
    QTableWidget *table = componentsTable->table();
    table->setRowCount(componentes.size());
    for (int row = 0; row < componentes.size(); ++row) {
        const QJsonObject componente = componentes.at(row).toObject();
        const QJsonObject componentTipo = componente.value(QStringLiteral("tipo")).toObject();
        const bool discarded = componente.value(QStringLiteral("descartado")).toBool();

        table->setItem(row, 0,
                       new QTableWidgetItem(componentTipo.value(QStringLiteral("nombre")).toString()));
        table->setItem(row, 1,
                       new QTableWidgetItem(prettyDate(componente.value(QStringLiteral("vencimiento")).toString())));
        table->setItem(row, 2, new QTableWidgetItem(display(componente.value(QStringLiteral("peso")))));
        table->setCellWidget(row, 3,
                             StatusChip::makeCell(discarded ? QStringLiteral("Sí") : QStringLiteral("No"),
                                                  discarded ? QStringLiteral("error") : QStringLiteral("muted")));
        table->setItem(row, 4, new QTableWidgetItem(display(componente.value(QStringLiteral("motivo_descarte")))));
    }
    if (componentes.isEmpty())
        componentsTable->showEmpty(QStringLiteral("Sin componentes registrados."));
    else
        componentsTable->showTable();

    componentsCard->addWidget(componentsTitle);
    componentsCard->addWidget(componentsTable);

    // --- Buttons -------------------------------------------------------------
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("Cerrar"));
    clearButtonIcon(buttons->button(QDialogButtonBox::Close));
    buttons->button(QDialogButtonBox::Close)->setCursor(Qt::PointingHandCursor);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *buttonsRow = new QHBoxLayout;
    buttonsRow->addStretch();
    buttonsRow->addWidget(buttons);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addLayout(chipsRow);
    layout->addLayout(cardsRow);
    layout->addWidget(cycleCard);
    layout->addWidget(componentsCard, 1);
    layout->addLayout(buttonsRow);
}
