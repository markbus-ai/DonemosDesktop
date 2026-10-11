#pragma once

#include <QJsonArray>
#include <QWidget>

class ApiClient;
class DataTable;
class QCheckBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * Donation list with search/date filters and a traceability detail dialog.
 * Paginates like the patient list and keeps the raw rows so the detail dialog
 * can reuse the already-fetched payload without another request.
 */
class DonacionesPage : public QWidget
{
    Q_OBJECT

public:
    explicit DonacionesPage(ApiClient *api, QWidget *parent = nullptr);

    /** Reload the current page from the API. */
    void refresh();

private slots:
    void onSearch();
    void onPrev();
    void onNext();
    void onFilterToggled();
    void onRowActivated(int row, int column);

private:
    void loadPage(int page);
    void fillTable(const QJsonArray &rows);
    void setBusy(bool busy);

    ApiClient *m_api;
    QLineEdit *m_search;
    QCheckBox *m_dateFilter;
    QDateEdit *m_desde;
    QDateEdit *m_hasta;
    QPushButton *m_searchButton;
    DataTable *m_table;
    QLabel *m_pageLabel;
    QLabel *m_status;
    QPushButton *m_prev;
    QPushButton *m_next;

    QJsonArray m_rows;
    int m_currentPage = 1;
    int m_lastPage = 1;

    static constexpr int kPerPage = 15;
};
