#include "ui/pages/ReportesPage.h"

#include "api/ApiClient.h"
#include "ui/Theme.h"
#include "ui/components/Card.h"
#include "ui/components/PageHeader.h"

#include <QCheckBox>
#include <QDate>
#include <QDateEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

constexpr int kLabelWidth = 120;
constexpr int kValueWidth = 48;

/** Remove every child widget from a bar host before repopulating it. */
void clearLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            widget->deleteLater();
        delete item;
    }
}

/** Human-readable sex bucket label. */
QString sexoLabel(const QString &key)
{
    if (key == QLatin1String("M"))
        return QStringLiteral("Masculino");
    if (key == QLatin1String("F"))
        return QStringLiteral("Femenino");
    return QStringLiteral("Sin dato");
}

/** "EN_PROCESO" -> "En proceso" for state/estado keys. */
QString prettyKey(QString key)
{
    key = key.toLower();
    key.replace(QLatin1Char('_'), QLatin1Char(' '));
    if (!key.isEmpty())
        key[0] = key.at(0).toUpper();
    return key;
}

Card *makeStatCard(const QString &caption, QLabel *&valueLabel, QWidget *parent)
{
    auto *card = new Card(parent);
    card->body()->setSpacing(Theme::SpacingXs);

    valueLabel = new QLabel(QStringLiteral("—"), card);
    valueLabel->setObjectName(QStringLiteral("PageTitle"));

    auto *captionLabel = new QLabel(caption, card);
    captionLabel->setObjectName(QStringLiteral("PageSubtitle"));

    card->addWidget(valueLabel);
    card->addWidget(captionLabel);
    return card;
}

/** Card with a title and a vertical host returned through `bars`. */
Card *makeBarCard(const QString &title, QVBoxLayout *&bars, QWidget *parent)
{
    auto *card = new Card(parent);
    card->body()->setSpacing(Theme::SpacingSm);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("CardTitle"));
    titleLabel->setWordWrap(true);

    auto *host = new QWidget(card);
    bars = new QVBoxLayout(host);
    bars->setContentsMargins(0, 0, 0, 0);
    bars->setSpacing(Theme::SpacingSm);

    card->addWidget(titleLabel);
    card->addWidget(host);
    return card;
}

} // namespace

ReportesPage::ReportesPage(ApiClient *api, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_dateFilter(new QCheckBox(QStringLiteral("Filtrar por fecha"), this))
    , m_desde(new QDateEdit(QDate(QDate::currentDate().year(), 1, 1), this))
    , m_hasta(new QDateEdit(QDate::currentDate(), this))
    , m_refreshButton(new QPushButton(QStringLiteral("Actualizar"), this))
    , m_status(new QLabel(this))
    , m_loading(new QLabel(QStringLiteral("Cargando reportes..."), this))
    , m_content(new QWidget(this))
    , m_totalValue(nullptr)
    , m_descartadasValue(nullptr)
    , m_sexoBars(nullptr)
    , m_productoBars(nullptr)
    , m_turnosBars(nullptr)
{
    // --- Header --------------------------------------------------------------
    auto *header = new PageHeader(QStringLiteral("Reportes"),
                                  QStringLiteral("Resumen operativo del banco de sangre"), this);

    // --- Filters -------------------------------------------------------------
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_refreshButton->setProperty("variant", "primary");
    m_desde->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_desde->setCalendarPopup(true);
    m_hasta->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_hasta->setCalendarPopup(true);
    m_desde->setEnabled(false);
    m_hasta->setEnabled(false);

    auto *filtersRow = new QHBoxLayout;
    filtersRow->setSpacing(Theme::SpacingSm);
    filtersRow->addWidget(m_dateFilter);
    filtersRow->addWidget(m_desde);
    auto *toLabel = new QLabel(QStringLiteral("a"), this);
    toLabel->setObjectName(QStringLiteral("InfoText"));
    filtersRow->addWidget(toLabel);
    filtersRow->addWidget(m_hasta);
    filtersRow->addStretch();
    filtersRow->addWidget(m_refreshButton);

    auto *filtersCard = new Card(this);
    filtersCard->addLayout(filtersRow);

    m_status->setObjectName(QStringLiteral("ErrorText"));
    m_status->setWordWrap(true);
    m_status->hide();

    m_loading->setObjectName(QStringLiteral("LoadingState"));
    m_loading->setAlignment(Qt::AlignCenter);

    // --- Content -------------------------------------------------------------
    auto *contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(Theme::SpacingMd);

    auto *statsRow = new QHBoxLayout;
    statsRow->setSpacing(Theme::SpacingMd);
    statsRow->addWidget(makeStatCard(QStringLiteral("Total de donaciones"), m_totalValue, m_content), 1);
    statsRow->addWidget(makeStatCard(QStringLiteral("Donaciones descartadas"), m_descartadasValue, m_content), 1);

    auto *chartsRow = new QHBoxLayout;
    chartsRow->setSpacing(Theme::SpacingMd);
    chartsRow->addWidget(makeBarCard(QStringLiteral("Donaciones por sexo"), m_sexoBars, m_content), 1);
    chartsRow->addWidget(makeBarCard(QStringLiteral("Donaciones por producto"), m_productoBars, m_content), 1);
    chartsRow->addWidget(makeBarCard(QStringLiteral("Turnos por estado"), m_turnosBars, m_content), 1);

    contentLayout->addLayout(statsRow);
    contentLayout->addLayout(chartsRow);
    contentLayout->addStretch(1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addWidget(filtersCard);
    layout->addWidget(m_status);
    layout->addWidget(m_loading);
    layout->addWidget(m_content, 1);

    connect(m_refreshButton, &QPushButton::clicked, this, &ReportesPage::refresh);
    connect(m_dateFilter, &QCheckBox::toggled, this, &ReportesPage::onFilterToggled);

    m_content->hide();
}

void ReportesPage::refresh()
{
    loadSummary();
}

void ReportesPage::onFilterToggled()
{
    const bool enabled = m_dateFilter->isChecked();
    m_desde->setEnabled(enabled);
    m_hasta->setEnabled(enabled);
}

void ReportesPage::loadSummary()
{
    QString path = QStringLiteral("/api/reportes/resumen");
    if (m_dateFilter->isChecked()) {
        path += QStringLiteral("?desde=%1&hasta=%2")
                    .arg(m_desde->date().toString(Qt::ISODate),
                         m_hasta->date().toString(Qt::ISODate));
    }

    setBusy(true);
    m_status->hide();
    m_content->hide();
    m_loading->show();

    m_api->get(path, [this](const ApiClient::Response &response) {
        setBusy(false);
        m_loading->hide();

        if (!response.ok) {
            m_status->setText(response.errorMessage);
            m_status->show();
            return;
        }

        const QJsonObject summary = response.json.object();
        m_totalValue->setText(QString::number(summary.value(QStringLiteral("total_donaciones")).toInt()));
        m_descartadasValue->setText(QString::number(summary.value(QStringLiteral("donaciones_descartadas")).toInt()));

        const QJsonObject porSexo = summary.value(QStringLiteral("por_sexo")).toObject();
        QList<QPair<QString, int>> sexoBars;
        for (const QString &key : {QStringLiteral("M"), QStringLiteral("F"), QStringLiteral("sin_dato")})
            sexoBars.append({sexoLabel(key), porSexo.value(key).toInt()});
        renderBars(m_sexoBars, sexoBars);

        QList<QPair<QString, int>> productoBars;
        const QJsonArray porProducto = summary.value(QStringLiteral("por_producto")).toArray();
        for (const QJsonValue &value : porProducto) {
            const QJsonObject row = value.toObject();
            productoBars.append({row.value(QStringLiteral("codigo")).toString(),
                                 row.value(QStringLiteral("total")).toInt()});
        }
        renderBars(m_productoBars, productoBars);

        QList<QPair<QString, int>> turnoBars;
        const QJsonObject turnos = summary.value(QStringLiteral("turnos_por_estado")).toObject();
        for (auto it = turnos.constBegin(); it != turnos.constEnd(); ++it)
            turnoBars.append({prettyKey(it.key()), it.value().toInt()});
        renderBars(m_turnosBars, turnoBars);

        m_content->show();
    });
}

void ReportesPage::renderBars(QVBoxLayout *target, const QList<QPair<QString, int>> &bars)
{
    clearLayout(target);

    if (bars.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("Sin datos en el período."), target->parentWidget());
        empty->setObjectName(QStringLiteral("CardText"));
        target->addWidget(empty);
        return;
    }

    int maximum = 1;
    for (const auto &bar : bars)
        maximum = qMax(maximum, bar.second);

    for (const auto &bar : bars) {
        auto *row = new QWidget(target->parentWidget());
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(Theme::SpacingSm);

        auto *name = new QLabel(bar.first, row);
        name->setObjectName(QStringLiteral("CardText"));
        name->setFixedWidth(kLabelWidth);

        auto *bar2 = new QProgressBar(row);
        bar2->setRange(0, maximum);
        bar2->setValue(bar.second);
        bar2->setTextVisible(false);

        auto *value = new QLabel(QString::number(bar.second), row);
        value->setObjectName(QStringLiteral("CardText"));
        value->setFixedWidth(kValueWidth);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rowLayout->addWidget(name);
        rowLayout->addWidget(bar2, 1);
        rowLayout->addWidget(value);

        target->addWidget(row);
    }
}

void ReportesPage::setBusy(bool busy)
{
    m_dateFilter->setEnabled(!busy);
    m_desde->setEnabled(!busy && m_dateFilter->isChecked());
    m_hasta->setEnabled(!busy && m_dateFilter->isChecked());
    m_refreshButton->setEnabled(!busy);
    setCursor(busy ? Qt::BusyCursor : Qt::ArrowCursor);
}
