#pragma once

#include <QWidget>

/** Simple "under construction" page used for sections not yet implemented. */
class PlaceholderPage : public QWidget
{
    Q_OBJECT

public:
    PlaceholderPage(const QString &title, const QString &message, QWidget *parent = nullptr);
};
