#ifndef EQUALIZERWINDOW_H
#define EQUALIZERWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

#include <time.h>
#include <string>

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
    QColor * mainColor;
    std::string * mainColorStr;
    QCheckBox * enabledCheckBox;

    explicit equalizerWindow(QWidget * parent = nullptr);
    ~equalizerWindow();

    void init();
    void reloadStyles ();

private:
    Ui::equalizerWindow * ui;

    QComboBox * templates;

    QSlider * pitch;
    QSlider * tempo;

    QSlider * freqs[FREQS_COUNT];
    int freq_values[FREQS_COUNT] = {31, 63, 87, 125, 175, 250, 350, 500, 700, 1000, 2000, 2800, 4000, 5600, 8000, 11200};

    HFX fx[FREQS_COUNT];
};

#endif // EQUALIZERWINDOW_H
