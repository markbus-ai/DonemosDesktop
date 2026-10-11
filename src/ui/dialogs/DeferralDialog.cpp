#include "ui/dialogs/DeferralDialog.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DeferralDialog::DeferralDialog(ApiClient *api, const QString &dni, Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(dni)
    , m_mode(mode)
    , m_motivo(new QComboBox(this))
    , m_desde(new QDateEdit(QDate::currentDate(), this))
    , m_permanente(new QCheckBox(QStringLiteral("Permanente (sin fecha de retorno)"), this))
    , m_useHasta(new QCheckBox(QStringLiteral("Definir fecha de retorno"), this))
    , m_hasta(new QDateEdit(QDate::currentDate().addMonths(6), this))
    , m_status(new QLabel(this))
    , m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    const bool isRestriccion = m_mode == Mode::Restriccion;
    const QString noun = isRestriccion ? QStringLiteral("restricción") : QStringLiteral("observación");

    setWindowTitle(QStringLiteral("Agregar %1").arg(noun));
    setMinimumWidth(460);

    auto *title = new QLabel(QStringLiteral("Agregar %1").arg(noun), this);
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("Paciente DNI %1").arg(dni), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));

    m_desde->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_desde->setCalendarPopup(true);
    m_hasta->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_hasta->setCalendarPopup(true);

    // Only a restriccion may be permanent; an observacion is always bounded in
    // time even when no explicit return date is informed.
    m_permanente->setVisible(isRestriccion);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    m_buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Guardar"));
    m_buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Cancelar"));
    applyPrimaryVariant(m_buttons->button(QDialogButtonBox::Ok));
    clearButtonIcon(m_buttons->button(QDialogButtonBox::Ok));
    clearButtonIcon(m_buttons->button(QDialogButtonBox::Cancel));
    m_buttons->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    m_buttons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    m_buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);

    auto *form = new QFormLayout;
    form->setSpacing(Theme::SpacingSm);
    form->addRow(QStringLiteral("Motivo"), m_motivo);
    form->addRow(QStringLiteral("Desde"), m_desde);
    form->addRow(QString(), m_permanente);
    form->addRow(QString(), m_useHasta);
    form->addRow(QStringLiteral("Hasta"), m_hasta);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_buttons);

    connect(m_permanente, &QCheckBox::toggled, this, &DeferralDialog::applyPermanentState);
    connect(m_useHasta, &QCheckBox::toggled, this, &DeferralDialog::applyPermanentState);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &DeferralDialog::onSubmit);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    applyPermanentState();
    loadMotivos();
}

void DeferralDialog::loadMotivos()
{
    m_api->get(QStringLiteral("/api/motivos"), [this](const ApiClient::Response &response) {
        if (!response.ok) {
            m_status->setText(QStringLiteral("No se pudieron cargar los motivos."));
            m_status->show();
            return;
        }

        m_motivo->clear();
        const QJsonArray motivos = response.json.array();
        for (const QJsonValue &value : motivos) {
            const QJsonObject motivo = value.toObject();
            m_motivo->addItem(motivo.value(QStringLiteral("nombre")).toString(),
                              motivo.value(QStringLiteral("id")).toInt());
        }
    });
}

void DeferralDialog::applyPermanentState()
{
    const bool permanent = m_permanente->isVisible() && m_permanente->isChecked();
    m_useHasta->setEnabled(!permanent);
    if (permanent)
        m_useHasta->setChecked(false);
    m_hasta->setEnabled(!permanent && m_useHasta->isChecked());
}

void DeferralDialog::setBusy(bool busy)
{
    m_buttons->setEnabled(!busy);
    m_motivo->setEnabled(!busy);
    m_desde->setEnabled(!busy);
    m_permanente->setEnabled(!busy);
    m_useHasta->setEnabled(!busy);
    m_hasta->setEnabled(!busy);
    m_buttons->button(QDialogButtonBox::Ok)
        ->setText(busy ? QStringLiteral("Guardando...") : QStringLiteral("Guardar"));
}

void DeferralDialog::onSubmit()
{
    const QVariant motivoId = m_motivo->currentData();
    if (!motivoId.isValid()) {
        m_status->setText(QStringLiteral("Seleccione un motivo."));
        m_status->show();
        return;
    }

    QJsonObject body{
        {QStringLiteral("motivo_id"), motivoId.toInt()},
        {QStringLiteral("desde"), m_desde->date().toString(Qt::ISODate)},
    };

    if (m_permanente->isVisible() && m_permanente->isChecked()) {
        body.insert(QStringLiteral("permanente"), true);
    } else if (m_useHasta->isChecked()) {
        if (m_hasta->date() < m_desde->date()) {
            m_status->setText(QStringLiteral("La fecha de retorno no puede ser anterior a la de inicio."));
            m_status->show();
            return;
        }
        body.insert(QStringLiteral("hasta"), m_hasta->date().toString(Qt::ISODate));
    }

    const QString path = m_mode == Mode::Restriccion
                             ? QStringLiteral("/api/pacientes/%1/restriccion").arg(m_dni)
                             : QStringLiteral("/api/pacientes/%1/observacion").arg(m_dni);

    setBusy(true);
    m_status->hide();

    m_api->post(path, body, [this](const ApiClient::Response &response) {
        setBusy(false);
        if (response.ok) {
            accept();
            return;
        }
        m_status->setText(response.errorMessage);
        m_status->show();
    });
}
