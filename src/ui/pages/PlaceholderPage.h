#pragma once

#include "ui/IconFactory.h"

#include <QWidget>

/**
 * "Coming soon" page used for sections not yet implemented. Renders a centered
 * card with an icon, title, explanation and a subtle badge instead of a blank
 * area.
 */
class PlaceholderPage : public QWidget
{
    Q_OBJECT

public:
    PlaceholderPage(const QString &title,
                    const QString &message,
                    IconFactory::Section section,
                    QWidget *parent = nullptr);
};
