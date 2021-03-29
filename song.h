#ifndef SONG_H
#define SONG_H

#include <QString>

#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

class Song
{
private:
    QString name = "";
public:
    QString path = "";

    Song();
    Song(QString p) : path(p) {};

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
};

#endif // SONG_H
