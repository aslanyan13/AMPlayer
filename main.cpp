#include "mainwindow.h"
#include "bass.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <iostream>

int main(int argc, char *argv[])
{
    srand(time(NULL));

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    a.setApplicationDisplayName("AMPlayer");
    a.setApplicationName("AMPlayer");
    a.setDesktopFileName("AMPlayer");
    a.setObjectName("AMPlayer");
    a.setStartDragTime(1000);
    a.setQuitOnLastWindowClosed(false);

    for (int i = 1; i <= 5; i++)
        a.setEffectEnabled((Qt::UIEffect)i, true);

    if (!BASS_Init(-1, 96000, BASS_DEVICE_16BITS | BASS_DEVICE_STEREO, 0, NULL)) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText("No audio devices found!");
        msgBox.setStyleSheet("background-color: #141414; color: silver;");
        msgBox.setInformativeText("Continue without audio?");
        int ret = msgBox.exec();

        switch (ret) {
          case QMessageBox::No:
              exit(0);
              break;
          default:
              break;
        }
    }

    BASS_SetConfig(BASS_CONFIG_SRC, 16);

    MainWindow w;

    return a.exec();
}
