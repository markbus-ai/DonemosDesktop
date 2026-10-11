#include "ui/MainWindow.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/pages/InicioPage.h"
#include "ui/pages/PacientesPage.h"
#include "ui/pages/PlaceholderPage.h"
#include "ui/pages/TurnosPage.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(ApiClient *api, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , m_sidebar(new QListWidget(this))
    , m_pages(new QStackedWidget(this))
    , m_inicioPage(new InicioPage(username, this))
    , m_pacientesPage(new PacientesPage(api, this))
    , m_turnosPage(new TurnosPage(api, this))
{
    setWindowTitle(QStringLiteral("Donemos Desktop"));
    setMinimumSize(1040, 660);
    resize(1240, 780);

    auto *sidebar = buildSidebar(username);

    // --- Page stack ----------------------------------------------------------
    m_pages->setObjectName(QStringLiteral("ContentArea"));
    m_pages->addWidget(m_inicioPage);
    m_pages->addWidget(m_pacientesPage);
    m_pages->addWidget(m_turnosPage);
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Aptitud"),
                                           QStringLiteral("Consulte y gestione la aptitud y los "
                                                          "diferimientos de los donantes."),
                                           QStyle::SP_DialogApplyButton, this));
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Donaciones"),
                                           QStringLiteral("Registre y haga seguimiento de las "
                                                          "donaciones de sangre."),
                                           QStyle::SP_DialogSaveButton, this));
    m_pages->addWidget(new PlaceholderPage(QStringLiteral("Reportes"),
                                           QStringLiteral("Genere reportes operativos y "
                                                          "estadísticas del banco de sangre."),
                                           QStyle::SP_FileDialogInfoView, this));

    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("CentralArea"));
    central->setAttribute(Qt::WA_StyledBackground, true);
    m_pages->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(sidebar);
    layout->addWidget(m_pages, 1);
    setCentralWidget(central);

    connect(m_sidebar, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);
    connect(m_inicioPage, &InicioPage::navigateRequested, this, [this](int index) {
        m_sidebar->setCurrentRow(index);
    });
    m_sidebar->setCurrentRow(0);
}

QWidget *MainWindow::buildSidebar(const QString &username)
{
    auto *sidebar = new QWidget(this);
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(240);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(Theme::SpacingMd, Theme::SpacingLg, Theme::SpacingMd, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingSm);

    auto *brand = new QLabel(QStringLiteral("Donemos"), sidebar);
    brand->setObjectName(QStringLiteral("BrandTitle"));
    auto *brandSubtitle = new QLabel(QStringLiteral("Sistema de banco de sangre"), sidebar);
    brandSubtitle->setObjectName(QStringLiteral("BrandSubtitle"));
    layout->addWidget(brand);
    layout->addWidget(brandSubtitle);
    layout->addSpacing(Theme::SpacingLg);

    m_sidebar->setObjectName(QStringLiteral("SidebarNav"));
    m_sidebar->setFrameShape(QFrame::NoFrame);
    m_sidebar->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setSpacing(0);
    m_sidebar->setUniformItemSizes(true);

    const auto navIcon = [this](QStyle::StandardPixmap pixmap) {
        return style()->standardIcon(pixmap);
    };
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_DirHomeIcon), QStringLiteral("Inicio")));
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_FileDialogListView), QStringLiteral("Pacientes")));
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("Turnos")));
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_DialogApplyButton), QStringLiteral("Aptitud")));
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_DialogSaveButton), QStringLiteral("Donaciones")));
    m_sidebar->addItem(new QListWidgetItem(navIcon(QStyle::SP_FileDialogInfoView), QStringLiteral("Reportes")));

    layout->addWidget(m_sidebar);
    layout->addStretch();

    auto *divider = new QFrame(sidebar);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet(QStringLiteral("color: %1; background-color: %1; max-height: 1px;")
                               .arg(QString::fromLatin1(Theme::Divider)));
    layout->addWidget(divider);
    layout->addSpacing(Theme::SpacingSm);

    auto *userLabel = new QLabel(username, sidebar);
    userLabel->setObjectName(QStringLiteral("SidebarUser"));
    auto *roleLabel = new QLabel(QStringLiteral("Personal de staff"), sidebar);
    roleLabel->setObjectName(QStringLiteral("SidebarRole"));
    layout->addWidget(userLabel);
    layout->addWidget(roleLabel);
    layout->addSpacing(Theme::SpacingSm);

    auto *logout = new QPushButton(QStringLiteral("Cerrar sesión"), sidebar);
    logout->setCursor(Qt::PointingHandCursor);
    layout->addWidget(logout);

    connect(logout, &QPushButton::clicked, this, &MainWindow::logoutRequested);

    return sidebar;
}

void MainWindow::onNavigationChanged(int row)
{
    if (row < 0)
        return;

    m_pages->setCurrentIndex(row);

    // Lazy-load the data-backed pages the first time they become visible.
    if (m_pages->currentWidget() == m_pacientesPage)
        m_pacientesPage->refresh();
    else if (m_pages->currentWidget() == m_turnosPage)
        m_turnosPage->refresh();
}
