#include "ui/dialogs/PacienteDetalleDialog.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"
#include "ui/components/StatusChip.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

PacienteDetalleDialog::PacienteDetalleDialog(ApiClient *api,
                                             const QString &dni,
                                             const QString &aptitudTipo,
                                             QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(dni)
    , m_aptitudTipo(aptitudTipo)
    , m_status(new QLabel(this))
    , m_aptitudChip(StatusChip::make(aptitudLabel(aptitudTipo), StatusChip::aptitudKind(aptitudTipo), this))
    , m_form(new QFormLayout)
{
    setWindowTitle(QStringLiteral("Paciente %1").arg(dni));
    setMinimumWidth(460);
    setModal(true);

    auto *title = new QLabel(QStringLiteral("Ficha del paciente"), this);
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("DNI %1").arg(dni), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));

    addRow(m_form, QStringLiteral("DNI"));
    addRow(m_form, QStringLiteral("Nombre"));
    addRow(m_form, QStringLiteral("Apellido"));
    addRow(m_form, QStringLiteral("Teléfono"));
    addRow(m_form, QStringLiteral("Sexo"));
    addRow(m_form, QStringLiteral("Altura"));
    m_form->addRow(QStringLiteral("Aptitud"), m_aptitudChip);

    auto *card = new Card(this);
    card->addLayout(m_form);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("Cerrar"));
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
    layout->addWidget(card);
    layout->addWidget(m_status);
    layout->addLayout(buttonsRow);

    m_api->get(QStringLiteral("/api/pacientes/%1").arg(dni),
               [this](const ApiClient::Response &response) {
                   if (!response.ok) {
                       m_status->setText(response.errorMessage);
                       m_status->show();
                       return;
                   }
                   populate(response.json.object());
               });
}

QLabel *PacienteDetalleDialog::addRow(QFormLayout *form, const QString &label)
{
    auto *value = new QLabel(QStringLiteral("-"), this);
    value->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(label, value);
    return value;
}

void PacienteDetalleDialog::populate(const QJsonObject &patient)
{
    const auto setValue = [this](int row, const QString &text) {
        if (QLayoutItem *item = m_form->itemAt(row, QFormLayout::FieldRole)) {
            if (auto *label = qobject_cast<QLabel *>(item->widget()))
                label->setText(text);
        }
    };

    setValue(0, patient.value(QStringLiteral("dni")).toString());
    setValue(1, patient.value(QStringLiteral("nombre")).toString());
    setValue(2, patient.value(QStringLiteral("apellido")).toString());
    setValue(3, patient.value(QStringLiteral("telefono")).toString());

    const QString sexo = patient.value(QStringLiteral("sexo")).toString();
    setValue(4, sexo.isEmpty() ? QStringLiteral("Sin especificar") : sexo);

    const QString altura = patient.value(QStringLiteral("altura")).toVariant().toString();
    setValue(5, altura.isEmpty() ? QStringLiteral("Sin especificar")
                                 : QStringLiteral("%1 cm").arg(altura));

    m_aptitudChip->setText(aptitudLabel(m_aptitudTipo));
    m_aptitudChip->setProperty("kind", StatusChip::aptitudKind(m_aptitudTipo));
    m_aptitudChip->style()->unpolish(m_aptitudChip);
    m_aptitudChip->style()->polish(m_aptitudChip);
}
