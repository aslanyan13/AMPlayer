#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt libraries
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QPaintEvent>
#include <QSystemTrayIcon>
#include <QPainterPath>
#include <QListWidget>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QListWidgetItem>
#include <QFormLayout>
#include <QMenu>
#include <QCheckBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QMouseEventTransition>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QLabel>
#include <QSlider>
#include <QKeyEvent>
#include <QTimer>
#include <QImage>
#include <QImageReader>
#include <QShortcut>
#include <QPixmap>
#include <QFontDatabase>
#include <QFont>
#include <QScrollBar>
#include <QCommonStyle>
#include <QLineEdit>
#include <QDirIterator>
#include <QWheelEvent>
#include <QHBoxLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QCursor>

// Qt WebSocket libs
#include <QtWebSockets>

// Taglib library
#include <taglib/taglib.h>
#include <taglib/tfile.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>
#include <taglib/audioproperties.h>

// C++ libs
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <future>
#include <chrono>
#include <thread>

// Bass header files
#include "bass.h"
#include "bassopus.h"
#include "bass_ape.h"
#include "bass_fx.h"
#include "bassflac.h"
#include "basswebm.h"
#include "bass_spx.h"
#include "bass_tta.h"

// Custom header files
#include "song.h"
#include "settingswindow.h"
#include "playlistreader.h"
#include "equalizerwindow.h"
#include "visualizationwindow.h"
#include "infowidget.h"
#include "customslider.h"
#include "startwidget.h"

#include "fifo_map.hpp"

using namespace std;
using nlohmann::fifo_map;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void reorderPlaylist();
    void reloadStyles();

    MainWindow(QWidget * parent = nullptr);
    ~MainWindow();

private slots:
    bool openFile ();
    void removeFile();
    bool openFolder ();

    void createPlaylist();
    void removePlaylist(int index);

    void menuContext ();
    void trayContext ();
    void playlistsBarContextMenu (const QPoint&);
    void changeCurrentPlaylist (int index);

    void setActive(QListWidgetItem *);
    void setActive(int index);

    void search (const QString & text);

    void backward();
    void forward();
    void pause();
    void changeVolume(int vol);
    void changeRepeat ();
    void changeShuffle ();

    void makeLoop();
    void updateTime();

    void remoteDeviceConnect ();
    void getRemoteCommands (const QString & command);
    void httpNewConnection ();
    QString getLocalAddress () {
        QList<QHostAddress> list = QNetworkInterface::allAddresses();

        for (int nIter = 0; nIter<list.count(); nIter++)
        {
            if(!list[nIter].isLoopback())
                if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol && list[nIter].toString()[0] == '1')
                    return list[nIter].toString();
        }
    }

    void settings ();
    void trackTimer () {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Timer");
        msgBox.setText("Timer will be added soon!");
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();
    }
    void audio3D () {
        QMessageBox msgBox;
        msgBox.setWindowTitle("3D Audio");
        msgBox.setText("3D Audio functions will be added soon!");
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();
    }
    void equalizer () {
        equalizerWin->raise();
        equalizerWin->show();
        equalizerWin->move (this->pos().x() + (this->size().width() - equalizerWin->size().width()) / 2, this->pos().y() + (this->size().height() - equalizerWin->size().height()) / 2);
        equalizerWin->setFocus();
    }
    void visualizations ();

    void colorChange();

    void slot_minimize() {
        trayIcon->setVisible(true);
        this->hide();
    };
    void slot_close() {
        this->close();
    };
    void showWindow () {
        this->show();
        this->setFocus();
        trayIcon->setVisible(false);
    }

    void rowsMoved(QModelIndex, int, int, QModelIndex, int);

    void addMark();
    void removeMark();
    void editMark();
    void drawMarksList();

private:
    HSTREAM channel;

    void createStream(HSTREAM & chan, QString file, DWORD flags) {
        chan = BASS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
        if (BASS_ErrorGetCode())
            chan = BASS_FLAC_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
        if (BASS_ErrorGetCode())
            chan = BASS_OPUS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
        if (BASS_ErrorGetCode())
            chan = BASS_WEBM_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags, 0);

        if (BASS_ErrorGetCode()) {
            chan = BASS_APE_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp").exists())
                    QDir().mkdir("C:/amptemp");

                QFile::copy(file, "C:/amptemp/tmp.ape");
                chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp/tmp.ape", 0, 0, flags);
            }
        }
        if (BASS_ErrorGetCode()) {
            remove("C:/amptemp/tmp.ape");
            chan = BASS_SPX_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp").exists())
                    QDir().mkdir("C:/amptemp");

                QFile::copy(file, "C:/amptemp/tmp.spx");
                chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp/tmp.spx", 0, 0, flags);
            }
        }
        if (BASS_ErrorGetCode()) {
            remove("C:/amptemp/tmp.spx");
            chan = BASS_TTA_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp").exists())
                    QDir().mkdir("C:/amptemp");

                QFile::copy(file, "C:/amptemp/tmp.tta");
                chan = BASS_TTA_StreamCreateFile(false, "C:/amptemp/tmp.tta", 0, 0, flags);
            }
        }
    }

    bool looped = false;
    bool paused = true;
    bool repeat = false;
    bool shuffle = false;
    bool liveSpec = false; // Is live spectrum mode enabled
    bool colorChanging = false; // Is color changing mode enabled
    bool coverLoaded = true;
    bool coverBgBlur = true;
    bool logging = true;
    bool remoteServerEnabled = false;
    bool remoteScrolling = false;
    bool volumeSliderToggled = false;
    bool muted = false;
    bool configLoaded = false;
    bool visualWindowOpened = false;
    bool equoEnabled = false;

    float loopStart = -1;
    float loopEnd = -1;
    void removeLoop() {
        loopStart = -1;
        loopEnd = -1;
        looped = false;
    }

    float starttime;
    float volume = 1;
    float prerenderedFft[1024];
    float coverBgOpacity = 255;

    int httpServerPort;

    QWinTaskbarProgress * taskbarProgress;

    fifo_map <QString, vector <Song>> playlists;

    QFile logfile;
    QFile configFile;

    QString currentPlaylistName = "";
    QString playingSongPlaylist = "";
    QString lastPlaylistName = "";

    vector <Song> playlist;
    int currentID = -1;
    int lastTrackID = -1;

    QLineEdit * searchSong;

    QWidget * titlebarWidget;

    QListWidget * playlistWidget;
    QTabBar * playlistsBar;

    QLabel * songTitle;
    QLabel * songInfo;
    QLabel * songDuration;
    QLabel * songPosition;
    QLabel * timecode;
    QLabel * windowTitle;
    CustomSlider * volumeSlider;

    QImage cover;

    QPushButton * repeatBtn;
    QPushButton * shuffleBtn;
    QPushButton * pauseBtn;
    QPushButton * audio3dBtn;
    QPushButton * equoBtn;
    QPushButton * timerBtn;
    QPushButton * visualBtn;
    QPushButton * volumeBtn;
    QPushButton * remoteBtn;
    QPushButton * closeBtn;
    QPushButton * minimizeBtn;

    QSystemTrayIcon * trayIcon;

    QPoint lastMousePosition;
    bool moving;

    QTimer * timer;

    Ui::MainWindow * ui;

    InfoWidget * infoWidget = nullptr;
    PlaylistReader * XMLreader = nullptr;
    settingsWindow * settingsWin = nullptr;
    equalizerWindow * equalizerWin = nullptr;
    VisualizationWindow * visualWin = nullptr;
    StartWidget * startWidget = nullptr;

    QWidget * marksWin = nullptr;
    QListWidget * marksList;

    QWebSocketServer * removeControlServer;
    QTcpServer * httpServer;
    QList <QWebSocket *> remoteDevices;

    void remoteControl() {
        if (!remoteServerEnabled) {
            if (removeControlServer->listen(QHostAddress::Any, 9999)) {
                qDebug() << "Remote control socket started listening in port 9999!";

                remoteServerEnabled = true;
                reloadStyles();

                connect(removeControlServer, &QWebSocketServer::newConnection, this, &MainWindow::remoteDeviceConnect);
            }
            else {
                qDebug() << "Failed to start socket! " << removeControlServer->errorString();
            }

            if (httpServer->listen(QHostAddress::Any, httpServerPort)) {
                qDebug() << "Http server started with port" << httpServerPort;

                connect(httpServer, &QTcpServer::newConnection, this, &MainWindow::httpNewConnection);
            } else
            {
                qDebug() << "Failed to start http server!";
            }
        }

        QMessageBox msgBox;
        msgBox.setWindowTitle("Remote Control Server");
        msgBox.setText("Remote control server address: " + getLocalAddress() + ":" + QString::number(httpServerPort));
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();
    }
    void sendMessageToRemoteDevices (QString message) {
        for (auto & device : remoteDevices) {
            device->sendTextMessage(message);
        }
    }
    QString getIdentifier(QWebSocket *peer)
    {
        return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
                                           QString::number(peer->peerPort()));
    }
    void deviceDisconnected();

    void searchInPlaylist(const QString & text);
    void drawAllPlaylists();
    void drawPlaylist();
    void setTitle();
    void prerenderFft (QString file);
    void clearPrerenderedFft() {
        for (int i = 0; i < 1024; i++)
            prerenderedFft[i] = 3;
    }

    // If main window closed, close subwindows
    void closeEvent(QCloseEvent * event) {
        this->equalizerWin->close();
        this->settingsWin->close();
        this->visualWin->close();
        this->trayIcon->setVisible(false);
        QApplication::exit();
    };

    void paintEvent(QPaintEvent * event);
    void mousePressEvent (QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent * event);
    void wheelEvent (QWheelEvent * event);

    QString seconds2qstring (float seconds);
    double qstring2seconds (QString time);

    void setPosition (float pos) {

        if (!looped)
            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, pos), BASS_POS_BYTE);
        else
        {
            if (pos < loopStart || pos > loopEnd)
                BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, loopStart), BASS_POS_BYTE);
            else
                BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, pos), BASS_POS_BYTE);
        }
    }

    float getDuration () {
        QWORD len = BASS_ChannelGetLength(channel, BASS_POS_BYTE); // the length in bytes
        return BASS_ChannelBytes2Seconds(channel, len); // the length in seconds
    }
    float getPosition () {
        QWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE);
        return BASS_ChannelBytes2Seconds(channel, pos);
    }

    void writeLog (QString text)
    {
        if (!logging) return;

        QTextStream out(&logfile);
        QDateTime now = QDateTime::currentDateTime();

        QString name = qgetenv("USER");
        if (name.isEmpty())
            name = qgetenv("USERNAME");

        out << name << " [" << now.toString("hh:mm:ss") << "]: " << text << "\n";
    }
    void writeErrorLog (QString text)
    {
        if (!logging) return;

        QTextStream out(&logfile);
        QDateTime now = QDateTime::currentDateTime();

        QString name = qgetenv("USER");
        if (name.isEmpty())
            name = qgetenv("USERNAME");

        out << "ERROR! " <<  name << " [" << now.toString("hh:mm:ss") << "]: " << text << "\n";
    }

    void coverBackgroundPopup() {
        for (int i = 255; i >= 180; i--)
        {
            this->coverBgOpacity = i;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    void coverBackgroundHide() {
        for (int i = 180; i <= 255; i++)
        {
            this->coverBgOpacity = i;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void jumpTo() {
        bool ok;
        QString pos = QInputDialog::getText(this, tr("Jump to position"),  tr("Jump to position:"), QLineEdit::Normal, "00:00", &ok);

        if (ok)
        {
            double time = qstring2seconds(pos);

            qDebug() << pos;
            qDebug() << time;

            if (time > getDuration() || time == -1)
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Error");
                msgBox.setText("Incorrect position!");
                msgBox.setStyleSheet("background-color: #101010; color: silver;");
                msgBox.exec();
            } else {
                songPosition->setText(pos);
                BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, time), BASS_POS_BYTE);
                taskbarProgress->setValue(time);
            }
        }
    }

    int getPlaylistIndexByName (QString name) {
        int i = 0;
        for (auto & playlist : playlists)
        {
            if (name == playlist.first) break;
            i++;
        }
        return i;
    }

    void removeDir(QString dir);
};
#endif // MAINWINDOW_H
