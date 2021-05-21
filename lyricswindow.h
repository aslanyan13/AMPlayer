#ifndef LYRICSWINDOW_H
#define LYRICSWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QFontDatabase>
#include <QLineEdit>
#include <QTimer>

#include <vector>
#include <algorithm>

#include "bass.h"

namespace Ui {
class LyricsWindow;
}

class LyricsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LyricsWindow(QWidget *parent = nullptr);
    ~LyricsWindow();

    QPushButton * addLyricsBtn;
    HSTREAM * channel;
    std::vector <std::pair<float, QString>> lyrics;

    void countCurrentLine () {
        float pos = getPosition();

        for (int i = 0; i < lyrics.size(); i++) {
            if (lyrics[i].first > pos + offset)
            {
                lineCounter = i;
                if (i == 0) lyricsLabel->setText("");
                else lyricsLabel->setText(lyrics[i - 1].second);
                break;
            }
        }
    }
    void resetLineCounter () {
        lineCounter = 0;
    }
signals:
    void closed();
private:
    Ui::LyricsWindow * ui;

    void closeEvent(QCloseEvent * event) {
        emit closed();
    }

    QString fontColor = "rgb(255, 255, 255)";
    QString backgroundColor = "rgb(0, 0, 0)";
    int fontSize = 24;

    int lineCounter = 0;
    // int offset = -0.100;
    int offset = 0;

    QPushButton * bgColorBtn;
    QPushButton * textColorBtn;

    QLineEdit * fontSizeLine = nullptr;

    QLabel * lyricsLabel = nullptr;
    QLabel * hint = nullptr;

    QTimer * timer;

    void resizeEvent(QResizeEvent * event) {
        lyricsLabel->setGeometry(0, 30, this->width(), this->height() - 30);
        hint->setGeometry(0, this->height() - 30, this->width(), 30);
    }
    void reloadStyles() {
        lyricsLabel->setStyleSheet("color: " + fontColor + "; background: " + backgroundColor + "; font-size: " + QString::number(fontSize) + "px;");
        textColorBtn->setStyleSheet("background: " + fontColor + "; border: 2px solid silver;");
        bgColorBtn->setStyleSheet("background: " + backgroundColor + "; border: 2px solid silver;");
    }

    QString qcolor2qstring (QColor color) {
        return "rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ")";
    }

    float getPosition () {
        QWORD pos = BASS_ChannelGetPosition(*channel, BASS_POS_BYTE);
        return BASS_ChannelBytes2Seconds(*channel, pos);
    }
};

#endif // LYRICSWINDOW_H
