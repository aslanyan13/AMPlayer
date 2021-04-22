#ifndef PLAYLISTREADER_H
#define PLAYLISTREADER_H

#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QDebug>

#include <iostream>
#include <vector>

// Taglib library
#include <taglib/taglib.h>
#include <taglib/tfile.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

#include "song.h"
#include "fifo_map.hpp"

using namespace std;
using nlohmann::fifo_map;

class PlaylistReader
{
private:
    QString filename;
    bool fileOpened;
public:
    PlaylistReader();
    PlaylistReader(QString f) : filename(f) {};

    void readPlaylists (fifo_map <QString, vector <Song>> & playlists);
    void writePlaylists(fifo_map <QString, vector <Song>> playlists);
};

#endif // PLAYLISTREADER_H
