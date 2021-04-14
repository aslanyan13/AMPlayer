#ifndef EQUALIZERWINDOW_H
#define EQUALIZERWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QDebug>

#include <time.h>

#include "bass.h"
#include "bass_fx.h"

const int FREQS_COUNT = 16;

namespace Ui {
class equalizerWindow;
}

class equalizerWindow : public QWidget
{
    Q_OBJECT

public:
    HSTREAM * channel;

    explicit equalizerWindow(QWidget * parent = nullptr);
    ~equalizerWindow();

    void init();

private:
    Ui::equalizerWindow * ui;

    QSlider * pitch;
    QSlider * tempo;

    QSlider * freqs[FREQS_COUNT];
    int freq_values[FREQS_COUNT] = {31, 63, 87, 125, 175, 250, 350, 500, 700, 1000, 2000, 2800, 4000, 5600, 8000, 11200};

    HFX fx[FREQS_COUNT];
};

#endif // EQUALIZERWINDOW_H
