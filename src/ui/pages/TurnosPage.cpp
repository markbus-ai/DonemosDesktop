#include "ui/pages/TurnosPage.h"

#include "api/ApiClient.h"
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
    , m_table(new QTableWidget(this))
    , m_status(new QLabel(this))
    , m_refreshButton(new QPushButton(QStringLiteral("Actualizar"), this))
    , m_newButton(new QPushButton(QStringLiteral("Nuevo turno"), this))
{
    auto *title = new QLabel(QStringLiteral("Turnos"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *headerRow = new QHBoxLayout;
    headerRow->addStretch();
    headerRow->addWidget(m_refreshButton);
    headerRow->addWidget(m_newButton);

    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("Fecha"),
        QStringLiteral("Hora"),
        QStringLiteral("Paciente"),
        QStringLiteral("Sede"),
        QStringLiteral("Tipo"),
        QStringLiteral("Estado"),
    });
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(kColumnPaciente, QHeaderView::Stretch);

    m_status->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_status->hide();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addLayout(headerRow);
    layout->addWidget(m_status);
    layout->addWidget(m_table, 1);

    connect(m_refreshButton, &QPushButton::clicked, this, &TurnosPage::refresh);
    connect(m_newButton, &QPushButton::clicked, this, &TurnosPage::onNewTurno);

    // Keep a predictable keyboard order: actions first, then the table.
    QWidget::setTabOrder(m_refreshButton, m_newButton);
    QWidget::setTabOrder(m_newButton, m_table);
}

void TurnosPage::refresh()
{
    setLoading(true);
    m_status->hide();

    m_api->get(QStringLiteral("/api/turnos"), [this](const ApiClient::Response &response) {
        setLoading(false);

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            return;
        }

        fillTable(response.json.array());
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
    m_table->setRowCount(0);
    m_table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject turno = rows.at(row).toObject();
        const QJsonObject paciente = turno.value(QStringLiteral("paciente")).toObject();
        const QJsonObject sede = turno.value(QStringLiteral("sede")).toObject();
        const QJsonObject tipo = turno.value(QStringLiteral("tipo")).toObject();

        const QString pacienteName = QStringLiteral("%1 %2")
                                         .arg(paciente.value(QStringLiteral("nombre")).toString(),
                                              paciente.value(QStringLiteral("apellido")).toString())
                                         .trimmed();

        m_table->setItem(row, kColumnFecha,
                         new QTableWidgetItem(turno.value(QStringLiteral("fecha")).toString()));
        m_table->setItem(row, kColumnHora,
                         new QTableWidgetItem(turno.value(QStringLiteral("hora")).toString()));
        m_table->setItem(row, kColumnPaciente, new QTableWidgetItem(pacienteName));
        m_table->setItem(row, kColumnSede,
                         new QTableWidgetItem(sede.value(QStringLiteral("nombre")).toString()));
        m_table->setItem(row, kColumnTipo,
                         new QTableWidgetItem(tipo.value(QStringLiteral("nombre")).toString()));
        m_table->setItem(row, kColumnEstado,
                         new QTableWidgetItem(turno.value(QStringLiteral("estado")).toString()));
    }
}

void TurnosPage::setLoading(bool loading)
{
    m_refreshButton->setEnabled(!loading);
    m_newButton->setEnabled(!loading);
    setCursor(loading ? Qt::BusyCursor : Qt::ArrowCursor);
}
