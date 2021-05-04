#include "equalizerwindow.h"
#include "ui_equalizerwindow.h"

equalizerWindow::equalizerWindow(QWidget *parent) : QWidget(parent), ui(new Ui::equalizerWindow)
{   
    ui->setupUi(this);

    this->setWindowTitle("Equalizer");
    this->setStyleSheet("background: #101010; color: silver;");
    this->setWindowFlags(Qt::Drawer | Qt::Window);

    enabledCheckBox = new QCheckBox(this);
    enabledCheckBox->setText("Enable");
    enabledCheckBox->setGeometry(20, 20, 50, 15);
    enabledCheckBox->show();

    templates = new QComboBox(this);
    templates->setGeometry(this->size().width() - 135, 20, 120, 20);
    templates->addItem("Custom");
    templates->addItem("Bass");
    templates->addItem("Jazz");
    templates->addItem("Rock");
    templates->addItem("Metal");
    templates->addItem("None");
    templates->show();

    for (int i = 0; i < FREQS_COUNT; i++)
    {
        freqs[i] = new QSlider(Qt::Vertical, this);
        freqs[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        freqs[i]->setStyleSheet("QSlider::groove:vertical {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 0px 7px;"\
                                "}" \
                               "QSlider::handle:vertical {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "height: 5px; " \
                                    "margin: -2px -5px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::sub-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: silver; " \
                                "}" \
                                "QSlider::add-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: silver; " \
                                "}");
        freqs[i]->setGeometry(15 + 30 * i, 60, 20, 150);
        freqs[i]->setRange(-12, 12);
        freqs[i]->setValue(0);
        freqs[i]->setSingleStep(1);
        freqs[i]->show();

        connect (freqs[i], &QSlider::customContextMenuRequested, [=]() {
             freqs[i]->setValue(0);
        });

        QLabel * freqNum = new QLabel (this);
        freqNum->setStyleSheet("font-size: 10px");

        if (freq_values[i] >= 1000 && freq_values[i] < 10000)
            freqNum->setText(QString::number(int(freq_values[i] / 100) / 10.0f) + "khz");
        else if (freq_values[i] >= 10000)
            freqNum->setText(QString::number(freq_values[i] / 1000) + "khz");
        else
            freqNum->setText(QString::number(freq_values[i]) + "hz");

        freqNum->setGeometry(10 + 30 * i, 220, 30, 20);
        freqNum->setAlignment(Qt::AlignCenter);
    }

    pitch = new QSlider(Qt::Horizontal, this);
    pitch->setGeometry(20, 250, 100, 15);
    pitch->setRange(-100, 100);
    pitch->setValue(0);
    pitch->setSingleStep(1);
    pitch->hide();

    tempo = new QSlider(Qt::Horizontal, this);
    tempo->setGeometry(140, 250, 100, 15);
    tempo->setRange(-50, 50);
    tempo->setValue(0);
    tempo->setSingleStep(1);
    tempo->hide();

    QLabel * hint = new QLabel(this);
    hint->setStyleSheet("color: gray");
    hint->setText("* Press right button to return default value");
    hint->setGeometry(20, 270, this->size().width() - 20, 20);
    hint->show();
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

void equalizerWindow::reloadStyles () {
    for (int i = 0; i < FREQS_COUNT; i++)
        freqs[i]->setStyleSheet("QSlider::groove:vertical {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 0px 7px;"\
                                "}" \
                               "QSlider::handle:vertical {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "height: 5px; " \
                                    "margin: -2px -5px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::sub-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: silver; " \
                                "}" \
                                "QSlider::add-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: " + tr((*mainColorStr).c_str()) + "; " \
                                "}");
}
