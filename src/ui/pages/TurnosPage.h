#pragma once

#include <QJsonArray>
#include <QWidget>

class ApiClient;
class DataTable;
class QLabel;
class QPushButton;

/** Appointment list with refresh and creation (including the 409 force flow). */
class TurnosPage : public QWidget
{
    Q_OBJECT

public:
    explicit TurnosPage(ApiClient *api, QWidget *parent = nullptr);

    void refresh();

private slots:
    void onNewTurno();

private:
    void fillTable(const QJsonArray &rows);
    void setLoading(bool loading);

    ApiClient *m_api;
    DataTable *m_table;
    QLabel *m_status;
    QPushButton *m_refreshButton;
    QPushButton *m_newButton;
};
