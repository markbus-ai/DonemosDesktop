#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

/**
 * Thin HTTP client for the Donemos Laravel API.
 *
 * Owns the base URL and the bearer token (persisted through QSettings) and maps
 * every reply into a single `Response` struct so callers never touch
 * QNetworkReply directly. All calls are asynchronous: the callback runs once the
 * reply finishes.
 */
class ApiClient : public QObject
{
    Q_OBJECT

public:
    struct Response
    {
        bool ok = false;             // HTTP 2xx
        int status = 0;              // HTTP status, 0 when the request never landed
        QJsonDocument json;          // parsed body (object or array)
        QString serverMessage;       // `message` field when the body carries one
        QJsonObject fieldErrors;     // 422 `errors` object: field -> [messages]
        bool networkError = false;   // transport failure (no HTTP status)
        QString errorMessage;        // friendly, user-facing description
    };

    using Callback = std::function<void(const Response &)>;

    explicit ApiClient(QObject *parent = nullptr);

    void loadSettings();
    void saveSettings() const;

    QString baseUrl() const;
    void setBaseUrl(const QString &baseUrl);

    QString token() const;
    void setToken(const QString &token);
    bool hasToken() const;

    /** Drop the session token (keeps the configured base URL) and persist. */
    void clearSession();

    void get(const QString &path, Callback callback);
    void post(const QString &path, const QJsonObject &body, Callback callback);
    void patch(const QString &path, const QJsonObject &body, Callback callback);
    void del(const QString &path, Callback callback);

signals:
    /** Emitted when an authenticated request is rejected with 401. */
    void sessionExpired();

private:
    void send(const QByteArray &verb,
              const QString &path,
              const QJsonObject &body,
              Callback callback);
    QUrl buildUrl(const QString &path) const;
    Response parseReply(QNetworkReply *reply) const;

    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QString m_token;
};
