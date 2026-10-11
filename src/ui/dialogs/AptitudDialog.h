#pragma once

#include <QDialog>
#include <QJsonObject>

class ApiClient;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QWidget;

/**
 * Change a patient's aptitude (PATCH /api/pacientes/{dni}/aptitud).
 *
 * APTO carries no extra data. APTO_OBSERVACION and NO_APTO require a motivo
 * from the catalog plus a start date, and accept an optional return date or a
 * permanent flag. The motivo catalog is fetched on demand.
 */
class AptitudDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AptitudDialog(ApiClient *api, const QString &dni, QWidget *parent = nullptr);

private slots:
    void onSubmit();

private:
    void loadMotivos();
    void applyTipoState();
    void setBusy(bool busy);
    bool selectedTipoNeedsMotivo() const;

    ApiClient *m_api;
    QString m_dni;
    QComboBox *m_tipo;
    QWidget *m_reasonBox;
    QComboBox *m_motivo;
    QDateEdit *m_desde;
    QCheckBox *m_permanente;
    QCheckBox *m_useHasta;
    QDateEdit *m_hasta;
    QLabel *m_status;
    QDialogButtonBox *m_buttons;
};
