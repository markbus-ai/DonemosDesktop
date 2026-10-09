#pragma once

#include <QStringList>
#include <QWidget>

class QLabel;
class QStackedWidget;
class QTableWidget;

/**
 * Styled data table with built-in loading and empty states.
 *
 * The widget stacks the table, a centered "loading" label and a centered
 * "empty" label, so callers never leave a blank area while data is fetched or
 * when a result set is empty.
 */
class DataTable : public QWidget
{
    Q_OBJECT

public:
    explicit DataTable(QWidget *parent = nullptr);

    QTableWidget *table() const;

    /** Configure columns and uppercase small-caps header labels. */
    void setHeaders(const QStringList &headers);

    void showLoading(const QString &text = QStringLiteral("Cargando información..."));
    void showEmpty(const QString &text = QStringLiteral("No hay registros para mostrar."));
    void showTable();

private:
    QStackedWidget *m_stack;
    QTableWidget *m_table;
    QLabel *m_loading;
    QLabel *m_empty;
};
