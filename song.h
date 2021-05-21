#ifndef SONG_H
#define SONG_H

#include <QtWinExtras/QtWin>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QDir>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMimeData>
#include <QDebug>
#include <QByteArray>
#include <QByteArrayData>

#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include <gdiplus/gdiplusheaders.h>
#include <string>
#include <utility>
#include <map>

#include <id3v2tag.h>
#include <flacpicture.h>
#include <flacfile.h>
#include <mpegfile.h>
#include <mp4coverart.h>
#include <mp4file.h>
#include <flacmetadatablock.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <attachedpictureframe.h>

// Bass header files
#include "bass.h"
#include "bassopus.h"
#include "bass_ape.h"
#include "bass_fx.h"
#include "bassflac.h"
#include "basswebm.h"
#include "bass_spx.h"
#include "bass_tta.h"

using namespace std;
using namespace TagLib;

class Song
{
private:
    QString name = "";
    QString suffix = "";

    float sampleRate;
    float bitrate;
    float dur = -1;
public:
    vector <pair<float, QString>> lyrics;

    QString lrcFile = "";
    QString path = "";

    map <int, QString> marks;

    Song();
    Song(QString p) : path(p) {
        suffix = path.mid(path.lastIndexOf('.') + 1);
        suffix = suffix.toUpper();

        if (suffix == "") suffix = "Unknown";
    };

    Song(const Song& o) = default;
    Song(Song&& o) = default;
    Song& operator=(const Song&) = default;

    void setLrcFile (QString path) {
        if (path == "") return;

        lrcFile = path;
        parseLyrics();
    }

    void parseLyrics();
    void countDuration() {
        HSTREAM stream;

        createStream(stream, path, 0);

        QWORD len = BASS_ChannelGetLength(stream, 0); // the length in bytes
        this->dur = BASS_ChannelBytes2Seconds(stream, len);

        BASS_StreamFree(stream);
    }

    void setName (QString n) {
        this->name = n;
    }
    void setNameFromPath () {
        int left = path.lastIndexOf('/');
        name = path.mid(left + 1);

        int right = name.lastIndexOf('.');
        name = name.mid(0, right);
    }

    QString getFileSizeMB () {
        QFile file (path);
        double size = file.size();
        size /= 1024; // Kbytes
        size /= 1024; // MBytes
        size = int(size * 100) / 100.0f;

        QString sizeStr = QString::number(size) + " MB";

        return sizeStr;
    }
    QString getName () {
        return name;
    }
    bool operator==(const Song right) {
        if (this->path == right.path) return true;
        return false;
    }


    float qstring2seconds (QString timecode) {
        if (timecode.contains(':') && timecode.count('.') == 1) {
            return qstring2seconds(timecode.mid(0, timecode.indexOf('.'))) + timecode.mid(timecode.indexOf('.') + 1).toFloat() / 100.0f;
        }
        else if (timecode.contains(':')) {
            float minutes = timecode.mid(0, timecode.indexOf(':')).toFloat();
            float seconds = timecode.mid(timecode.indexOf(':') + 1).toFloat();

            return minutes * 60 + seconds;
        }
    }

    void createStream(HSTREAM & chan, QString file, DWORD flags);

    double getDuration();
    QString getFormat();
    QImage getCover();
};

#endif // SONG_H
