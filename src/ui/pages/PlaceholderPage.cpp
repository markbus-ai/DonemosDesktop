#include "ui/pages/PlaceholderPage.h"

#include "ui/Theme.h"
#include "ui/components/Card.h"
#include "ui/components/PageHeader.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

PlaceholderPage::PlaceholderPage(const QString &title,
                                 const QString &message,
                                 IconFactory::Section section,
                                 QWidget *parent)
    : QWidget(parent)
{
    auto *header = new PageHeader(title, QStringLiteral("Módulo en preparación"), this);

    auto *card = new Card(this);
    card->setMaximumWidth(520);
    card->body()->setSpacing(Theme::SpacingSm);

    auto *iconLabel = new QLabel(card);
    // Muted tint: the module is not available yet, never a success/green mark.
    iconLabel->setPixmap(IconFactory::pixmap(section, 48, Theme::color(Theme::TextSecondary)));
    iconLabel->setAlignment(Qt::AlignHCenter);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("CardTitle"));
    titleLabel->setAlignment(Qt::AlignHCenter);

    auto *messageLabel = new QLabel(message, card);
    messageLabel->setObjectName(QStringLiteral("CardText"));
    messageLabel->setAlignment(Qt::AlignHCenter);
    messageLabel->setWordWrap(true);

    auto *badge = new QLabel(QStringLiteral("Disponible próximamente"), card);
    badge->setObjectName(QStringLiteral("ComingSoonBadge"));
    badge->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    auto *badgeRow = new QHBoxLayout;
    badgeRow->addStretch();
    badgeRow->addWidget(badge);
    badgeRow->addStretch();

    card->body()->addSpacing(Theme::SpacingSm);
    card->addWidget(iconLabel);
    card->addWidget(titleLabel);
    card->addWidget(messageLabel);
    card->body()->addSpacing(Theme::SpacingSm);
    card->addLayout(badgeRow);
    card->body()->addSpacing(Theme::SpacingSm);

    auto *cardRow = new QHBoxLayout;
    cardRow->addStretch();
    cardRow->addWidget(card);
    cardRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SpacingLg, Theme::SpacingLg,
                               Theme::SpacingLg, Theme::SpacingLg);
    layout->setSpacing(Theme::SpacingMd);
    layout->addWidget(header);
    layout->addStretch();
    layout->addLayout(cardRow);
    layout->addStretch();
}
