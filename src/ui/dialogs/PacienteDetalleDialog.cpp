#include "ui/dialogs/PacienteDetalleDialog.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"
#include "ui/components/StatusChip.h"
#include "ui/dialogs/EditarCicloDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonValue>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {

/** Em dash for null/undefined JSON values, plain number otherwise. */
QString counterText(const QJsonValue &value)
{
    if (value.isNull() || value.isUndefined())
        return QStringLiteral("—");
    return QString::number(value.toVariant().toLongLong());
}

qint64 counterRaw(const QJsonValue &value)
{
    if (value.isNull() || value.isUndefined())
        return 0;
    return value.toVariant().toLongLong();
}

} // namespace

PacienteDetalleDialog::PacienteDetalleDialog(ApiClient *api,
                                             const QString &dni,
                                             const QString &aptitudTipo,
                                             QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(dni)
    , m_aptitudTipo(aptitudTipo)
    , m_status(new QLabel(this))
    , m_feedback(new QLabel(this))
    , m_aptitudChip(StatusChip::make(aptitudLabel(aptitudTipo), StatusChip::aptitudKind(aptitudTipo), this))
    , m_form(new QFormLayout)
    , m_cicloValue(new QLabel(QStringLiteral("—"), this))
    , m_serieValue(new QLabel(QStringLiteral("—"), this))
    , m_donacionValue(new QLabel(QStringLiteral("—"), this))
    , m_editCicloButton(new QPushButton(QStringLiteral("Editar ciclo"), this))
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

    // --- Donation cycle (manual bookkeeping) ---------------------------------
    auto *cycleCard = new Card(this);
    auto *cycleTitle = new QLabel(QStringLiteral("Ciclo de donaciones"), cycleCard);
    cycleTitle->setObjectName(QStringLiteral("CardTitle"));
    auto *cycleHint = new QLabel(
        QStringLiteral("Datos manuales: el personal los completa y puede reiniciar "
                       "el ciclo o empezar una serie nueva cuando quiera."),
        cycleCard);
    cycleHint->setObjectName(QStringLiteral("InfoText"));
    cycleHint->setWordWrap(true);

    const auto addCycleRow = [this, cycleCard](QFormLayout *form, const QString &label, QLabel *value) {
        value->setTextInteractionFlags(Qt::TextSelectableByMouse);
        form->addRow(label, value);
    };
    auto *cycleForm = new QFormLayout;
    cycleForm->setSpacing(Theme::SpacingSm);
    addCycleRow(cycleForm, QStringLiteral("Ciclo actual"), m_cicloValue);
    addCycleRow(cycleForm, QStringLiteral("Serie actual"), m_serieValue);
    addCycleRow(cycleForm, QStringLiteral("Donación actual"), m_donacionValue);

    m_editCicloButton->setCursor(Qt::PointingHandCursor);

    auto *cycleActions = new QHBoxLayout;
    cycleActions->addStretch();
    cycleActions->addWidget(m_editCicloButton);

    cycleCard->addWidget(cycleTitle);
    cycleCard->addWidget(cycleHint);
    cycleCard->addLayout(cycleForm);
    cycleCard->addLayout(cycleActions);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    m_feedback->setObjectName(QStringLiteral("SuccessText"));
    m_feedback->setWordWrap(true);
    m_feedback->hide();

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
    layout->addWidget(card);
    layout->addWidget(cycleCard);
    layout->addWidget(m_status);
    layout->addWidget(m_feedback);
    layout->addLayout(buttonsRow);

    connect(m_editCicloButton, &QPushButton::clicked, this, &PacienteDetalleDialog::openEditCiclo);

    reload();
}

void PacienteDetalleDialog::reload()
{
    m_status->hide();
    m_api->get(QStringLiteral("/api/pacientes/%1").arg(m_dni),
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

    const QJsonValue ciclo = patient.value(QStringLiteral("ciclo_actual"));
    const QJsonValue serie = patient.value(QStringLiteral("serie_actual"));
    const QJsonValue donacion = patient.value(QStringLiteral("donacion_actual"));

    m_ciclo = counterRaw(ciclo);
    m_serie = counterRaw(serie);
    m_donacion = counterRaw(donacion);

    m_cicloValue->setText(counterText(ciclo));
    m_serieValue->setText(counterText(serie));
    m_donacionValue->setText(counterText(donacion));
}

void PacienteDetalleDialog::openEditCiclo()
{
    EditarCicloDialog dialog(m_api, m_dni, m_ciclo, m_serie, m_donacion, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    showFeedback(QStringLiteral("Ciclo actualizado."));
    reload();
}

void PacienteDetalleDialog::showFeedback(const QString &message)
{
    m_status->hide();
    m_feedback->setText(message);
    m_feedback->show();
}
