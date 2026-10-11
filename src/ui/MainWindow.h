#pragma once

#include <QMainWindow>

class ApiClient;
class AptitudPage;
class DonacionesPage;
class InicioPage;
class PacientesPage;
class ReportesPage;
class TurnosPage;
class QListWidget;
class QStackedWidget;

/**
 * Application shell: a branded sidebar driving a QStackedWidget of pages.
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
    QWidget *buildSidebar(const QString &username);

    QListWidget *m_sidebar;
    QStackedWidget *m_pages;
    InicioPage *m_inicioPage;
    PacientesPage *m_pacientesPage;
    TurnosPage *m_turnosPage;
    AptitudPage *m_aptitudPage;
    DonacionesPage *m_donacionesPage;
    ReportesPage *m_reportesPage;
};
