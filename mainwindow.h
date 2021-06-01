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
#include <QRadioButton>
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
#include "lyricswindow.h"

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
    void removePlaylist(int index, bool clear = false);
    void renamePlaylist(int index);

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

    void remoteControl();
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

        return "localhost";
    }

    void settings ();
    void showSongTimer () {
        timerWin->raise();
        timerWin->setFocus();
        timerWin->show();
    }
    void marksShow () {
        marksWin->raise();
        marksWin->show();
        marksWin->move (this->pos().x() + (this->size().width() - marksWin->size().width()) / 2, this->pos().y() + (this->size().height() - marksWin->size().height()) / 2);
        marksWin->setFocus();
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
    void drawMarksList();

    void loadLyrics(Song * song);

private:
    HSTREAM channel;

    void createStream(HSTREAM & chan, QString file, DWORD flags) {
        if (!BASS_StreamFree(chan))
            qDebug() << BASS_ErrorGetCode();

        chan = BASS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);

        if (BASS_ErrorGetCode()) {
            BASS_StreamFree(chan);
            chan = BASS_FLAC_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
        }
        if (BASS_ErrorGetCode()) {
            BASS_StreamFree(chan);
            chan = BASS_OPUS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
        }
        if (BASS_ErrorGetCode()) {
            BASS_StreamFree(chan);
            chan = BASS_WEBM_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags, 0);
        }

        if (BASS_ErrorGetCode()) {
            BASS_StreamFree(chan);
            chan = BASS_APE_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp").exists())
                    QDir().mkdir("C:/amptemp");

                QFile::copy(file, "C:/amptemp/tmp.ape");
                chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp/tmp.ape", 0, 0, flags);
            }
        }
        if (BASS_ErrorGetCode()) {
            BASS_StreamFree(chan);
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
            BASS_StreamFree(chan);
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
    bool lyricsEnabled = false;
    bool logging = true;
    bool playlistCreating = false;
    bool remoteServerEnabled = false;
    bool remoteScrolling = false;
    bool renamingPlaylist = false;
    bool timerStarted = false;
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
    int songTimerMode = -1;
    int songTimerCounter = -1;

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
    QPushButton * marksBtn;
    QPushButton * equoBtn;
    QPushButton * timerBtn;
    QPushButton * visualBtn;
    QPushButton * lyricsBtn;
    QPushButton * volumeBtn;
    QPushButton * remoteBtn;
    QPushButton * closeBtn;
    QPushButton * minimizeBtn;

    QSystemTrayIcon * trayIcon;

    QPoint lastMousePosition;
    bool moving;

    QTimer * timer;
    QTimer * songTimer;

    Ui::MainWindow * ui;

    InfoWidget * infoWidget = nullptr;
    PlaylistReader * XMLreader = nullptr;
    settingsWindow * settingsWin = nullptr;
    equalizerWindow * equalizerWin = nullptr;
    VisualizationWindow * visualWin = nullptr;
    StartWidget * startWidget = nullptr;
    LyricsWindow * lyricsWin = nullptr;

    QWidget * marksWin = nullptr;
    QListWidget * marksList;

    QWidget * timerWin = nullptr;
    void initTimerWindow();
    void songTimerEnded () {
        QTimer::singleShot(3000, [=]() {
            qDebug() << "Closed!";
            this->close();
        });

        QMessageBox msgBox;
        msgBox.setWindowTitle("Timer timed out");
        msgBox.setText("Program will be closed in 3 seconds...");
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();
    }

    QLabel * dropWidget;

    QWebSocketServer * removeControlServer;
    QTcpServer * httpServer;
    QList <QWebSocket *> remoteDevices;

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

    void dragEnterEvent(QDragEnterEvent *event)
    {
        QList <QUrl> list = event->mimeData()->urls();
        if (list.size() == 0)
            return;

        dropWidget->raise();
        dropWidget->show();

        event->acceptProposedAction();
    }
    void dragLeaveEvent(QDragLeaveEvent * event)
    {
        dropWidget->hide();
    }
    void dropEvent (QDropEvent * event)
    {
        QList <QUrl> list = event->mimeData()->urls();

        int duplicatesCount = 0;
        for (auto & file : list) {
            QString path = file.path();
            path = path.mid(1);
            QDir dir(path);
            if (dir.exists())
                continue;

            qDebug() << path << dir.exists();

            Song temp(path);

            QString formats = "*.tta *.spx *.ape *.mov *.ac3 *.asf *.webm *.wma *.adt *.adts *.wm *.wmv *.3g2 *.3gp *.flac *.aac *.mkv *.mka *.m4a *.m4b *.m4r *.mpa *.mpc *.mpeg *.m4p *.mp4 *.mp3 *.mp2 *.mp1 *.opus *.oga *.ogg *.aif *.aiff *.aifc *.wav *.avi";
            formats = formats.toUpper();

            if (formats.indexOf(temp.getFormat()) == -1)
            {
                duplicatesCount++;
                continue;
            }

            temp.countDuration();

            TagLib::FileRef f(path.toStdWString().c_str());

            wstring artist = L"", title = L"";

            // If file not load failed
            if (!f.isNull()) {
                artist = f.tag()->artist().toCWString();
                title = f.tag()->title().toCWString();
            }

            writeLog("Opened file: " + path);

            if (artist == L"" && title.find('-') == wstring::npos) {
                artist = L"Unknown Artist";
            } else if (artist == L"" && title.find('-') != wstring::npos) {
                artist = L"";
            }

            if (title == L"") {
                temp.setNameFromPath();
            } else {
                if (artist != L"")
                    temp.setName(QString::fromStdWString(artist) + " - " + QString::fromStdWString(title));
                else
                    temp.setName(QString::fromStdWString(title));
            }

            if (count(playlist.begin(), playlist.end(), temp) != 0)
            {
                duplicatesCount++;
                continue;
            }

            playlist.push_back(temp);
        }
        dropWidget->hide();

        QMessageBox msgBox;
        msgBox.setWindowTitle("Files adding");

        if (list.length() > 0 && duplicatesCount != list.length()) {
            msgBox.setText(tr(to_string(list.length() - duplicatesCount).c_str()) + " new files added to playlist successfully!");
        }
        else if (duplicatesCount == list.length() && list.length() > 0) {
            msgBox.setText ("0 new files added!");
        }
        else if (list.length() == 0) {
            msgBox.setText("There are no audio files in this folder!");
        }

        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();

        drawPlaylist();
        playlists[currentPlaylistName] = playlist;

    }

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
        lyricsWin->countCurrentLine();
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

            if (time > getDuration() || time == -1 || time < 0)
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Error");
                msgBox.setText("Timecode out of range!");
                msgBox.setStyleSheet("background-color: #101010; color: silver;");
                msgBox.exec();
            } else {
                setPosition(time);
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
