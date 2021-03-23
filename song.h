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
    wstring name = L"";
    wstring artist = L"";
    float duration = 0.0f;
public:
    wchar_t path[MAX_PATH] = L"";

    Song();
    Song(wchar_t p[MAX_PATH]) {
        for (int i = 0; i < MAX_PATH; i++)
            path[i] = p[i];
    };

    void setName (wstring n) {
        this->name = n;
    }
    void setNameFromPath () {
        wstring temp (path);
        int left = temp.find_last_of(L'\\');
        if (left == string::npos)
            left = temp.find_last_of(L'/');

        name = temp.substr (left + 1);

        int right = name.find_last_of(L'.', temp.length() - 1);
        name = name.substr(0, right);
    }
    void setPath (wchar_t p[MAX_PATH]) {
        for (int i = 0; i < MAX_PATH; i++)
            path[i] = p[i];
    }

    wstring getName () {
        return name;
    }
    bool operator==(const Song right) {
        if (wcscmp(this->path, right.path) == 0) return true;
        return false;
    }
};

#endif // SONG_H
