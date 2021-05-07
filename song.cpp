#include "song.h"

Song::Song()
{

}

double Song::getDuration () {
    HSTREAM stream;

    if (getFormat() == "FLAC")
        stream = BASS_FLAC_StreamCreateFile(false, path.toStdWString().c_str(), 0, 0, BASS_STREAM_DECODE);
    else
        stream = BASS_StreamCreateFile(FALSE, path.toStdWString().c_str(), 0, 0, BASS_STREAM_DECODE);

    QWORD len = BASS_ChannelGetLength(stream, 0); // the length in bytes
    double dur = BASS_ChannelBytes2Seconds(stream, len);

    BASS_StreamFree(stream);    // Clear memory

    return dur; // the length in seconds
}
QString Song::getFormat () {
    return suffix;
}
QImage Song::getCover() {
    QImage cover;

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

                    if (remove("cover.png") != 0)
                        qDebug() << "Cover delete error!";

                    ofstream file("cover.png", ios::out | ios::binary | ofstream::trunc);


                    file.write(PicFrame->picture().data(), PicFrame->picture().size());
                    file.close();

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

    return cover;
}
