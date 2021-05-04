#include "playlistreader.h"

void PlaylistReader::readPlaylists (fifo_map <QString, vector <Song>> & playlists)
{
    QFile file(filename);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "File open failed!";
    }
    else {
        QXmlStreamReader xmlReader;
        xmlReader.setDevice(&file);
        xmlReader.readNext();

        QString attribute_value = "";
        while (!xmlReader.atEnd())
        {
            if(xmlReader.isStartElement())
            {
                if(xmlReader.name() == "playlist") {
                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                        if (attr.name().toString() == "name") {
                            attribute_value = attr.value().toString();

                            playlists[attribute_value] = vector<Song>();
                        }
                    }
                }

                if (xmlReader.name() == "song" && attribute_value != "")
                {
                    Song temp(xmlReader.readElementText());

                    if (!QFile::exists(temp.path)) continue;

                    TagLib::FileRef f(temp.path.toStdWString().c_str());

                    wstring artist = L"", title = L"";

                    // If file not load failed
                    if (!f.isNull()) {
                        artist = f.tag()->artist().toCWString();
                        title = f.tag()->title().toCWString();
                    }

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

                    playlists[attribute_value].push_back(temp);
                }
            }

            xmlReader.readNext();
        }
    }
}
void PlaylistReader::writePlaylists(fifo_map<QString, vector<Song>> playlists)
{
    QFile file(filename);

    if (!file.open(QFile::WriteOnly))
    {
        qDebug() << "File open failed!";
        qDebug() << QDir::currentPath() + "/XML/playlists.xml";
        qDebug() << QFileDevice::OpenError;
    } else {
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);  // Устанавливаем автоформатирование текста
        xmlWriter.writeStartDocument();     // Запускаем запись в документ
        xmlWriter.writeStartElement("playlists");

        for (auto &playlist : playlists)
        {
            xmlWriter.writeStartElement("playlist");
            xmlWriter.writeAttribute("name", playlist.first);

            for (int i = 0; i < playlist.second.size(); i++)
            {
                xmlWriter.writeStartElement("song");
                xmlWriter.writeCharacters(playlist.second[i].path);
                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
    }
}
