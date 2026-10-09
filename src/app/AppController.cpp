#include "app/AppController.h"

#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"

#include <QJsonObject>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    connect(&m_api, &ApiClient::sessionExpired, this, &AppController::onSessionExpired);
}

void AppController::start()
{
    m_api.loadSettings();

    showLogin();

    if (!m_api.hasToken())
        return;

    // A token is stored: validate it before showing the main shell.
    m_login->setBusy(true);
    m_login->showInfo(QStringLiteral("Validando sesión..."));

    m_api.get(QStringLiteral("/api/auth/me"), [this](const ApiClient::Response &response) {
        if (response.ok) {
            const QString username = response.json.object().value(QStringLiteral("username")).toString();
            onLoggedIn(username);
            return;
        }

        if (m_login) {
            m_login->setBusy(false);
            m_login->showError(QStringLiteral("La sesión guardada expiró. Inicie sesión de nuevo."));
        }
    });
}

void AppController::showLogin()
{
    if (!m_login) {
        m_login = new LoginWindow(&m_api);
        connect(m_login, &LoginWindow::loggedIn, this, &AppController::onLoggedIn);
    }
    m_login->show();
    m_login->raise();
    m_login->activateWindow();
}

void AppController::onLoggedIn(const QString &username)
{
    if (m_login) {
        m_login->hide();
        m_login->deleteLater();
        m_login = nullptr;
    }

    m_main = new MainWindow(&m_api, username);
    connect(m_main, &MainWindow::logoutRequested, this, &AppController::onLogoutRequested);
    m_main->show();
}

void AppController::onLogoutRequested()
{
    // Best-effort server logout; the local session is cleared regardless.
    m_api.post(QStringLiteral("/api/auth/logout"), {}, [](const ApiClient::Response &) {});
    m_api.clearSession();

    if (m_main) {
        m_main->hide();
        m_main->deleteLater();
        m_main = nullptr;
    }

    showLogin();
    if (m_login)
        m_login->showInfo(QStringLiteral("Sesión cerrada."));
}

void AppController::onSessionExpired()
{
    // Only react when an active main window lost its session; the startup
    // validation path is already handled by the `/me` callback.
    if (!m_main)
        return;

    m_main->hide();
    m_main->deleteLater();
    m_main = nullptr;

    showLogin();
    if (m_login)
        m_login->showError(QStringLiteral("Tu sesión expiró. Inicie sesión de nuevo."));
}
