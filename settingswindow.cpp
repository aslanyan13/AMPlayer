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

    // setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::Window); // Window transparency

    this->setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    this->setStyleSheet("QWidget { background-color: #141414; color: silver; }");
    this->setWindowTitle("Settings");

    /*
    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);
    horizontalLayout->setMargin(0);

    titlebarWidget = new QWidget(this);
    titlebarWidget->setObjectName("titlebarWidget");
    titlebarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titlebarWidget->setLayout(horizontalLayout);
    titlebarWidget->setGeometry(0, 0, 400, 30);
    titlebarWidget->setStyleSheet("color: silver;");

    windowTitle = new QLabel(titlebarWidget);
    windowTitle->setGeometry(0, 0, 400, 30);
    windowTitle->setAlignment(Qt::AlignCenter);
    windowTitle->setObjectName("windowTitle");
    windowTitle->setText("Settings");

    */

    tabs = new QTabWidget(this);

    QWidget * tab1 = new QWidget(tabs);
    tab1->setStyleSheet("color: silver; padding: 15px");

    QLabel * text1 = new QLabel (tab1);
    text1->setStyleSheet("color: silver; font-size: 12px;");
    text1->setText("System color");

    for (int i = 0; i < 7; i++)
    {
        colorBtns[i] = new QPushButton(tab1);
        colorBtns[i]->setCursor(Qt::PointingHandCursor);
        colorBtns[i]->setStyleSheet("padding: 0px; border: 0px solid black; background: " + tr(qcolorToStr(colors[i]).c_str()) + ";");

        colorBtns[i]->setGeometry(20 + i * 35, 40, 30, 30);
        colorBtns[i]->raise();
        colorBtns[i]->show();

        connect(colorBtns[i], &QPushButton::pressed, [=] () {
            *mainColor = colors[i];
            *mainColorStr = qcolorToStr(colors[i]);
            this->reloadStyles();
        });
    }

    QRadioButton * colorChangingRadio = new QRadioButton("Auto color change", tab1);
    colorChangingRadio->setStyleSheet("color: silver; font-size: 12px;");
    colorChangingRadio->move(5, 80);
    colorChangingRadio->show();

    connect (colorChangingRadio, &QRadioButton::clicked, [=]() {
        *colorChanger = colorChangingRadio->isChecked();
    });

    QSlider * colorSpeedSlider = new QSlider(Qt::Horizontal, tab1);
    colorSpeedSlider->move(5, 120);
    colorSpeedSlider->setValue(20);
    colorSpeedSlider->setMinimum(1);
    colorSpeedSlider->setMaximum(30);
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
                                "}");

    colorSpeedSlider->show();

    connect (colorSpeedSlider, &QSlider::valueChanged, [=]() {
        *colorChangeSpeed = colorSpeedSlider->value();
    });

    tabs->addTab(tab1, tr("Theme"));
    tabs->tabBar()->setDrawBase(false);
    tabs->setStyleSheet("QTabWidget { height: 25px; font-size: 12px; border: 0px solid silver; background-color: #101010; color: silver; }" \
                      "QTabWidget::tab:selected { background-color: #141414; color: silver; border: 0px solid silver;}" \
                      "QTabWidget::tab:last { border-right: 0px solid #101010; } " \
                      "QTabWidget::scroller { width: 40px; }" \
                      "QTabWidget::close-button { padding: 4px; image: url(images/close.png); }" \
                      "QTabWidget QToolButton { border: 0px solid black; color: silver; background-color: #101010; }" \
                      "QTabWidget::tab { height: 25px; background-color: #101010; padding: 0px 20px; max-width: 150px; border: 0px solid silver; border-bottom: 1px solid silver; border-right: 4px solid #101010; color: silver; }" \
                      "QTabWidget::tear { border: 0px solid black; }");
    tabs->setGeometry(10, 20, 380, 230);

    tabs->show();

    this->setMouseTracking(true);
}

void settingsWindow::changeColor () {
    cout << "Button pressed!" << endl;
}

void settingsWindow::init()
{
    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);
}

void settingsWindow::reloadStyles() {
    // closeBtn->setStyleSheet("font-size: 24px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr->c_str()) + ";");
}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::mouseMoveEvent (QMouseEvent * event) {
    /*if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->buttons().testFlag(Qt::LeftButton) && moving) {
        this->move(this->pos() + (event->pos() - lastMousePosition));
    }
    */
}
void settingsWindow::mousePressEvent (QMouseEvent * event) {
    /*
    if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->button() == Qt::LeftButton) {
        moving = true;
        lastMousePosition = event->pos();
    }
    */
}
string settingsWindow::qcolorToStr (QColor color)
{
    string res = "rgb(";
    res += to_string (color.red()) + ", ";
    res += to_string (color.green()) + ", ";
    res += to_string (color.blue()) + ")";
    return res;
}
