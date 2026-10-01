#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("RVM Serial Client"));
    QApplication::setOrganizationName(QStringLiteral("RVM"));

    if (QStyleFactory::keys().contains(QStringLiteral("Fusion"))) {
        QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }

    MainWindow window;
    window.show();
    return app.exec();
}
