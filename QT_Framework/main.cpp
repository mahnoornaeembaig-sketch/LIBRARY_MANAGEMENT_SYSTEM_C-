#include <QApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include "mainwindow.h"
#include "user.h"

int User::nextRecordID = 0;
int User::nextHoldID   = 0;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Get the folder where the .exe is located
    QString appDir = QCoreApplication::applicationDirPath();

    // Load theme relative to the .exe
    QFile styleFile(appDir + "/theme.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
        styleFile.close();
    }

    MainWindow w;
    w.show();
    return a.exec();
}
