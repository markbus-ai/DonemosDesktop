#pragma once

#include <QDialog>

class ApiClient;
class QDialogButtonBox;
class QLabel;
class QSpinBox;

/**
 * Manual editor for a patient's current donation-cycle counters.
 *
 * The three values (ciclo, serie, donación) are free-form bookkeeping: staff
 * type them by hand and may reset any or all of them to "not set" (0 in the
 * spin box), starting a new ciclo/serie whenever they want. The system never
 * computes them.
 */
class EditarCicloDialog : public QDialog
{
    Q_OBJECT

public:
    EditarCicloDialog(ApiClient *api,
                      const QString &dni,
                      qint64 ciclo,
                      qint64 serie,
                      qint64 donacion,
                      QWidget *parent = nullptr);

private:
    void onSubmit();
    void setBusy(bool busy);

    ApiClient *m_api;
    QString m_dni;
    QSpinBox *m_ciclo;
    QSpinBox *m_serie;
    QSpinBox *m_donacion;
    QLabel *m_status;
    QDialogButtonBox *m_buttons;
};
