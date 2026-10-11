#include "ui/IconFactory.h"

#include "ui/Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace {

/** All glyphs are authored on this grid and scaled to the requested size. */
constexpr qreal kGrid = 24.0;

/** Single stroke style shared by the whole set, expressed in grid units. */
constexpr qreal kStrokeWidth = 2.1;

/** A person figure: a head circle plus an open shoulder arch. */
void addPerson(QPainterPath &path, qreal cx, qreal headCy, qreal headRadius, qreal baseline)
{
    path.addEllipse(QPointF(cx, headCy), headRadius, headRadius);

    const qreal shoulderRadius = headRadius * 1.75;
    const qreal archTop = headCy + headRadius + 1.5;
    const QRectF arch(cx - shoulderRadius, archTop, shoulderRadius * 2.0, shoulderRadius * 2.0);

    QPainterPath body;
    body.moveTo(cx - shoulderRadius, baseline);
    body.arcTo(arch, 180, -180);
    body.lineTo(cx + shoulderRadius, baseline);
    path.addPath(body);
}

QPainterPath homePath()
{
    QPainterPath path;
    // Roof.
    path.moveTo(3.5, 11.5);
    path.lineTo(12.0, 4.5);
    path.lineTo(20.5, 11.5);
    // Walls.
    path.moveTo(5.5, 10.5);
    path.lineTo(5.5, 19.5);
    path.lineTo(18.5, 19.5);
    path.lineTo(18.5, 10.5);
    // Door.
    path.moveTo(9.75, 19.5);
    path.lineTo(9.75, 14.5);
    path.lineTo(14.25, 14.5);
    path.lineTo(14.25, 19.5);
    return path;
}

QPainterPath peoplePath()
{
    QPainterPath path;
    // Back person first so the front one overlaps it.
    addPerson(path, 16.3, 8.4, 2.6, 19.8);
    addPerson(path, 9.0, 8.2, 3.3, 19.8);
    return path;
}

QPainterPath calendarPath()
{
    QPainterPath path;
    path.addRoundedRect(QRectF(3.5, 5.0, 17.0, 15.5), 2.0, 2.0);
    // Header separator.
    path.moveTo(3.5, 9.5);
    path.lineTo(20.5, 9.5);
    // Hangers.
    path.moveTo(8.0, 3.5);
    path.lineTo(8.0, 6.5);
    path.moveTo(16.0, 3.5);
    path.lineTo(16.0, 6.5);
    // Day marks.
    path.moveTo(7.5, 13.0);
    path.lineTo(10.0, 13.0);
    path.moveTo(12.5, 13.0);
    path.lineTo(15.0, 13.0);
    path.moveTo(7.5, 16.5);
    path.lineTo(10.0, 16.5);
    path.moveTo(12.5, 16.5);
    path.lineTo(15.0, 16.5);
    return path;
}

QPainterPath aptitudPath()
{
    QPainterPath path;
    // Shield outline.
    path.moveTo(12.0, 3.5);
    path.lineTo(19.5, 6.5);
    path.lineTo(19.5, 11.5);
    path.cubicTo(19.5, 16.3, 16.2, 19.6, 12.0, 20.7);
    path.cubicTo(7.8, 19.6, 4.5, 16.3, 4.5, 11.5);
    path.lineTo(4.5, 6.5);
    path.closeSubpath();
    // Check inside the shield (outline, not a filled success mark).
    path.moveTo(8.6, 12.1);
    path.lineTo(11.2, 14.7);
    path.lineTo(15.7, 9.6);
    return path;
}

QPainterPath donationPath()
{
    QPainterPath path;
    path.moveTo(12.0, 3.5);
    path.cubicTo(12.0, 3.5, 18.5, 11.3, 18.5, 15.0);
    // Rounded bottom, from the right side around to the left.
    path.arcTo(QRectF(5.5, 8.5, 13.0, 13.0), 0, -180);
    path.cubicTo(5.5, 11.3, 12.0, 3.5, 12.0, 3.5);
    path.closeSubpath();
    return path;
}

QPainterPath reportPath()
{
    QPainterPath path;
    // Axis.
    path.moveTo(4.0, 20.0);
    path.lineTo(20.0, 20.0);
    // Bars.
    path.moveTo(7.0, 20.0);
    path.lineTo(7.0, 13.5);
    path.moveTo(12.0, 20.0);
    path.lineTo(12.0, 8.5);
    path.moveTo(17.0, 20.0);
    path.lineTo(17.0, 5.0);
    return path;
}

QPainterPath pathFor(IconFactory::Section section)
{
    switch (section) {
    case IconFactory::Section::Inicio:
        return homePath();
    case IconFactory::Section::Pacientes:
        return peoplePath();
    case IconFactory::Section::Turnos:
        return calendarPath();
    case IconFactory::Section::Aptitud:
        return aptitudPath();
    case IconFactory::Section::Donaciones:
        return donationPath();
    case IconFactory::Section::Reportes:
        return reportPath();
    }
    return {};
}

} // namespace

namespace IconFactory {

QPixmap pixmap(Section section, int size, const QColor &color)
{
    QPixmap result(size, size);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.scale(size / kGrid, size / kGrid);

    QPen pen(color);
    pen.setWidthF(kStrokeWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(pathFor(section));

    return result;
}

QIcon navIcon(Section section)
{
    const QColor normalColor = Theme::color(Theme::TextSecondary);
    const QColor selectedColor = Theme::color(Theme::Primary);

    QIcon icon;
    for (int size : {16, 20, 24, 32}) {
        icon.addPixmap(pixmap(section, size, normalColor), QIcon::Normal);
        icon.addPixmap(pixmap(section, size, selectedColor), QIcon::Selected);
    }
    return icon;
}

} // namespace IconFactory
