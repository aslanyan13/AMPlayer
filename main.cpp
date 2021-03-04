#include "mainwindow.h"
#include "bass.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    BASS_Init(-1, 44100, BASS_DEVICE_16BITS | BASS_DEVICE_STEREO, 0, NULL);
    BASS_SetConfig(BASS_CONFIG_SRC, 16);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
