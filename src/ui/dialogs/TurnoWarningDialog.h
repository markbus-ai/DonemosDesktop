#pragma once

#include <QDialog>
#include <QStringList>

class QLabel;
class QLineEdit;

/**
 * 409 conflict warning for appointment creation. Lists the backend warnings
 * and asks for a motivo, required to force the appointment.
 */
class TurnoWarningDialog : public QDialog
{
    Q_OBJECT

public:
    TurnoWarningDialog(const QString &message,
                       const QStringList &warnings,
                       QWidget *parent = nullptr);

    /** Motivo typed by the user (trimmed), valid only when accepted. */
    QString motivo() const;

private:
    void onForce();

    QLineEdit *m_motivo;
    QLabel *m_error;
};
