#include "ui/dialogs/NuevoTurnoDialog.h"

#include "api/ApiClient.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimeEdit>
#include <QUrl>
#include <QVBoxLayout>

NuevoTurnoDialog::NuevoTurnoDialog(ApiClient *api, QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(new QLineEdit(this))
    , m_searchButton(new QPushButton(QStringLiteral("Buscar"), this))
    , m_patientLabel(new QLabel(QStringLiteral("Sin paciente seleccionado"), this))
    , m_tipo(new QComboBox(this))
    , m_fecha(new QDateEdit(QDate::currentDate(), this))
    , m_hora(new QTimeEdit(QTime::currentTime(), this))
    , m_status(new QLabel(this))
    , m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(QStringLiteral("Nuevo turno"));
    setMinimumWidth(430);

    m_dni->setPlaceholderText(QStringLiteral("DNI del paciente"));
    m_patientLabel->setStyleSheet(QStringLiteral("color: #666;"));

    m_fecha->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_fecha->setCalendarPopup(true);
    m_hora->setDisplayFormat(QStringLiteral("HH:mm"));

    m_status->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_status->setWordWrap(true);
    m_status->hide();

    m_buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Crear turno"));
    m_buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Cancelar"));
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    m_buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);

    auto *patientRow = new QHBoxLayout;
    patientRow->addWidget(m_dni, 1);
    patientRow->addWidget(m_searchButton);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Paciente"), patientRow);
    form->addRow(QString(), m_patientLabel);
    form->addRow(QStringLiteral("Tipo de donación"), m_tipo);
    form->addRow(QStringLiteral("Fecha"), m_fecha);
    form->addRow(QStringLiteral("Hora"), m_hora);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_buttons);

    connect(m_searchButton, &QPushButton::clicked, this, &NuevoTurnoDialog::onSearchPatient);
    connect(m_dni, &QLineEdit::returnPressed, this, &NuevoTurnoDialog::onSearchPatient);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &NuevoTurnoDialog::onSubmit);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    loadTipos();
}

void NuevoTurnoDialog::loadTipos()
{
    m_api->get(QStringLiteral("/api/tipos-donacion"), [this](const ApiClient::Response &response) {
        if (!response.ok) {
            m_status->setText(QStringLiteral("No se pudieron cargar los tipos de donación."));
            m_status->show();
            return;
        }

        m_tipo->clear();
        const QJsonArray tipos = response.json.array();
        for (const QJsonValue &value : tipos) {
            const QJsonObject tipo = value.toObject();
            m_tipo->addItem(tipo.value(QStringLiteral("nombre")).toString(),
                            tipo.value(QStringLiteral("id")).toInt());
        }
    });
}

void NuevoTurnoDialog::onSearchPatient()
{
    const QString dni = m_dni->text().trimmed();
    if (dni.isEmpty()) {
        m_status->setText(QStringLiteral("Ingresá un DNI para buscar."));
        m_status->show();
        return;
    }

    m_status->hide();
    m_searchButton->setEnabled(false);
    m_pacienteId = 0;
    m_patientLabel->setText(QStringLiteral("Buscando..."));

    const QString path = QStringLiteral("/api/pacientes?per_page=5&search=%1")
                             .arg(QString::fromUtf8(QUrl::toPercentEncoding(dni)));

    m_api->get(path, [this, dni](const ApiClient::Response &response) {
        m_searchButton->setEnabled(true);

        if (!response.ok) {
            m_patientLabel->setText(QStringLiteral("Sin paciente seleccionado"));
            m_status->setText(response.errorMessage);
            m_status->show();
            return;
        }

        const QJsonArray rows = response.json.object().value(QStringLiteral("data")).toArray();
        for (const QJsonValue &value : rows) {
            const QJsonObject patient = value.toObject();
            if (patient.value(QStringLiteral("dni")).toString() == dni) {
                m_pacienteId = patient.value(QStringLiteral("id")).toInt();
                m_patientLabel->setText(QStringLiteral("%1 %2 (%3)")
                                            .arg(patient.value(QStringLiteral("nombre")).toString(),
                                                 patient.value(QStringLiteral("apellido")).toString(),
                                                 dni));
                m_status->hide();
                return;
            }
        }

        m_patientLabel->setText(QStringLiteral("Sin paciente seleccionado"));
        m_status->setText(QStringLiteral("No se encontró un paciente con ese DNI."));
        m_status->show();
    });
}

void NuevoTurnoDialog::onSubmit()
{
    if (m_pacienteId <= 0) {
        m_status->setText(QStringLiteral("Buscá y seleccioná un paciente antes de crear el turno."));
        m_status->show();
        return;
    }
    submit(false, QString());
}

void NuevoTurnoDialog::submit(bool force, const QString &motivo)
{
    QJsonObject body{
        {QStringLiteral("paciente_id"), m_pacienteId},
        {QStringLiteral("fecha"), m_fecha->date().toString(Qt::ISODate)},
        {QStringLiteral("hora"), m_hora->time().toString(QStringLiteral("HH:mm"))},
    };

    const QVariant tipoId = m_tipo->currentData();
    if (tipoId.isValid())
        body.insert(QStringLiteral("tipo_id"), tipoId.toInt());

    if (force) {
        body.insert(QStringLiteral("forzar"), true);
        body.insert(QStringLiteral("motivo"), motivo);
    }

    setBusy(true);
    m_status->hide();

    m_api->post(QStringLiteral("/api/turnos"), body, [this](const ApiClient::Response &response) {
        setBusy(false);

        if (response.ok) {
            accept();
            return;
        }

        if (response.status == 409) {
            const QJsonObject object = response.json.object();
            QStringList warnings;
            const QJsonArray warningItems = object.value(QStringLiteral("warnings")).toArray();
            for (const QJsonValue &value : warningItems)
                warnings << value.toObject().value(QStringLiteral("message")).toString();

            handleWarning(object.value(QStringLiteral("message")).toString(), warnings);
            return;
        }

        if (response.status == 422 && !response.fieldErrors.isEmpty()) {
            m_status->setText(response.errorMessage);
            m_status->show();
            return;
        }

        m_status->setText(response.errorMessage);
        m_status->show();
    });
}

void NuevoTurnoDialog::handleWarning(const QString &message, const QStringList &warnings)
{
    QString text = message.isEmpty()
        ? QStringLiteral("El turno tiene advertencias.")
        : message;
    if (!warnings.isEmpty())
        text += QStringLiteral("\n\n- ") + warnings.join(QStringLiteral("\n- "));

    QMessageBox box(this);
    box.setWindowTitle(QStringLiteral("Advertencia"));
    box.setIcon(QMessageBox::Warning);
    box.setText(text);
    QPushButton *forceButton = box.addButton(QStringLiteral("Forzar turno"), QMessageBox::AcceptRole);
    box.addButton(QStringLiteral("Cancelar"), QMessageBox::RejectRole);
    box.exec();

    if (box.clickedButton() != forceButton)
        return;

    bool accepted = false;
    const QString motivo = QInputDialog::getText(this,
                                                 QStringLiteral("Motivo"),
                                                 QStringLiteral("Motivo para forzar el turno:"),
                                                 QLineEdit::Normal,
                                                 QString(),
                                                 &accepted);
    if (!accepted || motivo.trimmed().isEmpty()) {
        m_status->setText(QStringLiteral("Para forzar el turno tenés que indicar un motivo."));
        m_status->show();
        return;
    }

    submit(true, motivo.trimmed());
}

void NuevoTurnoDialog::setBusy(bool busy)
{
    m_buttons->setEnabled(!busy);
    m_searchButton->setEnabled(!busy);
    m_buttons->button(QDialogButtonBox::Ok)
        ->setText(busy ? QStringLiteral("Creando...") : QStringLiteral("Crear turno"));
}
