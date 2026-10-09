#include "ui/dialogs/PacienteDetalleDialog.h"

#include "api/ApiClient.h"
#include "ui/UiUtils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
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
    , m_form(new QFormLayout)
{
    setWindowTitle(QStringLiteral("Paciente %1").arg(dni));
    setMinimumWidth(400);

    m_status->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_status->setWordWrap(true);

    addRow(m_form, QStringLiteral("DNI"));
    addRow(m_form, QStringLiteral("Nombre"));
    addRow(m_form, QStringLiteral("Apellido"));
    addRow(m_form, QStringLiteral("Teléfono"));
    addRow(m_form, QStringLiteral("Sexo"));
    addRow(m_form, QStringLiteral("Altura"));
    addRow(m_form, QStringLiteral("Aptitud"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("Cerrar"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(m_form);
    layout->addWidget(m_status);
    layout->addWidget(buttons);

    m_api->get(QStringLiteral("/api/pacientes/%1").arg(dni),
               [this](const ApiClient::Response &response) {
                   if (!response.ok) {
                       m_status->setText(response.errorMessage);
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

    setValue(6, aptitudLabel(m_aptitudTipo));
}
