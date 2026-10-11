#include "ui/dialogs/NuevoPacienteDialog.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QWidget *wrapWithError(QWidget *field, QLabel *error, QWidget *parent)
{
    auto *container = new QWidget(parent);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(field);
    error->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
                             .arg(QString::fromLatin1(Theme::Error)));
    error->hide();
    layout->addWidget(error);
    return container;
}

QString firstMessage(const QJsonObject &errors, const QString &field)
{
    const QJsonValue value = errors.value(field);
    if (value.isArray() && !value.toArray().isEmpty())
        return value.toArray().first().toString();
    if (value.isString())
        return value.toString();
    return {};
}
} // namespace

NuevoPacienteDialog::NuevoPacienteDialog(ApiClient *api, QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(new QLineEdit(this))
    , m_nombre(new QLineEdit(this))
    , m_apellido(new QLineEdit(this))
    , m_telefono(new QLineEdit(this))
    , m_sexo(new QComboBox(this))
    , m_altura(new QDoubleSpinBox(this))
    , m_status(new QLabel(this))
    , m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(QStringLiteral("Nuevo paciente"));
    setMinimumWidth(480);

    auto *title = new QLabel(QStringLiteral("Nuevo paciente"), this);
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("Registre los datos del donante"), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));

    m_sexo->addItem(QStringLiteral("Sin especificar"), QString());
    m_sexo->addItem(QStringLiteral("Femenino"), QStringLiteral("F"));
    m_sexo->addItem(QStringLiteral("Masculino"), QStringLiteral("M"));

    m_altura->setRange(0.0, 250.0);
    m_altura->setDecimals(1);
    m_altura->setSuffix(QStringLiteral(" cm"));
    m_altura->setSpecialValueText(QStringLiteral("Sin especificar"));

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
    form->addRow(QStringLiteral("DNI *"), wrapWithError(m_dni, new QLabel(this), this));
    form->addRow(QStringLiteral("Nombre *"), wrapWithError(m_nombre, new QLabel(this), this));
    form->addRow(QStringLiteral("Apellido *"), wrapWithError(m_apellido, new QLabel(this), this));
    form->addRow(QStringLiteral("Teléfono *"), wrapWithError(m_telefono, new QLabel(this), this));
    form->addRow(QStringLiteral("Sexo"), m_sexo);
    form->addRow(QStringLiteral("Altura"), m_altura);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &NuevoPacienteDialog::onSubmit);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NuevoPacienteDialog::onSubmit()
{
    clearErrors();

    bool valid = true;
    const auto require = [&valid](QLineEdit *field, QLabel *error) {
        if (field->text().trimmed().isEmpty()) {
            error->setText(QStringLiteral("Este campo es obligatorio."));
            error->show();
            valid = false;
        }
    };

    // Error labels live next to each field inside the wrapper containers.
    const auto errorFor = [this](QLineEdit *field) -> QLabel * {
        return field->parentWidget()->findChild<QLabel *>();
    };

    require(m_dni, errorFor(m_dni));
    require(m_nombre, errorFor(m_nombre));
    require(m_apellido, errorFor(m_apellido));
    require(m_telefono, errorFor(m_telefono));

    if (!valid)
        return;

    QJsonObject body{
        {QStringLiteral("dni"), m_dni->text().trimmed()},
        {QStringLiteral("nombre"), m_nombre->text().trimmed()},
        {QStringLiteral("apellido"), m_apellido->text().trimmed()},
        {QStringLiteral("telefono"), m_telefono->text().trimmed()},
    };

    const QString sexo = m_sexo->currentData().toString();
    if (!sexo.isEmpty())
        body.insert(QStringLiteral("sexo"), sexo);
    if (m_altura->value() > 0.0)
        body.insert(QStringLiteral("altura"), m_altura->value());

    setBusy(true);
    m_status->hide();

    m_api->post(QStringLiteral("/api/pacientes"), body, [this](const ApiClient::Response &response) {
        setBusy(false);

        if (response.ok) {
            accept();
            return;
        }

        if (response.status == 422 && !response.fieldErrors.isEmpty()) {
            showFieldErrors(response.fieldErrors);
            return;
        }

        m_status->setText(response.errorMessage);
        m_status->show();
    });
}

void NuevoPacienteDialog::setBusy(bool busy)
{
    m_buttons->setEnabled(!busy);
    m_buttons->button(QDialogButtonBox::Ok)
        ->setText(busy ? QStringLiteral("Guardando...") : QStringLiteral("Guardar"));
}

void NuevoPacienteDialog::clearErrors()
{
    m_status->clear();
    m_status->hide();

    const auto clear = [](QLineEdit *field) {
        if (QLabel *error = field->parentWidget()->findChild<QLabel *>()) {
            error->clear();
            error->hide();
        }
    };

    clear(m_dni);
    clear(m_nombre);
    clear(m_apellido);
    clear(m_telefono);
}

void NuevoPacienteDialog::showFieldErrors(const QJsonObject &errors)
{
    const auto apply = [&errors](QLineEdit *field, const QString &name) {
        const QString message = firstMessage(errors, name);
        if (message.isEmpty())
            return;
        if (QLabel *error = field->parentWidget()->findChild<QLabel *>()) {
            error->setText(message);
            error->show();
        }
    };

    apply(m_dni, QStringLiteral("dni"));
    apply(m_nombre, QStringLiteral("nombre"));
    apply(m_apellido, QStringLiteral("apellido"));
    apply(m_telefono, QStringLiteral("telefono"));
}
