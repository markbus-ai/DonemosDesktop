#include <QApplication>
#include <QFile>
#include <QString>

#include "app/AppController.h"
#include "ui/Theme.h"

namespace {

/** Load the bundled brand stylesheet from the Qt resource system. */
QString loadStyleSheet()
{
    QFile file(QStringLiteral(":/style.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(file.readAll());
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("DonemosDesktop"));
    QApplication::setOrganizationName(QStringLiteral("Donemos"));

    // Force the light branded look before any widget is created.
    Theme::applyLightTheme();
    app.setStyleSheet(loadStyleSheet());

    AppController controller;
    controller.start();

    return app.exec();
}
