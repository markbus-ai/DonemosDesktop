#pragma once

#include <QWidget>

class QVBoxLayout;

/**
 * White surface container with a subtle border and rounded corners.
 *
 * Use `body()` (or the add* helpers) to lay out content inside the card.
 */
class Card : public QWidget
{
    Q_OBJECT

public:
    explicit Card(QWidget *parent = nullptr);

    QVBoxLayout *body() const;
    void addWidget(QWidget *widget);
    void addLayout(QLayout *layout);

private:
    QVBoxLayout *m_body;
};
