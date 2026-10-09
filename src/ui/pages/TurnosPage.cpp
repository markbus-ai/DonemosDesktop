#include "ui/pages/TurnosPage.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/components/Card.h"
#include "ui/components/DataTable.h"
#include "ui/components/PageHeader.h"
#include "ui/components/StatusChip.h"
#include "ui/dialogs/NuevoTurnoDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
constexpr int kColumnFecha = 0;
constexpr int kColumnHora = 1;
constexpr int kColumnPaciente = 2;
constexpr int kColumnSede = 3;
constexpr int kColumnTipo = 4;
constexpr int kColumnEstado = 5;
} // namespace

TurnosPage::TurnosPage(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_table(new DataTable(this))
    , m_status(new QLabel(this))
    , m_refreshButton(new QPushButton(QStringLiteral("Actualizar"), this))
    , m_newButton(new QPushButton(QStringLiteral("Nuevo turno"), this))
{
    // --- Header --------------------------------------------------------------
    auto *header = new PageHeader(QStringLiteral("Turnos"),
                                  QStringLiteral("Agenda de donaciones"), this);
    m_newButton->setProperty("variant", "primary");
    m_newButton->setCursor(Qt::PointingHandCursor);
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    header->addAction(m_refreshButton);
    header->addAction(m_newButton);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    // --- Table card ----------------------------------------------------------
    auto *tableCard = new Card(this);
    tableCard->body()->setSpacing(Theme::SpacingSm);
    m_table->setHeaders({
        QStringLiteral("Fecha"),
        QStringLiteral("Hora"),
        QStringLiteral("Paciente"),
        QStringLiteral("Sede"),
        QStringLiteral("Tipo"),
        QStringLiteral("Estado"),
    });

    QTableWidget *table = m_table->table();
    table->horizontalHeader()->setSectionResizeMode(kColumnFecha, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnHora, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnPaciente, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(kColumnSede, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(kColumnTipo, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(kColumnEstado, QHeaderView::ResizeToContents);

    tableCard->addWidget(m_status);
    tableCard->body()->addWidget(m_table, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addWidget(tableCard, 1);

    connect(m_refreshButton, &QPushButton::clicked, this, &TurnosPage::refresh);
    connect(m_newButton, &QPushButton::clicked, this, &TurnosPage::onNewTurno);

    QWidget::setTabOrder(m_refreshButton, m_newButton);
    QWidget::setTabOrder(m_newButton, table);

    m_table->showEmpty(QStringLiteral("No hay turnos para mostrar."));
}

void TurnosPage::refresh()
{
    setLoading(true);
    m_status->hide();
    m_table->showLoading();

    m_api->get(QStringLiteral("/api/turnos"), [this](const ApiClient::Response &response) {
        setLoading(false);

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            m_table->showEmpty(QStringLiteral("No se pudieron cargar los turnos."));
            return;
        }

        const QJsonArray rows = response.json.array();
        fillTable(rows);

        if (rows.isEmpty())
            m_table->showEmpty(QStringLiteral("No hay turnos registrados todavía."));
        else
            m_table->showTable();
    });
}

void TurnosPage::onNewTurno()
{
    NuevoTurnoDialog dialog(m_api, this);
    if (dialog.exec() == QDialog::Accepted)
        refresh();
}

void TurnosPage::fillTable(const QJsonArray &rows)
{
    QTableWidget *table = m_table->table();
    table->setRowCount(0);
    table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject turno = rows.at(row).toObject();
        const QJsonObject paciente = turno.value(QStringLiteral("paciente")).toObject();
        const QJsonObject sede = turno.value(QStringLiteral("sede")).toObject();
        const QJsonObject tipo = turno.value(QStringLiteral("tipo")).toObject();
        const QString estado = turno.value(QStringLiteral("estado")).toString();

        const QString pacienteName = QStringLiteral("%1 %2")
                                         .arg(paciente.value(QStringLiteral("nombre")).toString(),
                                              paciente.value(QStringLiteral("apellido")).toString())
                                         .trimmed();

        table->setItem(row, kColumnFecha,
                       new QTableWidgetItem(turno.value(QStringLiteral("fecha")).toString()));
        table->setItem(row, kColumnHora,
                       new QTableWidgetItem(turno.value(QStringLiteral("hora")).toString()));
        table->setItem(row, kColumnPaciente, new QTableWidgetItem(pacienteName));
        table->setItem(row, kColumnSede,
                       new QTableWidgetItem(sede.value(QStringLiteral("nombre")).toString()));
        table->setItem(row, kColumnTipo,
                       new QTableWidgetItem(tipo.value(QStringLiteral("nombre")).toString()));
        table->setCellWidget(row, kColumnEstado,
                             StatusChip::makeCell(StatusChip::turnoLabel(estado),
                                                  StatusChip::turnoKind(estado)));
    }
}

void TurnosPage::setLoading(bool loading)
{
    m_refreshButton->setEnabled(!loading);
    m_newButton->setEnabled(!loading);
    setCursor(loading ? Qt::BusyCursor : Qt::ArrowCursor);
}
