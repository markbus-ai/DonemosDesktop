#include "ui/Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QStyleFactory>
#include <QStyleHints>

namespace Theme {

QPalette lightPalette()
{
    QPalette palette;

    const QColor primary = color(Primary);
    const QColor surface = color(Surface);
    const QColor background = color(Background);
    const QColor textPrimary = color(TextPrimary);
    const QColor textSecondary = color(TextSecondary);
    const QColor divider = color(Divider);

    palette.setColor(QPalette::Window, surface);
    palette.setColor(QPalette::WindowText, textPrimary);
    palette.setColor(QPalette::Base, background);
    palette.setColor(QPalette::AlternateBase, surface);
    palette.setColor(QPalette::Text, textPrimary);
    palette.setColor(QPalette::PlaceholderText, textSecondary);
    palette.setColor(QPalette::Button, background);
    palette.setColor(QPalette::ButtonText, textPrimary);
    palette.setColor(QPalette::Highlight, primary);
    palette.setColor(QPalette::HighlightedText, color(TextOnPrimary));
    palette.setColor(QPalette::ToolTipBase, background);
    palette.setColor(QPalette::ToolTipText, textPrimary);
    palette.setColor(QPalette::Link, primary);
    palette.setColor(QPalette::Mid, divider);
    palette.setColor(QPalette::Disabled, QPalette::Text, textSecondary);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, textSecondary);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, textSecondary);

    return palette;
}

void applyLightTheme()
{
    // Fusion is bundled with Qt and renders predictably under any desktop.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setPalette(lightPalette());

    // Qt >= 6.8: pin the color scheme so a dark OS theme is never inherited.
    if (QStyleHints *hints = QGuiApplication::styleHints())
        hints->setColorScheme(Qt::ColorScheme::Light);
}

} // namespace Theme
