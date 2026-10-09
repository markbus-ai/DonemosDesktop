#include "ui/components/Card.h"

#include "ui/Theme.h"

#include <QLayout>
#include <QVBoxLayout>

Card::Card(QWidget *parent)
    : QWidget(parent)
    , m_body(new QVBoxLayout(this))
{
    setObjectName(QStringLiteral("Card"));
    setAttribute(Qt::WA_StyledBackground, true);
    m_body->setContentsMargins(Theme::SpacingMd, Theme::SpacingMd,
                               Theme::SpacingMd, Theme::SpacingMd);
    m_body->setSpacing(Theme::SpacingSm);
}

QVBoxLayout *Card::body() const
{
    return m_body;
}

void Card::addWidget(QWidget *widget)
{
    m_body->addWidget(widget);
}

void Card::addLayout(QLayout *layout)
{
    m_body->addLayout(layout);
}
