#include "mainwindow.h"
#include "bass.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

int main(int argc, char *argv[])
{
    srand(time(NULL));

    QApplication a(argc, argv);

    if (!BASS_Init(-1, 44100, BASS_DEVICE_16BITS | BASS_DEVICE_STEREO, 0, NULL)) {
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
    w.show();

    return a.exec();
}
