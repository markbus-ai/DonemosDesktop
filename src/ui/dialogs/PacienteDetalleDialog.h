#pragma once

#include <QDialog>
#include <QJsonObject>

class ApiClient;
class QFormLayout;
class QLabel;

/** Read-only detail view for a single patient, fetched by DNI. */
class PacienteDetalleDialog : public QDialog
{
    Q_OBJECT

public:
    PacienteDetalleDialog(ApiClient *api,
                          const QString &dni,
                          const QString &aptitudTipo,
                          QWidget *parent = nullptr);

private:
    QLabel *addRow(QFormLayout *form, const QString &label);
    void populate(const QJsonObject &patient);

    ApiClient *m_api;
    QString m_dni;
    QString m_aptitudTipo;
    QLabel *m_status;
    QLabel *m_aptitudChip;
    QFormLayout *m_form;
};
