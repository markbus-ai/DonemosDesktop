#include "ui/pages/InicioPage.h"

#include <QLabel>
#include <QVBoxLayout>

InicioPage::InicioPage(const QString &username, QWidget *parent)
    : QWidget(parent)
{
    auto *title = new QLabel(QStringLiteral("Inicio"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *welcome = new QLabel(QStringLiteral("Hola, %1.").arg(username), this);
    QFont welcomeFont = welcome->font();
    welcomeFont.setPointSize(welcomeFont.pointSize() + 2);
    welcome->setFont(welcomeFont);

    auto *hint = new QLabel(
        QStringLiteral("Usá el menú lateral para administrar pacientes y turnos.\n"
                       "Aptitud, Donaciones y Reportes están en construcción."),
        this);
    hint->setStyleSheet(QStringLiteral("color: #666;"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addWidget(welcome);
    layout->addWidget(hint);
    layout->addStretch();
}
