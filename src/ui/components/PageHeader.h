#pragma once

#include <QWidget>

class QHBoxLayout;

/**
 * Page header: title (with optional subtitle) on the left, action widgets on
 * the right. Reused by every real page so titles and action placement stay
 * consistent across the app.
 */
class PageHeader : public QWidget
{
    Q_OBJECT

public:
    explicit PageHeader(const QString &title,
                        const QString &subtitle = QString(),
                        QWidget *parent = nullptr);

    /** Append an action widget (usually a button) to the right-hand side. */
    void addAction(QWidget *widget);

private:
    QHBoxLayout *m_actions;
};
