#include "equalizerwindow.h"
#include "ui_equalizerwindow.h"

equalizerWindow::equalizerWindow(QWidget *parent) : QWidget(parent), ui(new Ui::equalizerWindow)
{   
    ui->setupUi(this);

    this->setWindowTitle("Sound Effects");
    this->setStyleSheet("background: #101010; color: silver;");
    this->setWindowFlags(Qt::Drawer | Qt::Window);

    enabledCheckBox = new QCheckBox(this);
    enabledCheckBox->setText("Enable SFX");
    enabledCheckBox->setGeometry(20, 20, 80, 15);
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
        freqs[i] = new CustomSlider(Qt::Vertical, this);
        freqs[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        freqs[i]->setCursor(Qt::PointingHandCursor);
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

    QFrame * line = new QFrame(this);
    line->setGeometry(15, 260, this->width() - 30, 3);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QLabel * tempoTitle = new QLabel("Tempo", this);
    tempoTitle->setGeometry(20, 272, 100, 15);
    tempoTitle->setStyleSheet("color: gray");
    tempoTitle->show();

    tempo = new CustomSlider(Qt::Horizontal, this);
    tempo->setContextMenuPolicy(Qt::CustomContextMenu);
    tempo->setGeometry(20, 290, 100, 18);
    tempo->setRange(-50, 200);
    tempo->setCursor(Qt::PointingHandCursor);
    tempo->setValue(0);
    tempo->setSingleStep(1);
    tempo->show();

    tempoValue = new QLabel(this);
    tempoValue->setGeometry(130, 290, 30, 16);
    tempoValue->setText("x1.0");
    tempoValue->show();

    QLabel * pitchTitle = new QLabel("Pitch", this);
    pitchTitle->setGeometry(180, 272, 100, 15);
    pitchTitle->setStyleSheet("color: gray");
    pitchTitle->show();

    pitch = new CustomSlider(Qt::Horizontal, this);
    pitch->setContextMenuPolicy(Qt::CustomContextMenu);
    pitch->setGeometry(180, 290, 100, 18);
    pitch->setRange(-100, 100);
    pitch->setCursor(Qt::PointingHandCursor);
    pitch->setValue(0);
    pitch->setSingleStep(1);
    pitch->show();

    pitchValue = new QLabel(this);
    pitchValue->setGeometry(290, 290, 30, 16);
    pitchValue->setText("0");
    pitchValue->show();

    QLabel * speedTitle = new QLabel("Speed", this);
    speedTitle->setGeometry(340, 272, 100, 15);
    speedTitle->setStyleSheet("color: gray");
    speedTitle->show();

    speed = new CustomSlider(Qt::Horizontal, this);
    speed->setContextMenuPolicy(Qt::CustomContextMenu);
    speed->setGeometry(340, 290, 100, 18);
    speed->setRange(50, 200);
    speed->setCursor(Qt::PointingHandCursor);
    speed->setValue(100);
    speed->setSingleStep(1);
    speed->show();

    speedValue = new QLabel(this);
    speedValue->setGeometry(450, 290, 30, 16);
    speedValue->setText("x1.0");
    speedValue->show();

    reverbTitle = new QLabel("Reverb", this);
    reverbTitle->setGeometry(20, 320, 100, 15);
    reverbTitle->setStyleSheet("color: gray");
    reverbTitle->show();

    reverb = new CustomSlider(Qt::Horizontal, this);
    reverb->setContextMenuPolicy(Qt::CustomContextMenu);
    reverb->setGeometry(20, 335, 100, 18);
    reverb->setRange(5, 20);
    reverb->setCursor(Qt::PointingHandCursor);
    reverb->setValue(5);
    reverb->setSingleStep(1);
    reverb->show();

    reverbValue = new QLabel(this);
    reverbValue->setGeometry(130, 335, 30, 16);
    reverbValue->setText("0");
    reverbValue->show();

    QPushButton * reverseBtn = new QPushButton(this);
    reverseBtn->setText("Reverse");
    reverseBtn->setGeometry(15, 400, 50, 20);
    reverseBtn->hide();

    QLabel * hint = new QLabel(this);
    hint->setStyleSheet("color: gray");
    hint->setText("*Hint: Press right button to return default value");
    hint->setGeometry(20, this->height() - 30, this->size().width() - 20, 20);
    hint->show();

    connect (tempo, &QSlider::customContextMenuRequested, [=]() {
        tempo->setValue(0);
    });
    connect (tempo, &QSlider::valueChanged, [=](int value) {
        if (channel == nullptr || *channel == NULL) return;

        BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO, value / 10.0f);
        tempoValue->setText("x" + QString::number(1 + (value / 100.0f)));
    });

    connect (pitch, &QSlider::customContextMenuRequested, [=]() {
        pitch->setValue(0);
    });
    connect (pitch, &QSlider::valueChanged, [=](int value) {
        if (channel == nullptr || *channel == NULL) return;

        BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO_PITCH, value / 10.0f);
        pitchValue->setText(QString::number(value / 10.0f));
    });

    connect (reverb, &QSlider::customContextMenuRequested, [=]() {
        reverb->setValue(0);
    });

    connect (speed, &QSlider::customContextMenuRequested, [=]() {
        speed->setValue(100);
    });
    connect (speed, &QSlider::valueChanged, [=](int value) {
        if (channel == nullptr || *channel == NULL) return;

        BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_FREQ, sampleRate * (value / 100.0f));
        speedValue->setText("x" + QString::number((value / 100.0f)));
    });
    connect (reverb, &QSlider::valueChanged, [=](int v) {
        reverbUpdate(v);
    });

    connect (reverseBtn, &QPushButton::clicked, [=]() {
        DWORD srcChan=BASS_FX_TempoGetSource(*channel);
        float dir;
        BASS_ChannelGetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, &dir);

        if (dir<0) {
            BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_FORWARD);
        } else {
            BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_REVERSE);
        }
    });
}

equalizerWindow::~equalizerWindow()
{
    delete ui;
}

void equalizerWindow::init() {
    *channel = BASS_FX_ReverseCreate(*channel, 2, BASS_STREAM_DECODE);
    *channel = BASS_FX_TempoCreate(*channel, NULL);

    BASS_ChannelGetAttribute(*channel, BASS_ATTRIB_FREQ, &sampleRate);

    for (int i = 0; i < FREQS_COUNT; i++)
    {
        BASS_BFX_PEAKEQ bfx;
        fx[i] = BASS_ChannelSetFX(*channel, BASS_FX_BFX_PEAKEQ, 0);

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

    BASS_DX8_REVERB bfx;
    reverbFx = BASS_ChannelSetFX(*channel, BASS_FX_DX8_REVERB, 0);
    bfx.fReverbMix = -96;
    BASS_FXSetParameters(reverbFx, &bfx);

    reverbUpdate(reverb->value());

    DWORD srcChan=BASS_FX_TempoGetSource(*channel);
    float dir;
    BASS_ChannelGetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, &dir);

    if (dir<0) {
        BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_FORWARD);
    } else {
        BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_REVERSE);
    }

    BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO, tempo->value() / 10.0f);
    BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_TEMPO_PITCH, pitch->value() / 10.0f);
    BASS_ChannelSetAttribute(*channel, BASS_ATTRIB_FREQ, sampleRate * (speed->value() / 100.0f));
}

void equalizerWindow::reloadStyles () {
    QString sliderStyleVertical = "QSlider::groove:vertical {" \
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
                      "}";
    QString sliderStyleHorizontal = "QSlider::groove:horizontal {" \
                          "border: 1px solid #999999; " \
                          "border-radius: 20px;" \
                          "background: #141414;"\
                          "margin: 7px 0px;"\
                      "}" \
                     "QSlider::handle:horizontal {" \
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                          "border: 1px solid #5c5c5c; "\
                          "width: 5px; " \
                          "margin: -5px -2px; " \
                          "border-radius: 20px; " \
                      "}"
                      "QSlider::sub-page:horizontal {" \
                          "border-radius: 20px;" \
                          "margin: 7px 0px;" \
                          "background: " + tr((*mainColorStr).c_str()) + "; " \
                      "}" \
                      "QSlider::add-page:horizontal {" \
                          "border-radius: 20px;" \
                          "margin: 7px 0px;" \
                          "background: silver; " \
                      "}";

    for (int i = 0; i < FREQS_COUNT; i++)
        freqs[i]->setStyleSheet(sliderStyleVertical);

    tempo->setStyleSheet(sliderStyleHorizontal);
    pitch->setStyleSheet(sliderStyleHorizontal);
    reverb->setStyleSheet(sliderStyleHorizontal);
    speed->setStyleSheet(sliderStyleHorizontal);
}
