#pragma once

#include <QDialog>
#include <QJsonObject>

class QLabel;

/**
 * Read-only donation traceability view.
 *
 * Receives the donation object already returned by the list endpoint (which
 * embeds paciente, sede, tipo, componentes and autoexclusion), so it never
 * issues another request.
 */
class DonacionDetalleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DonacionDetalleDialog(const QJsonObject &donation, QWidget *parent = nullptr);

private:
    QLabel *m_descartadaChip;
    QLabel *m_autoexclusionChip;
};
