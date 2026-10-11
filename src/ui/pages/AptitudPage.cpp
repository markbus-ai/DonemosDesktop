#include "ui/pages/AptitudPage.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"
#include "ui/components/PageHeader.h"
#include "ui/components/StatusChip.h"
#include "ui/dialogs/AptitudDialog.h"
#include "ui/dialogs/DeferralDialog.h"

#include <QDate>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

namespace {

/** ISO date (yyyy-MM-dd) rendered as dd/MM/yyyy, empty-safe. */
QString prettyDate(const QString &iso)
{
    const QDate date = QDate::fromString(iso, Qt::ISODate);
    return date.isValid() ? date.toString(QStringLiteral("dd/MM/yyyy")) : iso;
}

} // namespace

AptitudPage::AptitudPage(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_dni(new QLineEdit(this))
    , m_searchButton(new QPushButton(QStringLiteral("Buscar"), this))
    , m_status(new QLabel(this))
    , m_feedback(new QLabel(this))
    , m_placeholder(new QLabel(this))
    , m_patientCard(new Card(this))
    , m_nameLabel(new QLabel(m_patientCard))
    , m_metaLabel(new QLabel(m_patientCard))
    , m_aptitudChip(StatusChip::make(QString(), QStringLiteral("muted"), m_patientCard))
    , m_deferralChip(StatusChip::make(QString(), QStringLiteral("muted"), m_patientCard))
    , m_plateletChip(StatusChip::make(QString(), QStringLiteral("muted"), m_patientCard))
    , m_plateletInfo(new QLabel(m_patientCard))
    , m_cicloInfo(new QLabel(m_patientCard))
    , m_changeAptitudButton(new QPushButton(QStringLiteral("Cambiar aptitud"), m_patientCard))
    , m_addObservacionButton(new QPushButton(QStringLiteral("Agregar observación"), m_patientCard))
    , m_removeObservacionButton(new QPushButton(QStringLiteral("Quitar observación"), m_patientCard))
    , m_addRestriccionButton(new QPushButton(QStringLiteral("Agregar restricción"), m_patientCard))
    , m_removeRestriccionButton(new QPushButton(QStringLiteral("Quitar restricción"), m_patientCard))
    , m_enablePlateletsButton(new QPushButton(QStringLiteral("Habilitar plaquetas (6 meses)"), m_patientCard))
    , m_disablePlateletsButton(new QPushButton(QStringLiteral("Deshabilitar plaquetas"), m_patientCard))
{
    // --- Header --------------------------------------------------------------
    auto *header = new PageHeader(QStringLiteral("Aptitud"),
                                  QStringLiteral("Aptitud, diferimientos y plaquetas del donante"), this);

    // --- Patient lookup ------------------------------------------------------
    m_dni->setPlaceholderText(QStringLiteral("DNI del donante"));
    m_dni->setClearButtonEnabled(true);
    m_searchButton->setCursor(Qt::PointingHandCursor);

    auto *searchCard = new Card(this);
    auto *searchRow = new QHBoxLayout;
    searchRow->setSpacing(Theme::SpacingSm);
    searchRow->addWidget(m_dni, 1);
    searchRow->addWidget(m_searchButton);
    searchCard->addLayout(searchRow);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    m_feedback->setObjectName(QStringLiteral("SuccessText"));
    m_feedback->setWordWrap(true);
    m_feedback->hide();

    // --- Empty state ---------------------------------------------------------
    m_placeholder->setObjectName(QStringLiteral("EmptyState"));
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setWordWrap(true);
    m_placeholder->setText(QStringLiteral("Ingrese un DNI para consultar la aptitud del donante."));

    // --- Patient card --------------------------------------------------------
    m_patientCard->hide();
    m_nameLabel->setObjectName(QStringLiteral("CardTitle"));
    m_metaLabel->setObjectName(QStringLiteral("PageSubtitle"));
    m_metaLabel->setWordWrap(true);

    auto *nameRow = new QHBoxLayout;
    nameRow->setSpacing(Theme::SpacingMd);
    nameRow->addWidget(m_nameLabel);
    nameRow->addStretch();
    nameRow->addWidget(m_aptitudChip);

    auto *deferralTitle = new QLabel(QStringLiteral("Diferimiento"), m_patientCard);
    deferralTitle->setObjectName(QStringLiteral("FieldLabel"));
    auto *plateletTitle = new QLabel(QStringLiteral("Plaquetas"), m_patientCard);
    plateletTitle->setObjectName(QStringLiteral("FieldLabel"));
    m_plateletInfo->setObjectName(QStringLiteral("InfoText"));

    auto *cycleTitle = new QLabel(QStringLiteral("Ciclo de donaciones"), m_patientCard);
    cycleTitle->setObjectName(QStringLiteral("FieldLabel"));
    m_cicloInfo->setObjectName(QStringLiteral("InfoText"));

    auto *stateGrid = new QGridLayout;
    stateGrid->setHorizontalSpacing(Theme::SpacingMd);
    stateGrid->setVerticalSpacing(Theme::SpacingSm);
    stateGrid->addWidget(deferralTitle, 0, 0);
    stateGrid->addWidget(m_deferralChip, 0, 1, Qt::AlignLeft);
    stateGrid->addWidget(plateletTitle, 1, 0);
    stateGrid->addWidget(m_plateletChip, 1, 1, Qt::AlignLeft);
    stateGrid->addWidget(m_plateletInfo, 1, 2, Qt::AlignLeft);
    stateGrid->addWidget(cycleTitle, 2, 0);
    stateGrid->addWidget(m_cicloInfo, 2, 1, 1, 2, Qt::AlignLeft);
    stateGrid->setColumnStretch(3, 1);

    // --- Actions -------------------------------------------------------------
    m_changeAptitudButton->setProperty("variant", "primary");
    const QList<QPushButton *> actionButtons{
        m_changeAptitudButton, m_addObservacionButton, m_removeObservacionButton,
        m_addRestriccionButton, m_removeRestriccionButton,
        m_enablePlateletsButton, m_disablePlateletsButton,
    };
    for (QPushButton *button : actionButtons)
        button->setCursor(Qt::PointingHandCursor);

    auto *actionsGrid = new QGridLayout;
    actionsGrid->setHorizontalSpacing(Theme::SpacingSm);
    actionsGrid->setVerticalSpacing(Theme::SpacingSm);
    actionsGrid->addWidget(m_changeAptitudButton, 0, 0);
    actionsGrid->addWidget(m_addObservacionButton, 0, 1);
    actionsGrid->addWidget(m_removeObservacionButton, 0, 2);
    actionsGrid->addWidget(m_addRestriccionButton, 1, 0);
    actionsGrid->addWidget(m_removeRestriccionButton, 1, 1);
    actionsGrid->addWidget(m_enablePlateletsButton, 2, 0);
    actionsGrid->addWidget(m_disablePlateletsButton, 2, 1);
    actionsGrid->setColumnStretch(3, 1);

    m_patientCard->addLayout(nameRow);
    m_patientCard->addWidget(m_metaLabel);
    m_patientCard->addLayout(stateGrid);
    m_patientCard->body()->addSpacing(Theme::SpacingSm);
    m_patientCard->addLayout(actionsGrid);
    m_patientCard->body()->addStretch(1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addWidget(searchCard);
    layout->addWidget(m_status);
    layout->addWidget(m_feedback);
    layout->addWidget(m_placeholder, 1);
    layout->addWidget(m_patientCard, 1);

    connect(m_searchButton, &QPushButton::clicked, this, &AptitudPage::onSearch);
    connect(m_dni, &QLineEdit::returnPressed, this, &AptitudPage::onSearch);
    connect(m_changeAptitudButton, &QPushButton::clicked, this, &AptitudPage::onChangeAptitud);
    connect(m_addObservacionButton, &QPushButton::clicked, this, &AptitudPage::onAddObservacion);
    connect(m_removeObservacionButton, &QPushButton::clicked, this, &AptitudPage::onRemoveObservacion);
    connect(m_addRestriccionButton, &QPushButton::clicked, this, &AptitudPage::onAddRestriccion);
    connect(m_removeRestriccionButton, &QPushButton::clicked, this, &AptitudPage::onRemoveRestriccion);
    connect(m_enablePlateletsButton, &QPushButton::clicked, this, &AptitudPage::onEnablePlatelets);
    connect(m_disablePlateletsButton, &QPushButton::clicked, this, &AptitudPage::onDisablePlatelets);
}

void AptitudPage::refresh()
{
    if (!m_currentDni.isEmpty())
        loadPatient(m_currentDni);
}

void AptitudPage::onSearch()
{
    const QString dni = m_dni->text().trimmed();
    if (dni.isEmpty()) {
        showError(QStringLiteral("Ingrese un DNI para buscar."));
        return;
    }
    loadPatient(dni);
}

void AptitudPage::loadPatient(const QString &dni)
{
    m_currentDni = dni;
    setBusy(true);
    m_status->hide();

    const QString path = QStringLiteral("/api/pacientes?per_page=50&search=%1")
                             .arg(QString::fromUtf8(QUrl::toPercentEncoding(dni)));

    m_api->get(path, [this, dni](const ApiClient::Response &response) {
        setBusy(false);

        if (!response.ok) {
            showError(response.errorMessage);
            return;
        }

        const QJsonArray rows = response.json.object().value(QStringLiteral("data")).toArray();
        for (const QJsonValue &value : rows) {
            const QJsonObject patient = value.toObject();
            if (patient.value(QStringLiteral("dni")).toString() == dni) {
                m_patient = patient;
                m_placeholder->hide();
                m_patientCard->show();
                renderPatient();
                loadPlatelets();
                return;
            }
        }

        m_patient = QJsonObject{};
        m_patientCard->hide();
        m_placeholder->setText(QStringLiteral("No se encontró un donante con ese DNI."));
        m_placeholder->show();
    });
}

void AptitudPage::renderPatient()
{
    const QString nombre = m_patient.value(QStringLiteral("nombre")).toString();
    const QString apellido = m_patient.value(QStringLiteral("apellido")).toString();
    m_nameLabel->setText(QStringLiteral("%1 %2").arg(nombre, apellido).trimmed());

    QStringList meta;
    meta << QStringLiteral("DNI %1").arg(m_patient.value(QStringLiteral("dni")).toString());
    const QString sexo = m_patient.value(QStringLiteral("sexo")).toString();
    if (!sexo.isEmpty())
        meta << QStringLiteral("Sexo: %1").arg(sexo);
    const QString telefono = m_patient.value(QStringLiteral("telefono")).toString();
    if (!telefono.isEmpty())
        meta << QStringLiteral("Teléfono: %1").arg(telefono);
    m_metaLabel->setText(meta.join(QStringLiteral("  ·  ")));

    const QString aptitud = m_patient.value(QStringLiteral("aptitud")).toObject()
                                .value(QStringLiteral("tipo")).toString();
    StatusChip::update(m_aptitudChip, aptitudLabel(aptitud), StatusChip::aptitudKind(aptitud));

    m_hasActiveDeferral = m_patient.value(QStringLiteral("activeDeferral")).toBool();
    StatusChip::update(m_deferralChip,
                       m_hasActiveDeferral ? QStringLiteral("Diferimiento activo")
                                           : QStringLiteral("Sin diferimiento"),
                       m_hasActiveDeferral ? QStringLiteral("warning") : QStringLiteral("muted"));
    m_removeRestriccionButton->setEnabled(m_hasActiveDeferral);

    const auto counterOrDash = [this](const QString &key) -> QString {
        const QJsonValue value = m_patient.value(key);
        if (value.isNull() || value.isUndefined())
            return QStringLiteral("—");
        return QString::number(value.toVariant().toLongLong());
    };
    m_cicloInfo->setText(QStringLiteral("Ciclo %1  ·  Serie %2  ·  Donación %3")
                             .arg(counterOrDash(QStringLiteral("ciclo_actual")),
                                  counterOrDash(QStringLiteral("serie_actual")),
                                  counterOrDash(QStringLiteral("donacion_actual"))));
}

void AptitudPage::loadPlatelets()
{
    const QString path = QStringLiteral("/api/pacientes/%1/habilitacion-plaquetas").arg(m_currentDni);
    m_api->get(path, [this](const ApiClient::Response &response) {
        if (!response.ok) {
            StatusChip::update(m_plateletChip, QStringLiteral("Sin datos"), QStringLiteral("muted"));
            m_plateletInfo->setText(response.errorMessage);
            return;
        }

        const QString today = QDate::currentDate().toString(Qt::ISODate);
        bool active = false;
        QString hasta;
        const QJsonArray rows = response.json.array();
        for (const QJsonValue &value : rows) {
            const QJsonObject row = value.toObject();
            const QString desde = row.value(QStringLiteral("desde")).toString();
            const QString expiry = row.value(QStringLiteral("hasta")).toString();
            if (!desde.isEmpty() && desde <= today && (expiry.isEmpty() || expiry > today)) {
                active = true;
                hasta = expiry;
                break;
            }
        }

        m_plateletsEnabled = active;
        StatusChip::update(m_plateletChip,
                           active ? QStringLiteral("Habilitadas") : QStringLiteral("Deshabilitadas"),
                           active ? QStringLiteral("success") : QStringLiteral("muted"));
        m_plateletInfo->setText(active && !hasta.isEmpty()
                                    ? QStringLiteral("Vigente hasta %1").arg(prettyDate(hasta))
                                    : QString());
        m_enablePlateletsButton->setEnabled(!active);
        m_disablePlateletsButton->setEnabled(active);
    });
}

void AptitudPage::onChangeAptitud()
{
    AptitudDialog dialog(m_api, m_currentDni, this);
    if (dialog.exec() == QDialog::Accepted) {
        showFeedback(QStringLiteral("Aptitud actualizada."));
        loadPatient(m_currentDni);
    }
}

void AptitudPage::onAddObservacion()
{
    DeferralDialog dialog(m_api, m_currentDni, DeferralDialog::Mode::Observacion, this);
    if (dialog.exec() == QDialog::Accepted) {
        showFeedback(QStringLiteral("Observación registrada."));
        loadPatient(m_currentDni);
    }
}

void AptitudPage::onAddRestriccion()
{
    DeferralDialog dialog(m_api, m_currentDni, DeferralDialog::Mode::Restriccion, this);
    if (dialog.exec() == QDialog::Accepted) {
        showFeedback(QStringLiteral("Restricción registrada."));
        loadPatient(m_currentDni);
    }
}

void AptitudPage::onRemoveObservacion()
{
    confirmDelete(QStringLiteral("¿Quitar la observación activa de este donante?"),
                  QStringLiteral("/api/pacientes/%1/observacion").arg(m_currentDni),
                  QStringLiteral("Observación quitada."));
}

void AptitudPage::onRemoveRestriccion()
{
    confirmDelete(QStringLiteral("¿Quitar el diferimiento activo de este donante?"),
                  QStringLiteral("/api/pacientes/%1/restriccion").arg(m_currentDni),
                  QStringLiteral("Diferimiento quitado."));
}

void AptitudPage::onEnablePlatelets()
{
    setBusy(true);
    m_status->hide();

    m_api->post(QStringLiteral("/api/pacientes/%1/habilitacion-plaquetas").arg(m_currentDni),
                QJsonObject{}, [this](const ApiClient::Response &response) {
                    setBusy(false);
                    if (!response.ok) {
                        showError(response.errorMessage);
                        return;
                    }
                    showFeedback(QStringLiteral("Plaquetas habilitadas por 6 meses."));
                    loadPatient(m_currentDni);
                });
}

void AptitudPage::onDisablePlatelets()
{
    confirmDelete(QStringLiteral("¿Deshabilitar la habilitación de plaquetas?"),
                  QStringLiteral("/api/pacientes/%1/habilitacion-plaquetas").arg(m_currentDni),
                  QStringLiteral("Plaquetas deshabilitadas."));
}

void AptitudPage::confirmDelete(const QString &question, const QString &path, const QString &success)
{
    const auto answer = QMessageBox::question(this, QStringLiteral("Confirmar"), question,
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    setBusy(true);
    m_status->hide();

    m_api->del(path, [this, success](const ApiClient::Response &response) {
        setBusy(false);
        if (!response.ok) {
            showError(response.errorMessage);
            return;
        }
        showFeedback(success);
        loadPatient(m_currentDni);
    });
}

void AptitudPage::setBusy(bool busy)
{
    m_busy = busy;
    m_dni->setEnabled(!busy);
    m_searchButton->setEnabled(!busy);
    m_changeAptitudButton->setEnabled(!busy);
    m_addObservacionButton->setEnabled(!busy);
    m_removeObservacionButton->setEnabled(!busy);
    m_addRestriccionButton->setEnabled(!busy);
    m_removeRestriccionButton->setEnabled(!busy && m_hasActiveDeferral);
    m_enablePlateletsButton->setEnabled(!busy && !m_plateletsEnabled);
    m_disablePlateletsButton->setEnabled(!busy && m_plateletsEnabled);
    setCursor(busy ? Qt::BusyCursor : Qt::ArrowCursor);
}

void AptitudPage::showError(const QString &message)
{
    if (message.isEmpty()) {
        m_status->hide();
        return;
    }
    m_status->setText(message);
    m_status->show();
}

void AptitudPage::showFeedback(const QString &message)
{
    m_status->hide();
    m_feedback->setText(message);
    m_feedback->show();
}
