#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainterPath>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMouseEvent>
#include <QMouseEventTransition>
#include <QFileDialog>
#include <QPainter>
#include <QLabel>
#include <QSlider>
#include <QKeyEvent>
#include <QTimer>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QFontDatabase>
#include <QFont>

#include <iostream>
#include <vector>
#include <math.h>

#include "bass.h"
#include "song.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void test();

    bool openFile ();
    void setActive(QListWidgetItem *);
    void setActive(int index);

    void backward();
    void forward();
    void pause();
    void changeVolume(int vol);
    void updateTime();

    void changeRepeat () {
        repeat = !repeat;
        cout << "Repeat - " << repeat << endl;
    }

private:
    HSTREAM channel;

    bool paused = true;
    bool repeat = false;
    bool shuffle = false;
    bool liveSpec = false;

    float volume = 1;
    float prerenderedFft[1024];

    vector <Song> playlist;
    vector <Song>::iterator current;

    QListWidget * playlistWidget;
    QLabel * songTitle;
    QLabel * songDuration;
    QLabel * songPosition;
    QLabel * timecode;
    QSlider * volumeSlider;

    QPushButton * repeatBtn;
    QPushButton * pauseBtn;

    QTimer * timer;

    Ui::MainWindow *ui;

    void drawPlaylist();
    void setTitle();
    void prerenderFft ();

    void paintEvent(QPaintEvent * event);
    void keyPressEvent(QKeyEvent * event) {};
    void mousePressEvent (QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent * event);

    string seconds2string (float seconds);

    float getDuration () {
        QWORD len = BASS_ChannelGetLength(channel, BASS_POS_BYTE); // the length in bytes
        return BASS_ChannelBytes2Seconds(channel, len); // the length in seconds
    }
    float getPosition () {
        QWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE);
        return BASS_ChannelBytes2Seconds(channel, pos);
    }

};
#endif // MAINWINDOW_H
