#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // fusion style is good (i guess... :/)
    QApplication::setStyle("Fusion");
    app.setApplicationName("FGDBuilder");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("FGDBuilder");
    MainWindow window;
    window.show();
    return app.exec();
}
