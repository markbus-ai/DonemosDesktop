#include "ui/pages/PacientesPage.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/UiUtils.h"
#include "ui/components/Card.h"
#include "ui/components/DataTable.h"
#include "ui/components/PageHeader.h"
#include "ui/components/StatusChip.h"
#include "ui/dialogs/NuevoPacienteDialog.h"
#include "ui/dialogs/PacienteDetalleDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace {
constexpr int kColumnDni = 0;
constexpr int kColumnNombre = 1;
constexpr int kColumnApellido = 2;
constexpr int kColumnTelefono = 3;
constexpr int kColumnAptitud = 4;

constexpr int kRoleDni = Qt::UserRole;
constexpr int kRoleAptitud = Qt::UserRole + 1;
} // namespace

PacientesPage::PacientesPage(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_search(new QLineEdit(this))
    , m_table(new DataTable(this))
    , m_pageLabel(new QLabel(this))
    , m_status(new QLabel(this))
    , m_searchButton(new QPushButton(QStringLiteral("Buscar"), this))
    , m_prev(new QPushButton(QStringLiteral("Anterior"), this))
    , m_next(new QPushButton(QStringLiteral("Siguiente"), this))
    , m_newButton(new QPushButton(QStringLiteral("Nuevo paciente"), this))
{
    // --- Header --------------------------------------------------------------
    auto *header = new PageHeader(QStringLiteral("Pacientes"),
                                  QStringLiteral("Listado y búsqueda de donantes"), this);
    m_newButton->setProperty("variant", "primary");
    m_newButton->setCursor(Qt::PointingHandCursor);
    header->addAction(m_newButton);

    // --- Search card ---------------------------------------------------------
    m_search->setPlaceholderText(QStringLiteral("Buscar por DNI, nombre o apellido"));
    m_search->setClearButtonEnabled(true);
    m_searchButton->setCursor(Qt::PointingHandCursor);

    auto *searchCard = new Card(this);
    auto *searchRow = new QHBoxLayout;
    searchRow->setSpacing(Theme::SpacingSm);
    searchRow->addWidget(m_search, 1);
    searchRow->addWidget(m_searchButton);
    searchCard->addLayout(searchRow);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    // --- Table card ----------------------------------------------------------
    auto *tableCard = new Card(this);
    tableCard->body()->setSpacing(Theme::SpacingSm);

    QTableWidget *table = m_table->table();
    m_table->setHeaders({
        QStringLiteral("DNI"),
        QStringLiteral("Nombre"),
        QStringLiteral("Apellido"),
        QStringLiteral("Teléfono"),
        QStringLiteral("Aptitud"),
    });
    table->horizontalHeader()->setSectionResizeMode(kColumnDni, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnNombre, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(kColumnApellido, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(kColumnTelefono, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnAptitud, QHeaderView::ResizeToContents);

    auto *pagination = new QHBoxLayout;
    pagination->setSpacing(Theme::SpacingSm);
    m_pageLabel->setObjectName(QStringLiteral("PageSubtitle"));
    pagination->addWidget(m_prev);
    pagination->addWidget(m_pageLabel);
    pagination->addWidget(m_next);
    pagination->addStretch();

    tableCard->addWidget(m_status);
    tableCard->body()->addWidget(m_table, 1);
    tableCard->addLayout(pagination);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addWidget(searchCard);
    layout->addWidget(tableCard, 1);

    connect(m_searchButton, &QPushButton::clicked, this, &PacientesPage::onSearch);
    connect(m_search, &QLineEdit::returnPressed, this, &PacientesPage::onSearch);
    connect(m_prev, &QPushButton::clicked, this, &PacientesPage::onPrev);
    connect(m_next, &QPushButton::clicked, this, &PacientesPage::onNext);
    connect(m_newButton, &QPushButton::clicked, this, &PacientesPage::onNewPaciente);
    connect(table, &QTableWidget::cellDoubleClicked, this, &PacientesPage::onRowActivated);

    // Keep a predictable keyboard order: search, actions, then the table.
    QWidget::setTabOrder(m_search, m_searchButton);
    QWidget::setTabOrder(m_searchButton, m_newButton);
    QWidget::setTabOrder(m_newButton, table);
    QWidget::setTabOrder(table, m_prev);
    QWidget::setTabOrder(m_prev, m_next);

    m_table->showEmpty(QStringLiteral("No hay pacientes para mostrar."));
}

void PacientesPage::refresh()
{
    loadPage(m_currentPage);
}

void PacientesPage::onSearch()
{
    loadPage(1);
}

void PacientesPage::onPrev()
{
    if (m_currentPage > 1)
        loadPage(m_currentPage - 1);
}

void PacientesPage::onNext()
{
    if (m_currentPage < m_lastPage)
        loadPage(m_currentPage + 1);
}

void PacientesPage::onNewPaciente()
{
    NuevoPacienteDialog dialog(m_api, this);
    if (dialog.exec() == QDialog::Accepted)
        loadPage(1);
}

void PacientesPage::onRowActivated(int row, int /*column*/)
{
    QTableWidgetItem *item = m_table->table()->item(row, kColumnDni);
    if (!item)
        return;

    const QString dni = item->data(kRoleDni).toString();
    const QString aptitud = item->data(kRoleAptitud).toString();

    PacienteDetalleDialog dialog(m_api, dni, aptitud, this);
    dialog.exec();
}

void PacientesPage::loadPage(int page)
{
    m_query = m_search->text().trimmed();

    QString path = QStringLiteral("/api/pacientes?per_page=%1&page=%2")
                       .arg(kPerPage)
                       .arg(page);
    if (!m_query.isEmpty())
        path += QStringLiteral("&search=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(m_query)));

    setLoading(true);
    m_status->hide();
    m_table->showLoading();

    m_api->get(path, [this, page](const ApiClient::Response &response) {
        setLoading(false);

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            m_table->showEmpty(QStringLiteral("No se pudieron cargar los pacientes."));
            return;
        }

        const QJsonObject root = response.json.object();
        const QJsonArray rows = root.value(QStringLiteral("data")).toArray();
        fillTable(rows);

        const QJsonObject meta = root.value(QStringLiteral("meta")).toObject();
        m_currentPage = meta.value(QStringLiteral("current_page")).toInt(page);
        m_lastPage = qMax(1, meta.value(QStringLiteral("last_page")).toInt(1));
        const int total = meta.value(QStringLiteral("total")).toInt(rows.size());

        m_pageLabel->setText(QStringLiteral("Página %1 de %2 · %3 registros")
                                 .arg(m_currentPage)
                                 .arg(m_lastPage)
                                 .arg(total));
        m_prev->setEnabled(m_currentPage > 1);
        m_next->setEnabled(m_currentPage < m_lastPage);

        if (rows.isEmpty()) {
            m_table->showEmpty(m_query.isEmpty()
                                   ? QStringLiteral("No hay pacientes registrados todavía.")
                                   : QStringLiteral("No hay pacientes que coincidan con la búsqueda."));
        } else {
            m_table->showTable();
        }
    });
}

void PacientesPage::fillTable(const QJsonArray &rows)
{
    QTableWidget *table = m_table->table();
    table->setRowCount(0);
    table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject patient = rows.at(row).toObject();
        const QString dni = patient.value(QStringLiteral("dni")).toString();
        const QString aptitud = patient.value(QStringLiteral("aptitud")).toObject()
                                    .value(QStringLiteral("tipo")).toString();

        auto *dniItem = new QTableWidgetItem(dni);
        dniItem->setData(kRoleDni, dni);
        dniItem->setData(kRoleAptitud, aptitud);

        table->setItem(row, kColumnDni, dniItem);
        table->setItem(row, kColumnNombre,
                       new QTableWidgetItem(patient.value(QStringLiteral("nombre")).toString()));
        table->setItem(row, kColumnApellido,
                       new QTableWidgetItem(patient.value(QStringLiteral("apellido")).toString()));
        table->setItem(row, kColumnTelefono,
                       new QTableWidgetItem(patient.value(QStringLiteral("telefono")).toString()));
        table->setCellWidget(row, kColumnAptitud,
                             StatusChip::makeCell(aptitudLabel(aptitud),
                                                  StatusChip::aptitudKind(aptitud)));
    }
}

void PacientesPage::setLoading(bool loading)
{
    m_search->setEnabled(!loading);
    m_searchButton->setEnabled(!loading);
    m_newButton->setEnabled(!loading);
    setCursor(loading ? Qt::BusyCursor : Qt::ArrowCursor);
}
