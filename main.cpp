#include "mainwindow.h"
#include "bass.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (!BASS_Init(-1, 44100, BASS_DEVICE_16BITS | BASS_DEVICE_STEREO, 0, NULL)) {
        QMessageBox msgBox;
        msgBox.setText("No audio devices found!");
        return msgBox.exec();
    }

    BASS_SetConfig(BASS_CONFIG_SRC, 16);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
