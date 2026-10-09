#pragma once

#include <QObject>
#include <QString>

#include "api/ApiClient.h"

class LoginWindow;
class MainWindow;

/**
 * Owns the session lifecycle: restores a stored token, validates it against
 * `/api/auth/me`, and swaps between the login window and the main window.
 */
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    void start();

private slots:
    void onLoggedIn(const QString &username);
    void onLogoutRequested();
    void onSessionExpired();

private:
    void showLogin();

    ApiClient m_api;
    LoginWindow *m_login = nullptr;
    MainWindow *m_main = nullptr;
};
