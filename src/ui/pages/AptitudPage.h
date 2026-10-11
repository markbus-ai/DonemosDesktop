#pragma once

#include <QJsonObject>
#include <QWidget>

class ApiClient;
class Card;
class QDate;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * Patient-scoped aptitude screen: look a donor up by DNI and manage aptitude,
 * observations, deferrals and platelet habilitation from one place. Every
 * mutation refreshes the card so the chips always mirror the backend state.
 */
class AptitudPage : public QWidget
{
    Q_OBJECT

public:
    explicit AptitudPage(ApiClient *api, QWidget *parent = nullptr);

    /** Re-query the patient currently loaded, if any. */
    void refresh();

private slots:
    void onSearch();
    void onChangeAptitud();
    void onAddObservacion();
    void onRemoveObservacion();
    void onAddRestriccion();
    void onRemoveRestriccion();
    void onEnablePlatelets();
    void onDisablePlatelets();

private:
    void loadPatient(const QString &dni);
    void loadPlatelets();
    void renderPatient();
    void setBusy(bool busy);
    void showError(const QString &message);
    void showFeedback(const QString &message);
    void confirmDelete(const QString &question, const QString &path, const QString &success);

    ApiClient *m_api;
    QLineEdit *m_dni;
    QPushButton *m_searchButton;
    QLabel *m_status;
    QLabel *m_feedback;
    QLabel *m_placeholder;
    Card *m_patientCard;
    QLabel *m_nameLabel;
    QLabel *m_metaLabel;
    QLabel *m_aptitudChip;
    QLabel *m_deferralChip;
    QLabel *m_plateletChip;
    QLabel *m_plateletInfo;
    QLabel *m_cicloInfo;

    QPushButton *m_changeAptitudButton;
    QPushButton *m_addObservacionButton;
    QPushButton *m_removeObservacionButton;
    QPushButton *m_addRestriccionButton;
    QPushButton *m_removeRestriccionButton;
    QPushButton *m_enablePlateletsButton;
    QPushButton *m_disablePlateletsButton;

    QString m_currentDni;
    QJsonObject m_patient;
    bool m_plateletsEnabled = false;
    bool m_hasActiveDeferral = false;
    bool m_busy = false;
};
