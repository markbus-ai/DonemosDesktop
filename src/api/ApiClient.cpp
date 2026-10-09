#include "api/ApiClient.h"

#include <QJsonArray>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

namespace {
constexpr auto kDefaultBaseUrl = "http://127.0.0.1:8000";
constexpr auto kSettingsOrg = "Donemos";
constexpr auto kSettingsApp = "DonemosDesktop";
constexpr auto kKeyBaseUrl = "api/baseUrl";
constexpr auto kKeyToken = "api/token";

/** First message from a Laravel 422 `errors` object, for a compact error text. */
QString firstFieldMessage(const QJsonObject &fieldErrors)
{
    for (auto it = fieldErrors.constBegin(); it != fieldErrors.constEnd(); ++it) {
        const QJsonValue value = it.value();
        if (value.isArray() && !value.toArray().isEmpty())
            return value.toArray().first().toString();
        if (value.isString())
            return value.toString();
    }
    return {};
}
} // namespace

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_baseUrl(QString::fromLatin1(kDefaultBaseUrl))
{
}

void ApiClient::loadSettings()
{
    QSettings settings(QString::fromLatin1(kSettingsOrg), QString::fromLatin1(kSettingsApp));
    m_baseUrl = settings.value(QString::fromLatin1(kKeyBaseUrl), QString::fromLatin1(kDefaultBaseUrl)).toString();
    m_token = settings.value(QString::fromLatin1(kKeyToken)).toString();

    if (m_baseUrl.trimmed().isEmpty())
        m_baseUrl = QString::fromLatin1(kDefaultBaseUrl);
}

void ApiClient::saveSettings() const
{
    QSettings settings(QString::fromLatin1(kSettingsOrg), QString::fromLatin1(kSettingsApp));
    settings.setValue(QString::fromLatin1(kKeyBaseUrl), m_baseUrl);
    settings.setValue(QString::fromLatin1(kKeyToken), m_token);
}

QString ApiClient::baseUrl() const
{
    return m_baseUrl;
}

void ApiClient::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl.trimmed();
    if (m_baseUrl.endsWith('/'))
        m_baseUrl.chop(1);
}

QString ApiClient::token() const
{
    return m_token;
}

void ApiClient::setToken(const QString &token)
{
    m_token = token;
}

bool ApiClient::hasToken() const
{
    return !m_token.isEmpty();
}

void ApiClient::clearSession()
{
    m_token.clear();
    saveSettings();
}

QUrl ApiClient::buildUrl(const QString &path) const
{
    // `path` always starts with '/', e.g. "/api/pacientes?per_page=20".
    QString base = m_baseUrl;
    if (base.endsWith('/'))
        base.chop(1);
    return QUrl(base + path);
}

void ApiClient::get(const QString &path, Callback callback)
{
    send("GET", path, {}, std::move(callback));
}

void ApiClient::post(const QString &path, const QJsonObject &body, Callback callback)
{
    send("POST", path, body, std::move(callback));
}

void ApiClient::patch(const QString &path, const QJsonObject &body, Callback callback)
{
    send("PATCH", path, body, std::move(callback));
}

void ApiClient::send(const QByteArray &verb,
                     const QString &path,
                     const QJsonObject &body,
                     Callback callback)
{
    QNetworkRequest request(buildUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");

    const bool authenticated = !m_token.isEmpty();
    if (authenticated)
        request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    const QByteArray payload = body.isEmpty()
        ? QByteArray()
        : QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager->sendCustomRequest(request, verb, payload);

    connect(reply, &QNetworkReply::finished, this, [this, reply, authenticated, callback = std::move(callback)]() {
        const Response response = parseReply(reply);
        reply->deleteLater();

        if (response.status == 401 && authenticated) {
            clearSession();
            emit sessionExpired();
        }

        if (callback)
            callback(response);
    });
}

ApiClient::Response ApiClient::parseReply(QNetworkReply *reply) const
{
    Response response;

    const QVariant statusAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    response.status = statusAttr.isValid() ? statusAttr.toInt() : 0;

    const QByteArray raw = reply->readAll();
    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error == QJsonParseError::NoError)
        response.json = document;

    const QJsonObject object = document.isObject() ? document.object() : QJsonObject{};
    response.serverMessage = object.value("message").toString();
    response.fieldErrors = object.value("errors").toObject();

    if (response.status == 0) {
        response.networkError = true;
        response.errorMessage = QStringLiteral("No se pudo conectar con el servidor. "
                                              "Verificá la URL y tu conexión.");
        return response;
    }

    response.ok = response.status >= 200 && response.status < 300;
    if (!response.ok) {
        if (!response.serverMessage.isEmpty())
            response.errorMessage = response.serverMessage;
        else if (!response.fieldErrors.isEmpty())
            response.errorMessage = firstFieldMessage(response.fieldErrors);
        else
            response.errorMessage = QStringLiteral("Ocurrió un error inesperado (HTTP %1).")
                                        .arg(response.status);
    }

    return response;
}
