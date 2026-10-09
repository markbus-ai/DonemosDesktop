#pragma once

#include <QJsonArray>
#include <QWidget>

class ApiClient;
class Card;
class DataTable;
class QLabel;
class QLineEdit;
class QPushButton;

/** Paginated patient list with search, creation and a detail view. */
class PacientesPage : public QWidget
{
    Q_OBJECT

public:
    explicit PacientesPage(ApiClient *api, QWidget *parent = nullptr);

    /** Reload the current page from the API. */
    void refresh();

private slots:
    void onSearch();
    void onPrev();
    void onNext();
    void onNewPaciente();
    void onRowActivated(int row, int column);

private:
    void loadPage(int page);
    void fillTable(const QJsonArray &rows);
    void setLoading(bool loading);

    ApiClient *m_api;
    QLineEdit *m_search;
    DataTable *m_table;
    QLabel *m_pageLabel;
    QLabel *m_status;
    QPushButton *m_searchButton;
    QPushButton *m_prev;
    QPushButton *m_next;
    QPushButton *m_newButton;

    int m_currentPage = 1;
    int m_lastPage = 1;
    QString m_query;

    static constexpr int kPerPage = 20;
};
