#include "ui/components/PageHeader.h"

#include "ui/Theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

PageHeader::PageHeader(const QString &title, const QString &subtitle, QWidget *parent)
    : QWidget(parent)
    , m_actions(new QHBoxLayout)
{
    auto *titleLabel = new QLabel(title, this);
    titleLabel->setObjectName(QStringLiteral("PageTitle"));

    auto *textColumn = new QVBoxLayout;
    textColumn->setContentsMargins(0, 0, 0, 0);
    textColumn->setSpacing(2);
    textColumn->addWidget(titleLabel);

    if (!subtitle.isEmpty()) {
        auto *subtitleLabel = new QLabel(subtitle, this);
        subtitleLabel->setObjectName(QStringLiteral("PageSubtitle"));
        textColumn->addWidget(subtitleLabel);
    }

    m_actions->setContentsMargins(0, 0, 0, 0);
    m_actions->setSpacing(Theme::SpacingSm);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::SpacingMd);
    layout->addLayout(textColumn);
    layout->addStretch();
    layout->addLayout(m_actions, 0);
}

void PageHeader::addAction(QWidget *widget)
{
    m_actions->addWidget(widget);
}
