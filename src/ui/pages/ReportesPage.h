#pragma once

#include <QWidget>

#include <utility>

class ApiClient;
class Card;
class QCheckBox;
class QDateEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;

/**
 * Read-only operations dashboard. Shows the headline donation counters plus
 * simple horizontal bars for donations by sex, by product and appointments by
 * state, all fed by /api/reportes/resumen.
 */
class ReportesPage : public QWidget
{
    Q_OBJECT

public:
    explicit ReportesPage(ApiClient *api, QWidget *parent = nullptr);

    /** Reload the summary for the current filter. */
    void refresh();

private slots:
    void onFilterToggled();

private:
    void loadSummary();
    void renderBars(QVBoxLayout *target, const QList<QPair<QString, int>> &bars);
    void setBusy(bool busy);

    ApiClient *m_api;
    QCheckBox *m_dateFilter;
    QDateEdit *m_desde;
    QDateEdit *m_hasta;
    QPushButton *m_refreshButton;
    QLabel *m_status;
    QLabel *m_loading;
    QWidget *m_content;
    QLabel *m_totalValue;
    QLabel *m_descartadasValue;
    QVBoxLayout *m_sexoBars;
    QVBoxLayout *m_productoBars;
    QVBoxLayout *m_turnosBars;
};
