#include "ui/dialogs/EditarCicloDialog.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

/** Highest counter we allow staff to type; purely a sanity ceiling. */
constexpr int kMaxCounter = 999999;

/** 0 means "not set" in the spin box, rendered as an em dash. */
QSpinBox *makeCounter(QWidget *parent, qint64 value)
{
    auto *box = new QSpinBox(parent);
    box->setRange(0, kMaxCounter);
    box->setSpecialValueText(QStringLiteral("—"));
    box->setValue(static_cast<int>(value));
    return box;
}

} // namespace

EditarCicloDialog::EditarCicloDialog(ApiClient *api,
                                     const QString &dni,
                                     qint64 ciclo,
                                     qint64 serie,
                                     qint64 donacion,
                                     QWidget *parent)
    : QDialog(parent)
    , m_api(api)
    , m_dni(dni)
    , m_ciclo(makeCounter(this, ciclo))
    , m_serie(makeCounter(this, serie))
    , m_donacion(makeCounter(this, donacion))
    , m_status(new QLabel(this))
    , m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(QStringLiteral("Editar ciclo"));
    setMinimumWidth(480);

    auto *title = new QLabel(QStringLiteral("Editar ciclo de donaciones"), this);
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("DNI %1").arg(dni), this);
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));

    auto *hint = new QLabel(
        QStringLiteral("Valores manuales: el personal los completa a mano y puede "
                       "reiniciarlos o empezar un ciclo o serie nuevo cuando quiera. "
                       "Ponga 0 en un campo para dejarlo sin definir (se muestra como —)."),
        this);
    hint->setObjectName(QStringLiteral("InfoText"));
    hint->setWordWrap(true);

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
    form->addRow(QStringLiteral("Ciclo"), m_ciclo);
    form->addRow(QStringLiteral("Serie"), m_serie);
    form->addRow(QStringLiteral("Donación"), m_donacion);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(hint);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &EditarCicloDialog::onSubmit);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void EditarCicloDialog::onSubmit()
{
    // Always send all three keys: a 0 in the spin box clears the field (null).
    const auto counter = [](QSpinBox *box) -> QJsonValue {
        return box->value() > 0 ? QJsonValue(box->value()) : QJsonValue(QJsonValue::Null);
    };

    QJsonObject body{
        {QStringLiteral("ciclo_actual"), counter(m_ciclo)},
        {QStringLiteral("serie_actual"), counter(m_serie)},
        {QStringLiteral("donacion_actual"), counter(m_donacion)},
    };

    setBusy(true);
    m_status->hide();

    m_api->patch(QStringLiteral("/api/pacientes/%1").arg(m_dni), body,
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

void EditarCicloDialog::setBusy(bool busy)
{
    m_buttons->setEnabled(!busy);
    m_ciclo->setEnabled(!busy);
    m_serie->setEnabled(!busy);
    m_donacion->setEnabled(!busy);
    m_buttons->button(QDialogButtonBox::Ok)
        ->setText(busy ? QStringLiteral("Guardando...") : QStringLiteral("Guardar"));
}
