#ifndef SONG_H
#define SONG_H

#include <QtWinExtras/QtWin>
#include <QString>
#include <QPixmap>
#include <QImage>
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

#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <attachedpictureframe.h>

#include "bass.h"

using namespace std;
using namespace TagLib;


class Song
{
private:
    QString name = "";
    QString suffix = "";
public:
    QString path = "";

    Song();
    Song(QString p) : path(p) {
        QMimeDatabase db;
        QMimeType * mime = new QMimeType(db.mimeTypeForFile(p, QMimeDatabase::MatchContent));
        suffix = mime->preferredSuffix();
        suffix = suffix.toUpper();

        if (suffix == "") suffix = "Unknown";

        delete mime;
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

    QString getName () {
        return name;
    }
    bool operator==(const Song right) {
        if (this->path == right.path) return true;
        return false;
    }

    double getDuration();
    QString getFormat();
    QImage getCover();
};

#endif // SONG_H
