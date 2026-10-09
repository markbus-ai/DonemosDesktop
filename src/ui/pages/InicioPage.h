#pragma once

#include <QWidget>

class InicioPage : public QWidget
{
    Q_OBJECT

public:
    explicit InicioPage(const QString &username, QWidget *parent = nullptr);
};
