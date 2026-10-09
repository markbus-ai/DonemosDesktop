#pragma once

#include <QDialog>
#include <QJsonObject>

class ApiClient;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QDialogButtonBox;

/** Create-patient form with inline per-field 422 error display. */
class NuevoPacienteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NuevoPacienteDialog(ApiClient *api, QWidget *parent = nullptr);

private slots:
    void onSubmit();

private:
    void setBusy(bool busy);
    void clearErrors();
    void showFieldErrors(const QJsonObject &errors);

    ApiClient *m_api;
    QLineEdit *m_dni;
    QLineEdit *m_nombre;
    QLineEdit *m_apellido;
    QLineEdit *m_telefono;
    QComboBox *m_sexo;
    QDoubleSpinBox *m_altura;
    QLabel *m_status;
    QDialogButtonBox *m_buttons;
};
