#include <QCoreApplication>
#include <ghostc.h>

int main(int argc, char *argv[]) {
    QCA::Initializer init;
    QCoreApplication a(argc, argv);

    QStringList args = a.arguments();
    GhostC *ghost = new GhostC(&args);

    if (args.count() > 1) {
        ghost->process();
    } else {
        qDebug("ERROR: No argument given.\n");
        ghost->printHelp();
    } //endif

    return 0;
} //endfunction main
