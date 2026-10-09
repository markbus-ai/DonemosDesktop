#pragma once

#include <QMainWindow>

class ApiClient;
class PacientesPage;
class QListWidget;
class QStackedWidget;

/**
 * Application shell: sidebar navigation driving a QStackedWidget of pages.
 * Emits `logoutRequested` and lets the controller own the session lifecycle.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ApiClient *api, const QString &username, QWidget *parent = nullptr);

signals:
    void logoutRequested();

private slots:
    void onNavigationChanged(int row);

private:
    QListWidget *m_sidebar;
    QStackedWidget *m_pages;
    PacientesPage *m_pacientesPage;
};
