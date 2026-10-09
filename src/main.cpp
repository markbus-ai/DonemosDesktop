#include <QApplication>

#include "app/AppController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("DonemosDesktop"));
    QApplication::setOrganizationName(QStringLiteral("Donemos"));

    AppController controller;
    controller.start();

    return app.exec();
}
