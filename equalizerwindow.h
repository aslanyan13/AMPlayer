#ifndef EQUALIZERWINDOW_H
#define EQUALIZERWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

#include <time.h>
#include <string>

#include "bass.h"
#include "bass_fx.h"

#include "customslider.h"

const int FREQS_COUNT = 16;

namespace Ui {
class equalizerWindow;
}

class equalizerWindow : public QWidget
{
    Q_OBJECT

public:
    HSTREAM * channel = nullptr;
    QColor * mainColor;
    std::string * mainColorStr;
    QCheckBox * enabledCheckBox;

    explicit equalizerWindow(QWidget * parent = nullptr);
    ~equalizerWindow();

    void init();
    void reloadStyles ();

private:
    float sampleRate;

    bool reversed = false;

    Ui::equalizerWindow * ui;

    QComboBox * templates;

    CustomSlider * pitch;
    CustomSlider * tempo;
    CustomSlider * speed;
    CustomSlider * reverb;

    QLabel * pitchValue;
    QLabel * tempoValue;
    QLabel * reverbValue;
    QLabel * speedValue;

    QLabel * reverbTitle;

    void reverbUpdate(int v) {
        int value = v;

        v = 20 - v;
        BASS_DX8_REVERB p;

        BASS_FXGetParameters(reverbFx, &p);
        p.fReverbMix = -0.012f * (float)(v * v * v);
        BASS_FXSetParameters(reverbFx, &p);

        reverbValue->setText(QString::number(int((value - 5) / 1.5f)));
    }

    CustomSlider * freqs[FREQS_COUNT];
    int freq_values[FREQS_COUNT] = {31, 63, 87, 125, 175, 250, 350, 500, 700, 1000, 2000, 2800, 4000, 5600, 8000, 11200};

    HFX fx[FREQS_COUNT];
    HFX reverbFx;
};

#endif // EQUALIZERWINDOW_H
