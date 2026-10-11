#include "ui/dialogs/AptitudDialog.h"

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

AptitudDialog::AptitudDialog(ApiClient *api, const QString &dni, QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(dni)
    , m_tipo(new QComboBox(this))
    , m_reasonBox(new QWidget(this))
    , m_motivo(new QComboBox(this))
    , m_desde(new QDateEdit(QDate::currentDate(), this))
    , m_permanente(new QCheckBox(QStringLiteral("Permanente (sin fecha de retorno)"), this))
    , m_useHasta(new QCheckBox(QStringLiteral("Definir fecha de retorno"), this))
    , m_hasta(new QDateEdit(QDate::currentDate().addMonths(6), this))
    , m_status(new QLabel(this))
    , m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(QStringLiteral("Cambiar aptitud"));
    setMinimumWidth(460);

    auto *title = new QLabel(QStringLiteral("Cambiar aptitud"), this);
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("Paciente DNI %1").arg(dni), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));

    // Apto by default, then the two deferral tiers that need a motivo.
    m_tipo->addItem(QStringLiteral("Apto"), QStringLiteral("APTO"));
    m_tipo->addItem(QStringLiteral("Apto con observación"), QStringLiteral("APTO_OBSERVACION"));
    m_tipo->addItem(QStringLiteral("No apto"), QStringLiteral("NO_APTO"));

    m_desde->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_desde->setCalendarPopup(true);
    m_hasta->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_hasta->setCalendarPopup(true);

    // Conditional block: only the non-APTO tiers need motivo + dates.
    auto *reasonForm = new QFormLayout(m_reasonBox);
    reasonForm->setContentsMargins(0, 0, 0, 0);
    reasonForm->setSpacing(Theme::SpacingSm);
    reasonForm->addRow(QStringLiteral("Motivo"), m_motivo);
    reasonForm->addRow(QStringLiteral("Desde"), m_desde);
    reasonForm->addRow(QString(), m_permanente);
    reasonForm->addRow(QString(), m_useHasta);
    reasonForm->addRow(QStringLiteral("Hasta"), m_hasta);

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
    form->addRow(QStringLiteral("Aptitud"), m_tipo);
    form->addRow(m_reasonBox);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_buttons);

    connect(m_tipo, &QComboBox::currentIndexChanged, this, &AptitudDialog::applyTipoState);
    connect(m_permanente, &QCheckBox::toggled, this, &AptitudDialog::applyTipoState);
    connect(m_useHasta, &QCheckBox::toggled, this, &AptitudDialog::applyTipoState);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &AptitudDialog::onSubmit);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    applyTipoState();
    loadMotivos();
}

void AptitudDialog::loadMotivos()
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

bool AptitudDialog::selectedTipoNeedsMotivo() const
{
    const QString tipo = m_tipo->currentData().toString();
    return tipo == QLatin1String("APTO_OBSERVACION") || tipo == QLatin1String("NO_APTO");
}

void AptitudDialog::applyTipoState()
{
    const bool needsReason = selectedTipoNeedsMotivo();
    m_reasonBox->setVisible(needsReason);

    // A permanent deferral has no return date, so hasta can never be combined.
    const bool permanent = m_permanente->isChecked();
    m_useHasta->setEnabled(!permanent);
    if (permanent)
        m_useHasta->setChecked(false);
    m_hasta->setEnabled(!permanent && m_useHasta->isChecked());
}

void AptitudDialog::setBusy(bool busy)
{
    m_buttons->setEnabled(!busy);
    m_tipo->setEnabled(!busy);
    m_reasonBox->setEnabled(!busy);
    m_buttons->button(QDialogButtonBox::Ok)
        ->setText(busy ? QStringLiteral("Guardando...") : QStringLiteral("Guardar"));
}

void AptitudDialog::onSubmit()
{
    const QString tipo = m_tipo->currentData().toString();
    QJsonObject body{{QStringLiteral("tipo"), tipo}};

    if (selectedTipoNeedsMotivo()) {
        const QVariant motivoId = m_motivo->currentData();
        if (!motivoId.isValid()) {
            m_status->setText(QStringLiteral("Seleccione un motivo."));
            m_status->show();
            return;
        }
        body.insert(QStringLiteral("motivo_id"), motivoId.toInt());
        body.insert(QStringLiteral("desde"), m_desde->date().toString(Qt::ISODate));

        if (m_permanente->isChecked()) {
            body.insert(QStringLiteral("permanente"), true);
        } else if (m_useHasta->isChecked()) {
            if (m_hasta->date() < m_desde->date()) {
                m_status->setText(QStringLiteral("La fecha de retorno no puede ser anterior a la de inicio."));
                m_status->show();
                return;
            }
            body.insert(QStringLiteral("hasta"), m_hasta->date().toString(Qt::ISODate));
        }
    }

    setBusy(true);
    m_status->hide();

    m_api->patch(QStringLiteral("/api/pacientes/%1/aptitud").arg(m_dni), body,
                 [this](const ApiClient::Response &response) {
                     setBusy(false);
                     if (response.ok) {
                         accept();
                         return;
                     }
                     m_status->setText(response.errorMessage);
                     m_status->show();
                 });
}
