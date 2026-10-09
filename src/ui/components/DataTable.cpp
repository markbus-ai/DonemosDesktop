#include "ui/components/DataTable.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QLabel *makeStateLabel(const QString &objectName, QWidget *parent)
{
    auto *label = new QLabel(parent);
    label->setObjectName(objectName);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    return label;
}
} // namespace

DataTable::DataTable(QWidget *parent)
    : QWidget(parent)
    , m_stack(new QStackedWidget(this))
    , m_table(new QTableWidget(this))
    , m_loading(makeStateLabel(QStringLiteral("LoadingState"), this))
    , m_empty(makeStateLabel(QStringLiteral("EmptyState"), this))
{
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFocusPolicy(Qt::StrongFocus);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(40);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_table->horizontalHeader()->setSectionsClickable(false);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_loading->setText(QStringLiteral("Cargando información..."));
    m_empty->setText(QStringLiteral("No hay registros para mostrar."));

    m_stack->addWidget(m_table);
    m_stack->addWidget(m_loading);
    m_stack->addWidget(m_empty);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    showTable();
}

QTableWidget *DataTable::table() const
{
    return m_table;
}

void DataTable::setHeaders(const QStringList &headers)
{
    QStringList uppercase;
    uppercase.reserve(headers.size());
    for (const QString &header : headers)
        uppercase << header.toUpper();

    m_table->setColumnCount(uppercase.size());
    m_table->setHorizontalHeaderLabels(uppercase);
}

void DataTable::showLoading(const QString &text)
{
    m_loading->setText(text);
    m_stack->setCurrentWidget(m_loading);
}

void DataTable::showEmpty(const QString &text)
{
    m_empty->setText(text);
    m_stack->setCurrentWidget(m_empty);
}

void DataTable::showTable()
{
    m_stack->setCurrentWidget(m_table);
}
