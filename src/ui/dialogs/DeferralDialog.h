#pragma once

#include <QDialog>

class ApiClient;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QWidget;

/**
 * Create a deferral row for a patient.
 *
 * `Observacion` posts to /observacion (motivo + desde + optional hasta).
 * `Restriccion` posts to /restriccion and additionally accepts the permanent
 * flag, which forbids a return date.
 */
class DeferralDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { Observacion, Restriccion };

    explicit DeferralDialog(ApiClient *api, const QString &dni, Mode mode, QWidget *parent = nullptr);

private slots:
    void onSubmit();

private:
    void loadMotivos();
    void applyPermanentState();
    void setBusy(bool busy);

    ApiClient *m_api;
    QString m_dni;
    Mode m_mode;
    QComboBox *m_motivo;
    QDateEdit *m_desde;
    QCheckBox *m_permanente;
    QCheckBox *m_useHasta;
    QDateEdit *m_hasta;
    QLabel *m_status;
    QDialogButtonBox *m_buttons;
};
