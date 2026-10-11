#pragma once

#include <QDialog>
#include <QJsonObject>

class ApiClient;
class QFormLayout;
class QLabel;
class QPushButton;

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
    void reload();
    void populate(const QJsonObject &patient);
    void openEditCiclo();
    void showFeedback(const QString &message);

    ApiClient *m_api;
    QString m_dni;
    QString m_aptitudTipo;
    QLabel *m_status;
    QLabel *m_feedback;
    QLabel *m_aptitudChip;
    QFormLayout *m_form;
    QLabel *m_cicloValue;
    QLabel *m_serieValue;
    QLabel *m_donacionValue;
    QPushButton *m_editCicloButton;

    // Current raw counters (0 means "not set"), used to prefill the editor.
    qint64 m_ciclo = 0;
    qint64 m_serie = 0;
    qint64 m_donacion = 0;
};
