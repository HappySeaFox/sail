#include <QApplication>

#include "qtsail.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QtSail qt;

    qt.resize(800, 500);
    qt.show();

    return app.exec();
}
