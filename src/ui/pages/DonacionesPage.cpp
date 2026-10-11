#include "ui/pages/DonacionesPage.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/components/Card.h"
#include "ui/components/DataTable.h"
#include "ui/components/PageHeader.h"
#include "ui/components/StatusChip.h"
#include "ui/dialogs/DonacionDetalleDialog.h"

#include <QCheckBox>
#include <QDate>
#include <QDateEdit>
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
constexpr int kColumnNumero = 0;
constexpr int kColumnFecha = 1;
constexpr int kColumnPaciente = 2;
constexpr int kColumnTipo = 3;
constexpr int kColumnComponentes = 4;
constexpr int kColumnDescartada = 5;

constexpr int kRoleRow = Qt::UserRole;

QString prettyDate(const QString &iso)
{
    const QDate date = QDate::fromString(iso, Qt::ISODate);
    return date.isValid() ? date.toString(QStringLiteral("dd/MM/yyyy")) : iso;
}
} // namespace

DonacionesPage::DonacionesPage(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_search(new QLineEdit(this))
    , m_dateFilter(new QCheckBox(QStringLiteral("Filtrar por fecha"), this))
    , m_desde(new QDateEdit(QDate::currentDate().addMonths(-1), this))
    , m_hasta(new QDateEdit(QDate::currentDate(), this))
    , m_searchButton(new QPushButton(QStringLiteral("Buscar"), this))
    , m_table(new DataTable(this))
    , m_pageLabel(new QLabel(this))
    , m_status(new QLabel(this))
    , m_prev(new QPushButton(QStringLiteral("Anterior"), this))
    , m_next(new QPushButton(QStringLiteral("Siguiente"), this))
{
    // --- Header --------------------------------------------------------------
    auto *header = new PageHeader(QStringLiteral("Donaciones"),
                                  QStringLiteral("Registro y trazabilidad de donaciones"), this);

    // --- Filters card --------------------------------------------------------
    m_search->setPlaceholderText(QStringLiteral("Buscar por Nº de donación"));
    m_search->setClearButtonEnabled(true);
    m_searchButton->setCursor(Qt::PointingHandCursor);
    m_desde->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_desde->setCalendarPopup(true);
    m_hasta->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_hasta->setCalendarPopup(true);
    m_desde->setEnabled(false);
    m_hasta->setEnabled(false);

    auto *filtersRow = new QHBoxLayout;
    filtersRow->setSpacing(Theme::SpacingSm);
    filtersRow->addWidget(m_search, 1);
    filtersRow->addWidget(m_dateFilter);
    filtersRow->addWidget(m_desde);
    auto *toLabel = new QLabel(QStringLiteral("a"), this);
    toLabel->setObjectName(QStringLiteral("InfoText"));
    filtersRow->addWidget(toLabel);
    filtersRow->addWidget(m_hasta);
    filtersRow->addWidget(m_searchButton);

    auto *filtersCard = new Card(this);
    filtersCard->addLayout(filtersRow);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    // --- Table card ----------------------------------------------------------
    auto *tableCard = new Card(this);
    tableCard->body()->setSpacing(Theme::SpacingSm);
    m_table->setHeaders({
        QStringLiteral("Nº"),
        QStringLiteral("Fecha"),
        QStringLiteral("Paciente"),
        QStringLiteral("Tipo"),
        QStringLiteral("Componentes"),
        QStringLiteral("Descartada"),
    });

    QTableWidget *table = m_table->table();
    table->horizontalHeader()->setSectionResizeMode(kColumnNumero, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnFecha, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnPaciente, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(kColumnTipo, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnComponentes, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnDescartada, QHeaderView::ResizeToContents);

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
    layout->addWidget(filtersCard);
    layout->addWidget(tableCard, 1);

    connect(m_searchButton, &QPushButton::clicked, this, &DonacionesPage::onSearch);
    connect(m_search, &QLineEdit::returnPressed, this, &DonacionesPage::onSearch);
    connect(m_dateFilter, &QCheckBox::toggled, this, &DonacionesPage::onFilterToggled);
    connect(m_prev, &QPushButton::clicked, this, &DonacionesPage::onPrev);
    connect(m_next, &QPushButton::clicked, this, &DonacionesPage::onNext);
    connect(table, &QTableWidget::cellDoubleClicked, this, &DonacionesPage::onRowActivated);

    m_table->showEmpty(QStringLiteral("No hay donaciones para mostrar."));
}

void DonacionesPage::refresh()
{
    loadPage(m_currentPage);
}

void DonacionesPage::onSearch()
{
    loadPage(1);
}

void DonacionesPage::onFilterToggled()
{
    const bool enabled = m_dateFilter->isChecked();
    m_desde->setEnabled(enabled);
    m_hasta->setEnabled(enabled);
}

void DonacionesPage::onPrev()
{
    if (m_currentPage > 1)
        loadPage(m_currentPage - 1);
}

void DonacionesPage::onNext()
{
    if (m_currentPage < m_lastPage)
        loadPage(m_currentPage + 1);
}

void DonacionesPage::onRowActivated(int row, int /*column*/)
{
    QTableWidgetItem *item = m_table->table()->item(row, kColumnNumero);
    if (!item)
        return;

    const int sourceRow = item->data(kRoleRow).toInt();
    if (sourceRow < 0 || sourceRow >= m_rows.size())
        return;

    DonacionDetalleDialog dialog(m_rows.at(sourceRow).toObject(), this);
    dialog.exec();
}

void DonacionesPage::loadPage(int page)
{
    QString path = QStringLiteral("/api/donaciones?per_page=%1&page=%2").arg(kPerPage).arg(page);

    const QString query = m_search->text().trimmed();
    if (!query.isEmpty())
        path += QStringLiteral("&search=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(query)));

    if (m_dateFilter->isChecked()) {
        path += QStringLiteral("&desde=%1").arg(m_desde->date().toString(Qt::ISODate));
        path += QStringLiteral("&hasta=%1").arg(m_hasta->date().toString(Qt::ISODate));
    }

    setBusy(true);
    m_status->hide();
    m_table->showLoading();

    m_api->get(path, [this, page](const ApiClient::Response &response) {
        setBusy(false);

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            m_table->showEmpty(QStringLiteral("No se pudieron cargar las donaciones."));
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

        if (rows.isEmpty())
            m_table->showEmpty(QStringLiteral("No hay donaciones que coincidan con el filtro."));
        else
            m_table->showTable();
    });
}

void DonacionesPage::fillTable(const QJsonArray &rows)
{
    m_rows = rows;
    QTableWidget *table = m_table->table();
    table->setRowCount(0);
    table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject donation = rows.at(row).toObject();
        const QJsonObject paciente = donation.value(QStringLiteral("paciente")).toObject();
        const QJsonObject tipo = donation.value(QStringLiteral("tipo")).toObject();
        const QJsonArray componentes = donation.value(QStringLiteral("componentes")).toArray();
        const bool descartada = donation.value(QStringLiteral("descartada")).toBool();

        auto *numeroItem = new QTableWidgetItem(donation.value(QStringLiteral("numero_donacion")).toString());
        numeroItem->setData(kRoleRow, row);

        const QString pacienteName = QStringLiteral("%1 %2")
                                         .arg(paciente.value(QStringLiteral("nombre")).toString(),
                                              paciente.value(QStringLiteral("apellido")).toString())
                                         .trimmed();

        table->setItem(row, kColumnNumero, numeroItem);
        table->setItem(row, kColumnFecha,
                       new QTableWidgetItem(prettyDate(donation.value(QStringLiteral("fecha")).toString())));
        table->setItem(row, kColumnPaciente, new QTableWidgetItem(pacienteName));
        table->setItem(row, kColumnTipo,
                       new QTableWidgetItem(tipo.value(QStringLiteral("nombre")).toString()));
        table->setItem(row, kColumnComponentes,
                       new QTableWidgetItem(QString::number(componentes.size())));
        table->setCellWidget(row, kColumnDescartada,
                             StatusChip::makeCell(descartada ? QStringLiteral("Sí") : QStringLiteral("No"),
                                                  descartada ? QStringLiteral("error") : QStringLiteral("muted")));
    }
}

void DonacionesPage::setBusy(bool busy)
{
    m_search->setEnabled(!busy);
    m_dateFilter->setEnabled(!busy);
    m_desde->setEnabled(!busy && m_dateFilter->isChecked());
    m_hasta->setEnabled(!busy && m_dateFilter->isChecked());
    m_searchButton->setEnabled(!busy);
    setCursor(busy ? Qt::BusyCursor : Qt::ArrowCursor);
}
