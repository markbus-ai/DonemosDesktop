#include "ui/LoginWindow.h"

#include "api/ApiClient.h"

#include <QFormLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

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
    setMinimumSize(420, 320);

    auto *title = new QLabel(QStringLiteral("Donemos"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 10);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel(QStringLiteral("Sistema de banco de sangre"), this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(QStringLiteral("color: #666;"));

    m_username->setPlaceholderText(QStringLiteral("Usuario"));
    m_password->setPlaceholderText(QStringLiteral("Contraseña"));
    m_password->setEchoMode(QLineEdit::Password);
    m_baseUrl->setText(m_api->baseUrl());
    m_baseUrl->setPlaceholderText(QStringLiteral("http://127.0.0.1:8000"));

    m_status->setWordWrap(true);
    m_status->setAlignment(Qt::AlignCenter);
    m_status->hide();

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Usuario"), m_username);
    form->addRow(QStringLiteral("Contraseña"), m_password);
    form->addRow(QStringLiteral("Servidor"), m_baseUrl);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 32, 40, 32);
    layout->setSpacing(14);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(10);
    layout->addLayout(form);
    layout->addWidget(m_status);
    layout->addWidget(m_loginButton);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_username, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
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
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b00020;")
                                    : QStringLiteral("color: #666;"));
    m_status->setVisible(!message.isEmpty());
}

void LoginWindow::onLoginClicked()
{
    const QString username = m_username->text().trimmed();
    const QString password = m_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError(QStringLiteral("Ingresá usuario y contraseña."));
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
            showError(QStringLiteral("Demasiados intentos fallidos. Esperá unos minutos y volvé a intentar."));
        } else {
            showError(response.errorMessage);
        }
    });
}
