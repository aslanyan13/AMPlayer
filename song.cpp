#include "song.h"

Song::Song()
{

}

double Song::getDuration () {
    return this->dur; // the length in seconds
}
QString Song::getFormat () {
    return suffix;
}
QImage Song::getCover() {
    QImage cover;

    if (remove("cover.png") != 0)
        qDebug() << "Cover delete error!";

    if (this->getFormat() == "M4A" || this->getFormat() == "MP4") {
        TagLib::MP4::File f(path.toStdWString().c_str());
        TagLib::MP4::Tag * tag = f.tag();
        TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();
        TagLib::MP4::Item coverItem = itemsListMap["covr"];
        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();

        if (!coverArtList.isEmpty()) {
            TagLib::MP4::CoverArt coverArt = coverArtList.front();

            ofstream file("cover.png", ios::out | ios::binary | ofstream::trunc);

            file.write(coverArt.data().data(), coverArt.data().size());
            file.close();
        }
    }

    if (this->getFormat() == "FLAC") {
        TagLib::FLAC::File * file = new  TagLib::FLAC::File(path.toStdWString().c_str());
        const TagLib::List<TagLib::FLAC::Picture*>& picList = file->pictureList();
        TagLib::FLAC::Picture * picFrame = nullptr;

        for (int i = 0; i < picList.size(); i++) {
            picFrame = (TagLib::FLAC::Picture *)picList[i];

            if (picFrame->data().size() == 0) continue;

            ofstream file("cover.png", ios::out | ios::binary | ofstream::trunc);

            file.write(picFrame->data().data(), picFrame->data().size());
            file.close();

            break;
        }
        delete file;
    }

    if (this->getFormat() == "MP3") {
        MPEG::File * f = new MPEG::File(path.toStdWString().c_str());
        ID3v2::Tag * id3v2tag = f->ID3v2Tag();
        TagLib::ID3v2::AttachedPictureFrame * PicFrame = nullptr;
        auto frameList = id3v2tag->frameListMap()["APIC"];

        if (id3v2tag)
        {
            if (frameList.size() > 0)
            {
                for (int i = 0; i < frameList.size(); i++)
                {
                    PicFrame = (TagLib::ID3v2::AttachedPictureFrame *)(frameList[i]);

                    if (PicFrame->picture().size() == 0)
                        continue;

                    ofstream file("cover.png", ios::out | ios::binary | ofstream::trunc);

                    file.write(PicFrame->picture().data(), PicFrame->picture().size());
                    file.close();

                    break;
                }
            }
        }
        else
        {
            qDebug() << "id3v2 not present";
        }
        delete f;
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Gdiplus::Bitmap * bitmap = Gdiplus::Bitmap::FromFile(L"cover.png");

    HBITMAP handleToSliceRet = NULL;
    bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &handleToSliceRet);

    QPixmap pixmap = QtWin::fromHBITMAP(handleToSliceRet);
    cover = pixmap.toImage();

    QSize coverSize = cover.size();

    // Image cropping to make it square
    if (coverSize.width() > coverSize.height())
    {
        cover = cover.copy((coverSize.width() - coverSize.height()) / 2, 0, coverSize.height(), coverSize.height());
    }
    if (coverSize.height() > coverSize.width())
    {
        cover = cover.copy(0, (coverSize.height() - coverSize.width()) / 2, coverSize.height(), coverSize.height());
    }

    qDebug() << cover.size();

    DeleteObject(bitmap);
    DeleteObject(handleToSliceRet);

    delete bitmap;
    return cover;
}
void Song::parseLyrics() {
    QFile file (lrcFile);

    if (file.open(QIODevice::ReadWrite | QFile::Text)) {
        while (!file.atEnd()) {
            QString line = file.readLine();
            line = line.simplified();
            QString text = line.mid(line.indexOf("]") + 1);
            float time = qstring2seconds(line.mid(1, line.indexOf(']') - 1));

            if (isnan(time)) continue;
            lyrics.push_back(make_pair(time, text));
        }
    } else {
        qDebug() << "Error!";
        qDebug() << file.errorString();
    }
}
void Song::createStream(HSTREAM & chan, QString file, DWORD flags) {
    if (getFormat() == "FLAC" || getFormat() == "OGA")
        chan = BASS_FLAC_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
    else if (getFormat() == "OPUS")
        chan = BASS_OPUS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
    else if (getFormat() == "WEBM" || getFormat() == "MKA" || getFormat() == "MKV")
        chan = BASS_WEBM_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags, 0);
    else if (getFormat() == "APE") {
        if (!QDir("C:/amptemp2").exists())
            QDir().mkdir("C:/amptemp2");

        QFile::copy(file, "C:/amptemp2/tmp.ape");
        chan = BASS_APE_StreamCreateFile(false, "C:/amptemp2/tmp.ape", 0, 0, flags);
    }
    else if (getFormat() == "SPX") {
        if (!QDir("C:/amptemp2").exists())
            QDir().mkdir("C:/amptemp2");

        QFile::copy(file, "C:/amptemp2/tmp.spx");
        chan = BASS_SPX_StreamCreateFile(false, "C:/amptemp2/tmp.spx", 0, 0, flags);
    }
    else if (getFormat() == "TTA") {
        if (!QDir("C:/amptemp2").exists())
            QDir().mkdir("C:/amptemp2");

        QFile::copy(file, "C:/amptemp2/tmp.tta");
        chan = BASS_TTA_StreamCreateFile(false, "C:/amptemp2/tmp.tta", 0, 0, flags);
    }
    else
        chan = BASS_StreamCreateFile(false, file.toStdWString().c_str(), 0, 0, flags);
}
