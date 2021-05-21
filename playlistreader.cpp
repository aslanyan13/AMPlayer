#include "playlistreader.h"

void PlaylistReader::readPlaylists (fifo_map <QString, vector <Song>> & playlists)
{
    float start = clock();
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
                            if (attribute_value == "") continue;

                            playlists[attribute_value] = vector<Song>();
                        }
                    }
                }

                if (xmlReader.name() == "song" && attribute_value != "")
                {
                    QString jsonData;
                    QString lrcFile;

                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                        if (attr.name().toString() == "marks") {
                            jsonData = attr.value().toString();
                        }
                        if (attr.name().toString() == "lrc")
                            lrcFile = attr.value().toString();
                    }

                    QJsonObject obj;
                    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());

                    if(!doc.isNull())
                    {
                        if(doc.isObject())
                        {
                            obj = doc.object();
                        }
                        else
                        {
                            qDebug() << "Document is not an object";
                        }
                    }
                    else
                    {
                        qDebug() << "Invalid JSON...\n" << jsonData;
                    }

                    Song temp(xmlReader.readElementText());
                    temp.countDuration();
                    temp.setLrcFile(lrcFile);

                    foreach(const QString& key, obj.keys()) {
                        QJsonValue value = obj.value(key);
                        // qDebug() << "Key = " << key << ", Value = " << value.toString();
                        temp.marks[key.toDouble()] = value.toString();
                    }

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
    qDebug() << "Playlist readed! " << clock() - start;
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

                QString json = "{ ";
                int j = 1;
                for (auto & mark : playlist.second[i].marks)
                {

                    json += "\"" +  QString::number(mark.first) + "\": \"" + mark.second + "\"";
                    if (j != playlist.second[i].marks.size()) json += ",";
                    j++;
                }
                json += "}";

                xmlWriter.writeAttribute("marks", json);
                if (playlist.second[i].lrcFile != "")
                    xmlWriter.writeAttribute("lrc", playlist.second[i].lrcFile);
                xmlWriter.writeCharacters(playlist.second[i].path);

                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
    }
}
