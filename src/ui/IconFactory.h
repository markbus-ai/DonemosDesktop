#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>

/**
 * Programmatic monochrome icon set for the app sections.
 *
 * The QStyle standard icons are inconsistent (mixed colors and styles), so the
 * sidebar and the "coming soon" cards draw their own glyphs instead. Every icon
 * is authored on a 24x24 grid as a stroked path and rendered at any size into a
 * transparent QPixmap, so the whole set shares one line style and one tint.
 *
 * No icon fonts, SVG files or extra libraries: QPainter is the only dependency.
 */
namespace IconFactory {

/** Sidebar/game sections that own an icon. */
enum class Section {
    Inicio,
    Pacientes,
    Turnos,
    Aptitud,
    Donaciones,
    Reportes,
};

/** Render the section glyph at `size` px, stroked with `color`. */
QPixmap pixmap(Section section, int size, const QColor &color);

/**
 * Nav icon carrying both a normal (Theme::TextSecondary) and a selected
 * (Theme::Primary) pixmap, so QListWidget swaps the tint automatically when the
 * row becomes selected without any extra wiring.
 */
QIcon navIcon(Section section);

} // namespace IconFactory
