#include "ui/LoginWindow.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"

#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QShowEvent>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

namespace {

/** Labeled input field: caption above the widget. */
QWidget *makeField(const QString &caption, QWidget *field, QWidget *parent)
{
    auto *container = new QWidget(parent);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::SpacingXs);

    auto *label = new QLabel(caption, container);
    label->setObjectName(QStringLiteral("FieldLabel"));
    layout->addWidget(label);
    layout->addWidget(field);
    return container;
}

} // namespace

LoginWindow::LoginWindow(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_username(new QLineEdit(this))
    , m_password(new QLineEdit(this))
    , m_baseUrl(new QLineEdit(this))
    , m_status(new QLabel(this))
    , m_loginButton(new QPushButton(QStringLiteral("Ingresar"), this))
{
    setWindowTitle(QStringLiteral("Donemos - Ingreso"));
    setObjectName(QStringLiteral("LoginRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(560, 640);

    // --- Branded card --------------------------------------------------------
    auto *card = new QWidget(this);
    card->setObjectName(QStringLiteral("Card"));
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setFixedWidth(420);

    auto *brand = new QLabel(QStringLiteral("Donemos"), card);
    brand->setObjectName(QStringLiteral("LoginBrand"));
    brand->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel(QStringLiteral("Sistema de banco de sangre"), card);
    subtitle->setObjectName(QStringLiteral("LoginSubtitle"));
    subtitle->setAlignment(Qt::AlignCenter);

    m_username->setPlaceholderText(QStringLiteral("Usuario"));
    m_password->setPlaceholderText(QStringLiteral("Contraseña"));
    m_password->setEchoMode(QLineEdit::Password);
    m_baseUrl->setText(m_api->baseUrl());
    m_baseUrl->setPlaceholderText(QStringLiteral("http://127.0.0.1:8000"));

    m_status->setObjectName(QStringLiteral("StatusText"));
    m_status->setWordWrap(true);
    m_status->setAlignment(Qt::AlignCenter);
    m_status->hide();

    m_loginButton->setProperty("variant", "primary");
    m_loginButton->setCursor(Qt::PointingHandCursor);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(Theme::SpacingXl, Theme::SpacingXl,
                                   Theme::SpacingXl, Theme::SpacingXl);
    cardLayout->setSpacing(Theme::SpacingMd);
    cardLayout->addWidget(brand);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(Theme::SpacingSm);
    cardLayout->addWidget(makeField(QStringLiteral("Usuario"), m_username, card));
    cardLayout->addWidget(makeField(QStringLiteral("Contraseña"), m_password, card));
    cardLayout->addWidget(makeField(QStringLiteral("Servidor"), m_baseUrl, card));
    cardLayout->addWidget(m_status);
    cardLayout->addSpacing(Theme::SpacingSm);
    cardLayout->addWidget(m_loginButton);

    auto *centering = new QHBoxLayout;
    centering->addStretch();
    centering->addWidget(card);
    centering->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->addStretch();
    layout->addLayout(centering);
    layout->addStretch();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_username, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_baseUrl, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);

    setTabOrder(m_username, m_password);
    setTabOrder(m_password, m_baseUrl);
    setTabOrder(m_baseUrl, m_loginButton);
    m_username->setFocus();
}

void LoginWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_centered)
        return;

    if (QScreen *screen = this->screen()) {
        const QRect available = screen->availableGeometry();
        move(available.center() - rect().center());
        m_centered = true;
    }
}

void LoginWindow::setBusy(bool busy)
{
    m_loginButton->setEnabled(!busy);
    m_username->setEnabled(!busy);
    m_password->setEnabled(!busy);
    m_baseUrl->setEnabled(!busy);
    m_loginButton->setText(busy ? QStringLiteral("Ingresando...") : QStringLiteral("Ingresar"));
}

void LoginWindow::showError(const QString &message)
{
    setStatus(message, true);
}

void LoginWindow::showInfo(const QString &message)
{
    setStatus(message, false);
}

void LoginWindow::setStatus(const QString &message, bool isError)
{
    m_status->setText(message);
    m_status->setProperty("tone", isError ? "error" : "info");
    m_status->style()->unpolish(m_status);
    m_status->style()->polish(m_status);
    m_status->setVisible(!message.isEmpty());
}

void LoginWindow::onLoginClicked()
{
    const QString username = m_username->text().trimmed();
    const QString password = m_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError(QStringLiteral("Ingrese usuario y contraseña."));
        return;
    }

    // Persist the server before attempting the request so retries use the same host.
    m_api->setBaseUrl(m_baseUrl->text());
    m_api->saveSettings();

    setBusy(true);
    setStatus(QStringLiteral("Verificando credenciales..."), false);

    const QJsonObject body{
        {QStringLiteral("username"), username},
        {QStringLiteral("password"), password},
    };

    m_api->post(QStringLiteral("/api/auth/login"), body, [this](const ApiClient::Response &response) {
        setBusy(false);

        if (response.ok) {
            const QJsonObject object = response.json.object();
            m_api->setToken(object.value(QStringLiteral("token")).toString());
            m_api->saveSettings();

            const QString resolved = object.value(QStringLiteral("user")).toObject()
                                         .value(QStringLiteral("username"))
                                         .toString();
            emit loggedIn(resolved.isEmpty() ? m_username->text().trimmed() : resolved);
            return;
        }

        if (response.status == 401) {
            showError(QStringLiteral("Usuario o contraseña incorrectos."));
        } else if (response.status == 429) {
            showError(QStringLiteral("Demasiados intentos fallidos. Espere unos minutos e intente de nuevo."));
        } else {
            showError(response.errorMessage);
        }
    });
}
