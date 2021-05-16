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
#include <windows.h>
#include <gdiplus.h>
#include <gdiplus/gdiplusheaders.h>
#include <string>
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
public:
    QString path = "";
    map <int, QString> marks;

    Song();
    Song(QString p) : path(p) {
        // QMimeDatabase db;
        // QMimeType * mime = new QMimeType(db.mimeTypeForFile(p, QMimeDatabase::MatchContent));
        // suffix = mime->preferredSuffix();
        // suffix = suffix.toUpper();

        suffix = path.mid(path.lastIndexOf('.') + 1);
        suffix = suffix.toUpper();

        if (suffix == "") suffix = "Unknown";

        // delete mime;
    };

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
                if (!QDir("C:/amptemp2").exists())
                    QDir().mkdir("C:/amptemp2");

                QFile::copy(file, "C:/amptemp2/tmp.ape");
                chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp2/tmp.ape", 0, 0, flags);
            }
        }
        if (BASS_ErrorGetCode()) {
            remove("C:/amptemp2/tmp.ape");
            chan = BASS_SPX_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp2").exists())
                    QDir().mkdir("C:/amptemp2");

                QFile::copy(file, "C:/amptemp2/tmp.spx");
                chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp2/tmp.spx", 0, 0, flags);
            }
        }
        if (BASS_ErrorGetCode()) {
            remove("C:/amptemp2/tmp.spx");
            chan = BASS_TTA_StreamCreateFile(false, file.toStdString().c_str(), 0, 0, flags);
            if (BASS_ErrorGetCode()) {
                if (!QDir("C:/amptemp2").exists())
                    QDir().mkdir("C:/amptemp2");

                QFile::copy(file, "C:/amptemp2/tmp.tta");
                chan = BASS_TTA_StreamCreateFile(false, "C:/amptemp2/tmp.tta", 0, 0, flags);
            }
        }
    }

    double getDuration();
    QString getFormat();
    QImage getCover();
};

#endif // SONG_H
