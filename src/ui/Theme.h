#pragma once

#include <QColor>
#include <QPalette>
#include <QString>

/**
 * Brand design tokens for the Donemos desktop client.
 *
 * Mirrors the palette used by the mobile app so both clients share one visual
 * identity. `resources/style.qss` mirrors these values for declarative styling;
 * this header is the source of truth for any color needed from C++.
 */
namespace Theme {

// --- Colors ----------------------------------------------------------------
inline constexpr const char *Primary = "#E5484D";
inline constexpr const char *PrimaryDark = "#C5373C";
inline constexpr const char *PrimaryTint = "#FDECEC";

inline constexpr const char *Background = "#FFFFFF";
inline constexpr const char *Surface = "#F2F2F7";
inline constexpr const char *SurfacePressed = "#E5E5EA";

inline constexpr const char *TextPrimary = "#1C1C1E";
inline constexpr const char *TextSecondary = "#8E8E93";
inline constexpr const char *TextOnPrimary = "#FFFFFF";

inline constexpr const char *Divider = "#E5E5EA";

inline constexpr const char *Success = "#30A46C";
inline constexpr const char *Warning = "#F5A524";
inline constexpr const char *Error = "#E5484D";

// --- Spacing scale (px) ----------------------------------------------------
inline constexpr int SpacingXs = 4;
inline constexpr int SpacingSm = 8;
inline constexpr int SpacingMd = 16;
inline constexpr int SpacingLg = 24;
inline constexpr int SpacingXl = 32;
inline constexpr int SpacingXxl = 48;

// --- Radius (px) -----------------------------------------------------------
inline constexpr int RadiusSm = 8;
inline constexpr int RadiusMd = 12;

/** QColor from one of the hex constants above. */
inline QColor color(const char *hex)
{
    return QColor(QString::fromLatin1(hex));
}

/** Light application palette that overrides any dark OS theme. */
QPalette lightPalette();

/**
 * Force the light branded look: Fusion style, light palette and an explicit
 * light color scheme. Call before creating widgets and before setting the
 * application stylesheet.
 */
void applyLightTheme();

} // namespace Theme
