#include "equalizerwindow.h"
#include "ui_equalizerwindow.h"

equalizerWindow::equalizerWindow(QWidget *parent) : QWidget(parent), ui(new Ui::equalizerWindow)
{   
    ui->setupUi(this);

    for (int i = 0; i < FREQS_COUNT; i++)
    {
        freqs[i] = new QSlider(Qt::Vertical, this);
        freqs[i]->setGeometry(20 + 25 * i, 80, 15, 150);
        freqs[i]->setRange(-10, 10);
        freqs[i]->setValue(0);
        freqs[i]->setSingleStep(1);
        freqs[i]->show();
    }

    pitch = new QSlider(Qt::Horizontal, this);
    pitch->setGeometry(20, 20, 100, 15);
    pitch->setRange(-100, 100);
    pitch->setValue(0);
    pitch->setSingleStep(1);
    pitch->show();

    tempo = new QSlider(Qt::Horizontal, this);
    tempo->setGeometry(140, 20, 100, 15);
    tempo->setRange(-50, 50);
    tempo->setValue(0);
    tempo->setSingleStep(1);
    tempo->show();
}

equalizerWindow::~equalizerWindow()
{
    delete ui;
}

void equalizerWindow::init() {
    for (int i = 0; i < FREQS_COUNT; i++)
    {
        float start = clock();

        BASS_BFX_PEAKEQ bfx;
        fx[i] = BASS_ChannelSetFX(*channel, BASS_FX_BFX_PEAKEQ, 0);

        // qDebug() << BASS_ErrorGetCode();

        bfx.fBandwidth = 0.2f;
        bfx.fGain = freqs[i]->value();
        bfx.lBand = i;
        bfx.lChannel = BASS_BFX_CHANALL;
        bfx.fCenter = freq_values[i];

        BASS_FXSetParameters(fx[i], &bfx);

        connect (freqs[i], &QSlider::valueChanged, [=](int value) {
            BASS_BFX_PEAKEQ bfx;
            bfx.fCenter = freq_values[i];
            bfx.lBand = i;
            BASS_FXGetParameters(fx[i], &bfx);
            bfx.fGain = value;
            BASS_FXSetParameters(fx[i], &bfx);
        });
    }

    connect (tempo, &QSlider::valueChanged, [=](int value) {
        BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO, value / 10.0f);
    });

    connect (pitch, &QSlider::valueChanged, [=](int value) {
        BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO_PITCH, value / 10.0f);
    });
}
