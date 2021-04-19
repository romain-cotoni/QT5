#include <cnc.h>

int main (int argc, char** argv)
{
    qputenv("QT_STYLE_OVERRIDE",""); //peut être pour corriger un bug sous LINUX-MINT
    QApplication app(argc,argv);
    //app.setOverrideCursor(QCursor((Qt::CrossCursor)));
    CNC instance;
    instance.show();

    return app.exec();
}
