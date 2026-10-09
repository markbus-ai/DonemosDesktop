#pragma once

#include <QWidget>

/**
 * Landing page: a welcome card plus quick links into the main sections, so the
 * first screen the staff sees is never empty.
 */
class InicioPage : public QWidget
{
    Q_OBJECT

public:
    explicit InicioPage(const QString &username, QWidget *parent = nullptr);

signals:
    /** Request navigation to a sidebar page index. */
    void navigateRequested(int index);
};
