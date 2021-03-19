#ifndef PLAYLISTREADER_H
#define PLAYLISTREADER_H

#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QDebug>

#include <iostream>
#include <vector>
#include <map>

#include "song.h"

using namespace std;

class PlaylistReader
{
private:
    QString filename;
    bool fileOpened;
public:
    PlaylistReader();
    PlaylistReader(QString f) : filename(f) {};

    map <QString, vector <Song>> readPlaylists ();
    void writePlaylists(map <QString, vector <Song>> playlists);
};

#endif // PLAYLISTREADER_H
