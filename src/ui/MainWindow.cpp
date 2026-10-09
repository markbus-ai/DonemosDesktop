#include "ui/MainWindow.h"

#include "api/ApiClient.h"
#include "ui/pages/InicioPage.h"
#include "ui/pages/PacientesPage.h"
#include "ui/pages/PlaceholderPage.h"

#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(ApiClient *api, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , m_sidebar(new QListWidget(this))
    , m_pages(new QStackedWidget(this))
    , m_pacientesPage(new PacientesPage(api, this))
{
    setWindowTitle(QStringLiteral("Donemos Desktop"));
    resize(1200, 760);

    // --- Toolbar: identity + logout -----------------------------------------
    auto *toolbar = addToolBar(QStringLiteral("Principal"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    auto *spacer = new QWidget(toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    auto *userLabel = new QLabel(QStringLiteral("Usuario: %1").arg(username), toolbar);
    userLabel->setContentsMargins(0, 0, 12, 0);
    toolbar->addWidget(userLabel);

    auto *logoutAction = toolbar->addAction(QStringLiteral("Cerrar sesión"));
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logoutRequested);

    // --- Sidebar navigation --------------------------------------------------
    m_sidebar->setFixedWidth(200);
    m_sidebar->addItem(QStringLiteral("Inicio"));
    m_sidebar->addItem(QStringLiteral("Pacientes"));
    m_sidebar->addItem(QStringLiteral("Turnos"));
    m_sidebar->addItem(QStringLiteral("Aptitud"));
    m_sidebar->addItem(QStringLiteral("Donaciones"));
    m_sidebar->addItem(QStringLiteral("Reportes"));

    // --- Page stack ----------------------------------------------------------
    m_pages->addWidget(new InicioPage(username, this));
    m_pages->addWidget(m_pacientesPage);
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Turnos"),
                                           QStringLiteral("Agenda de turnos de donación."), this));
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Aptitud"),
                                           QStringLiteral("Gestión de aptitud y diferimientos."), this));
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Donaciones"),
                                           QStringLiteral("Registro y seguimiento de donaciones."), this));
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Reportes"),
                                           QStringLiteral("Reportes operativos y estadísticas."), this));

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_sidebar);
    layout->addWidget(m_pages, 1);
    setCentralWidget(central);

    connect(m_sidebar, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);
    m_sidebar->setCurrentRow(0);
}

void MainWindow::onNavigationChanged(int row)
{
    if (row < 0)
        return;

    m_pages->setCurrentIndex(row);

    // Lazy-load the data-backed pages the first time they become visible.
    if (m_pages->currentWidget() == m_pacientesPage)
        m_pacientesPage->refresh();
}
