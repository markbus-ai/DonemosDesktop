#include "ui/pages/InicioPage.h"

#include "ui/Theme.h"
#include "ui/components/Card.h"
#include "ui/components/PageHeader.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <functional>

namespace {

/** Small quick-link card with a title, description and a navigation button. */
Card *makeQuickLink(const QString &title,
                    const QString &description,
                    const QString &buttonText,
                    int pageIndex,
                    QWidget *parent,
                    const std::function<void(int)> &navigate)
{
    auto *card = new Card(parent);
    card->body()->setSpacing(Theme::SpacingSm);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("CardTitle"));
    auto *descriptionLabel = new QLabel(description, card);
    descriptionLabel->setObjectName(QStringLiteral("CardText"));
    descriptionLabel->setWordWrap(true);

    auto *button = new QPushButton(buttonText, card);
    button->setCursor(Qt::PointingHandCursor);
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    QObject::connect(button, &QPushButton::clicked, card, [navigate, pageIndex]() {
        navigate(pageIndex);
    });

    card->addWidget(titleLabel);
    card->addWidget(descriptionLabel);
    card->body()->addStretch();
    card->body()->addWidget(button, 0, Qt::AlignLeft);

    return card;
}

} // namespace

InicioPage::InicioPage(const QString &username, QWidget *parent)
    : QWidget(parent)
{
    auto *header = new PageHeader(QStringLiteral("Inicio"),
                                  QStringLiteral("Resumen general del sistema"), this);

    // --- Welcome card --------------------------------------------------------
    auto *welcome = new Card(this);
    auto *welcomeTitle = new QLabel(QStringLiteral("Hola, %1").arg(username), welcome);
    welcomeTitle->setObjectName(QStringLiteral("CardTitle"));
    auto *welcomeText = new QLabel(
        QStringLiteral("Bienvenido al sistema de banco de sangre. Use el menú lateral "
                       "o los accesos rápidos para administrar pacientes y turnos."),
        welcome);
    welcomeText->setObjectName(QStringLiteral("CardText"));
    welcomeText->setWordWrap(true);
    welcome->addWidget(welcomeTitle);
    welcome->addWidget(welcomeText);

    // --- Quick links ---------------------------------------------------------
    const auto navigate = [this](int index) { emit navigateRequested(index); };
    auto *pacientesCard = makeQuickLink(QStringLiteral("Pacientes"),
                                        QStringLiteral("Busque donantes, registre nuevos y "
                                                       "consulte su ficha y aptitud."),
                                        QStringLiteral("Ver pacientes"),
                                        1, this, navigate);
    auto *turnosCard = makeQuickLink(QStringLiteral("Turnos"),
                                     QStringLiteral("Consulte la agenda y agende nuevas "
                                                    "donaciones."),
                                     QStringLiteral("Ver turnos"),
                                     2, this, navigate);

    auto *quickLinks = new QHBoxLayout;
    quickLinks->setSpacing(Theme::SpacingMd);
    quickLinks->addWidget(pacientesCard, 1);
    quickLinks->addWidget(turnosCard, 1);

    // --- Modules in preparation ---------------------------------------------
    auto *modules = new Card(this);
    auto *modulesTitle = new QLabel(QStringLiteral("Módulos en preparación"), modules);
    modulesTitle->setObjectName(QStringLiteral("CardTitle"));
    auto *modulesText = new QLabel(
        QStringLiteral("Aptitud, Donaciones y Reportes estarán disponibles en una próxima versión."),
        modules);
    modulesText->setObjectName(QStringLiteral("CardText"));
    modulesText->setWordWrap(true);
    modules->addWidget(modulesTitle);
    modules->addWidget(modulesText);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addWidget(welcome);
    layout->addLayout(quickLinks);
    layout->addWidget(modules);
    layout->addStretch();
}
