#include <QApplication>
#include "MainWindow.h"
#include "DiscordRPC.h"

static const int64_t DISCORD_CLIENT_ID = 1515443340865441892;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FGDBuilder");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("FGDBuilder");

    DiscordRPC::instance().init(DISCORD_CLIENT_ID);

    MainWindow window;
    window.show();

    int ret = app.exec();
    DiscordRPC::instance().shutdown();
    return ret;
}
