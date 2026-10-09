#pragma once

#include <QWidget>

class ApiClient;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * Staff authentication form. On success it stores the bearer token through the
 * shared ApiClient and emits `loggedIn` with the resolved username.
 *
 * Renders as a centered branded card on a SURFACE background.
 */
class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(ApiClient *api, QWidget *parent = nullptr);

    void setBusy(bool busy);
    void showError(const QString &message);
    void showInfo(const QString &message);

signals:
    void loggedIn(const QString &username);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onLoginClicked();

private:
    void setStatus(const QString &message, bool isError);

    ApiClient *m_api;
    QLineEdit *m_username;
    QLineEdit *m_password;
    QLineEdit *m_baseUrl;
    QLabel *m_status;
    QPushButton *m_loginButton;
    bool m_centered = false;
};
