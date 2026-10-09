#include "ui/pages/PacientesPage.h"

#include "api/ApiClient.h"
#include "ui/UiUtils.h"
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
    , m_table(new QTableWidget(this))
    , m_pageLabel(new QLabel(this))
    , m_status(new QLabel(this))
    , m_prev(new QPushButton(QStringLiteral("Anterior"), this))
    , m_next(new QPushButton(QStringLiteral("Siguiente"), this))
    , m_newButton(new QPushButton(QStringLiteral("Nuevo paciente"), this))
{
    auto *title = new QLabel(QStringLiteral("Pacientes"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_search->setPlaceholderText(QStringLiteral("Buscar por DNI, nombre o apellido"));

    auto *searchButton = new QPushButton(QStringLiteral("Buscar"), this);

    auto *headerRow = new QHBoxLayout;
    headerRow->addWidget(m_search, 1);
    headerRow->addWidget(searchButton);
    headerRow->addWidget(m_newButton);

    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("DNI"),
        QStringLiteral("Nombre"),
        QStringLiteral("Apellido"),
        QStringLiteral("Teléfono"),
        QStringLiteral("Aptitud"),
    });
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(kColumnNombre, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColumnApellido, QHeaderView::Stretch);

    m_status->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_status->hide();

    auto *pagination = new QHBoxLayout;
    pagination->addWidget(m_prev);
    pagination->addWidget(m_pageLabel);
    pagination->addWidget(m_next);
    pagination->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addLayout(headerRow);
    layout->addWidget(m_status);
    layout->addWidget(m_table, 1);
    layout->addLayout(pagination);

    connect(searchButton, &QPushButton::clicked, this, &PacientesPage::onSearch);
    connect(m_search, &QLineEdit::returnPressed, this, &PacientesPage::onSearch);
    connect(m_prev, &QPushButton::clicked, this, &PacientesPage::onPrev);
    connect(m_next, &QPushButton::clicked, this, &PacientesPage::onNext);
    connect(m_newButton, &QPushButton::clicked, this, &PacientesPage::onNewPaciente);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &PacientesPage::onRowActivated);

    // Keep a predictable keyboard order: search, actions, then the table.
    QWidget::setTabOrder(m_search, searchButton);
    QWidget::setTabOrder(searchButton, m_newButton);
    QWidget::setTabOrder(m_newButton, m_table);
    QWidget::setTabOrder(m_table, m_prev);
    QWidget::setTabOrder(m_prev, m_next);
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
    QTableWidgetItem *item = m_table->item(row, kColumnDni);
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

    m_api->get(path, [this, page](const ApiClient::Response &response) {
        setLoading(false);

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            return;
        }

        const QJsonObject root = response.json.object();
        fillTable(root.value(QStringLiteral("data")).toArray());

        const QJsonObject meta = root.value(QStringLiteral("meta")).toObject();
        m_currentPage = meta.value(QStringLiteral("current_page")).toInt(page);
        m_lastPage = qMax(1, meta.value(QStringLiteral("last_page")).toInt(1));
        const int total = meta.value(QStringLiteral("total")).toInt(m_table->rowCount());

        m_pageLabel->setText(QStringLiteral("Página %1 de %2 · %3 registros")
                                 .arg(m_currentPage)
                                 .arg(m_lastPage)
                                 .arg(total));
        m_prev->setEnabled(m_currentPage > 1);
        m_next->setEnabled(m_currentPage < m_lastPage);
    });
}

void PacientesPage::fillTable(const QJsonArray &rows)
{
    m_table->setRowCount(0);
    m_table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject patient = rows.at(row).toObject();
        const QString dni = patient.value(QStringLiteral("dni")).toString();
        const QString aptitud = patient.value(QStringLiteral("aptitud")).toObject()
                                    .value(QStringLiteral("tipo")).toString();

        auto *dniItem = new QTableWidgetItem(dni);
        dniItem->setData(kRoleDni, dni);
        dniItem->setData(kRoleAptitud, aptitud);

        m_table->setItem(row, kColumnDni, dniItem);
        m_table->setItem(row, kColumnNombre,
                         new QTableWidgetItem(patient.value(QStringLiteral("nombre")).toString()));
        m_table->setItem(row, kColumnApellido,
                         new QTableWidgetItem(patient.value(QStringLiteral("apellido")).toString()));
        m_table->setItem(row, kColumnTelefono,
                         new QTableWidgetItem(patient.value(QStringLiteral("telefono")).toString()));
        m_table->setItem(row, kColumnAptitud, new QTableWidgetItem(aptitudLabel(aptitud)));
    }
}

void PacientesPage::setLoading(bool loading)
{
    m_search->setEnabled(!loading);
    m_newButton->setEnabled(!loading);
    setCursor(loading ? Qt::BusyCursor : Qt::ArrowCursor);
}
