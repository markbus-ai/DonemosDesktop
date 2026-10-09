#include "ui/pages/PlaceholderPage.h"

#include <QLabel>
#include <QVBoxLayout>

PlaceholderPage::PlaceholderPage(const QString &title, const QString &message, QWidget *parent)
    : QWidget(parent)
{
    auto *titleLabel = new QLabel(title, this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *messageLabel = new QLabel(message, this);
    messageLabel->setStyleSheet(QStringLiteral("color: #666;"));

    auto *badge = new QLabel(QStringLiteral("En construcción"), this);
    badge->setStyleSheet(QStringLiteral(
        "color: #8a6d3b; background: #fcf8e3; border: 1px solid #faebcc;"
        "border-radius: 4px; padding: 4px 10px;"));
    badge->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(12);
    layout->addWidget(titleLabel);
    layout->addWidget(messageLabel);
    layout->addWidget(badge);
    layout->addStretch();
}
