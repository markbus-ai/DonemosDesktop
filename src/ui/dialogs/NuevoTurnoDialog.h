#pragma once

#include <QDialog>

class ApiClient;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimeEdit;

/**
 * Appointment creation. Resolves the patient by DNI, pulls the donation types
 * from the catalog and handles the backend 409 warning with an explicit force
 * step (which requires a motivo).
 */
class NuevoTurnoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NuevoTurnoDialog(ApiClient *api, QWidget *parent = nullptr);

private slots:
    void onSearchPatient();
    void onSubmit();

private:
    void loadTipos();
    void setBusy(bool busy);
    void submit(bool force, const QString &motivo);
    void handleWarning(const QString &message, const QStringList &warnings);

    ApiClient *m_api;
    QLineEdit *m_dni;
    QPushButton *m_searchButton;
    QLabel *m_patientLabel;
    QComboBox *m_tipo;
    QDateEdit *m_fecha;
    QTimeEdit *m_hora;
    QLabel *m_status;
    QDialogButtonBox *m_buttons;

    int m_pacienteId = 0;
};
