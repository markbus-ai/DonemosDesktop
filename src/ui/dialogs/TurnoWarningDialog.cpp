#include "ui/dialogs/TurnoWarningDialog.h"

#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

TurnoWarningDialog::TurnoWarningDialog(const QString &message,
                                       const QStringList &warnings,
                                       QWidget *parent)
    : QDialog(parent)
    , m_motivo(new QLineEdit(this))
    , m_error(new QLabel(this))
{
    setWindowTitle(QStringLiteral("Advertencia"));
    setMinimumWidth(460);
    setModal(true);

    auto *title = new QLabel(QStringLiteral("El turno tiene advertencias"), this);
    title->setObjectName(QStringLiteral("PageTitle"));

    auto *messageLabel = new QLabel(
        message.isEmpty() ? QStringLiteral("Revise las advertencias antes de continuar.") : message,
        this);
    messageLabel->setObjectName(QStringLiteral("CardText"));
    messageLabel->setWordWrap(true);

    auto *card = new Card(this);
    card->addWidget(messageLabel);
    if (!warnings.isEmpty()) {
        auto *warningsLabel = new QLabel(QStringLiteral("- ") + warnings.join(QStringLiteral("\n- ")), this);
        warningsLabel->setObjectName(QStringLiteral("CardText"));
        warningsLabel->setWordWrap(true);
        card->addWidget(warningsLabel);
    }

    auto *motivoLabel = new QLabel(QStringLiteral("Motivo para forzar el turno"), this);
    motivoLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_motivo->setPlaceholderText(QStringLiteral("Indique el motivo"));

    m_error->setObjectName(QStringLiteral("ErrorText"));
    m_error->setWordWrap(true);
    m_error->hide();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Cancelar"));
    clearButtonIcon(buttons->button(QDialogButtonBox::Cancel));
    buttons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);

    auto *forceButton = buttons->addButton(QStringLiteral("Forzar turno"),
                                           QDialogButtonBox::AcceptRole);
    applyPrimaryVariant(forceButton);
    forceButton->setCursor(Qt::PointingHandCursor);
    forceButton->setDefault(true);

    connect(forceButton, &QPushButton::clicked, this, &TurnoWarningDialog::onForce);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_motivo, &QLineEdit::returnPressed, this, &TurnoWarningDialog::onForce);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(title);
    layout->addWidget(card);
    layout->addWidget(motivoLabel);
    layout->addWidget(m_motivo);
    layout->addWidget(m_error);
    layout->addWidget(buttons);

    m_motivo->setFocus();
}

QString TurnoWarningDialog::motivo() const
{
    return m_motivo->text().trimmed();
}

void TurnoWarningDialog::onForce()
{
    if (m_motivo->text().trimmed().isEmpty()) {
        m_error->setText(QStringLiteral("Para forzar el turno debe indicar un motivo."));
        m_error->show();
        m_motivo->setFocus();
        return;
    }

    accept();
}
