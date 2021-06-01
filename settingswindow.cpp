#include "settingswindow.h"
#include "ui_settingswindow.h"

settingsWindow::settingsWindow(QWidget *parent) : QWidget(parent), ui(new Ui::settingsWindow)
{
    colors[0] = QColor(255, 37, 79);   // Raspberry color
    colors[1] = QColor(37, 255, 20);   // Neon green color
    colors[2] = QColor(255, 255, 0);   // Yellow
    colors[3] = QColor(91, 192, 222);  // Cyan
    colors[4] = QColor(188, 58, 255);  // Violette
    colors[5] = QColor(255, 0, 134);   // Pink
    colors[6] = QColor(255, 130, 21);  // Orange

    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    this->setStyleSheet("QWidget { background-color: #101010; color: silver; }");
    this->setWindowFlags(Qt::Drawer);
    this->setWindowTitle("Settings");

    QLabel * text1 = new QLabel (this);
    text1->setStyleSheet("color: silver; font-size: 12px;");
    text1->setGeometry(20, 15, 100, 20);
    text1->setText("System color");

    for (int i = 0; i < 7; i++)
    {
        colorBtns[i] = new QPushButton(this);
        colorBtns[i]->setCursor(Qt::PointingHandCursor);
        colorBtns[i]->setStyleSheet("padding: 0px; border: 0px solid black; background: " + tr(qcolorToStr(colors[i]).c_str()) + ";");

        colorBtns[i]->setGeometry(30 + i * 35, 40, 30, 30);
        colorBtns[i]->raise();
        colorBtns[i]->show();

        connect(colorBtns[i], &QPushButton::pressed, [=] () {
            *mainColor = colors[i];
            *mainColorStr = qcolorToStr(colors[i]);
            this->reloadStyles();
        });
    }

    QRadioButton * colorChangingRadio = new QRadioButton("Auto color change", this);
    colorChangingRadio->setStyleSheet("color: silver; font-size: 12px;");
    colorChangingRadio->move(20, 100);
    colorChangingRadio->show();

    connect (colorChangingRadio, &QRadioButton::clicked, [=]() {
        *colorChanger = colorChangingRadio->isChecked();
    });

    QLabel * speedLabel = new QLabel("Speed: ", this);
    speedLabel->setGeometry(30, 130, 50, 25);

    colorSpeedSlider = new CustomSlider(Qt::Horizontal, this);
    colorSpeedSlider->setGeometry(80, 135, 80, 18);
    colorSpeedSlider->setValue(20);
    colorSpeedSlider->setMinimum(1);
    colorSpeedSlider->setMaximum(30);

    colorSpeedSlider->show();

    connect (colorSpeedSlider, &QSlider::valueChanged, [=]() {
        *colorChangeSpeed = colorSpeedSlider->value();
    });

    this->setMouseTracking(true);
}

void settingsWindow::init()
{
    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);

    reloadStyles();
}
settingsWindow::~settingsWindow()
{
    delete ui;
}
string settingsWindow::qcolorToStr (QColor color)
{
    string res = "rgb(";
    res += to_string (color.red()) + ", ";
    res += to_string (color.green()) + ", ";
    res += to_string (color.blue()) + ")";
    return res;
}
void settingsWindow::reloadStyles() {
    colorSpeedSlider->setStyleSheet("QSlider::groove:horizontal {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #181818;"\
                                    "margin: 7px 0;"\
                                "}" \
                               "QSlider::handle:horizontal {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "width: 5px; " \
                                    "margin: -5px -2px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ " \
                                    "border-radius: 20px; "\
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
                                "}");
}
