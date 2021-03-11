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

    // setWindowFlags(Qt::Window);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::Window); // Window transparency

    this->setStyleSheet("QWidget { background-color: #212121; }");

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

    this->setMouseTracking(true);
}

void settingsWindow::init()
{
    int color = rand() % 7;
    *mainColor = colors[color];
    *mainColorStr = qcolorToStr(colors[color]);

    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);

    closeBtn = new QPushButton(this);
    closeBtn->setFont(fontAwesome);
    closeBtn->setGeometry(360, 10, 30, 30);
    closeBtn->setStyleSheet("font-size: 24px; border: 0px solid silver; background-color: #212121; color: " + tr(mainColorStr->c_str()) + ";");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setText("\uf00d");
    closeBtn->show();

    QLabel * info = new QLabel(this);
    info->setText("Settings will be added soon!");
    info->setGeometry(0, 50, 400, 50);
    info->setStyleSheet("color: silver;");
    info->setAlignment(Qt::AlignCenter);
    info->show();

    connect (closeBtn, SIGNAL(clicked()), this, SLOT(slot_close()));
}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::mouseMoveEvent (QMouseEvent * event) {
    if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->buttons().testFlag(Qt::LeftButton) && moving) {
        this->move(this->pos() + (event->pos() - lastMousePosition));
    }
}
void settingsWindow::mousePressEvent (QMouseEvent * event) {
    if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->button() == Qt::LeftButton) {
        moving = true;
        lastMousePosition = event->pos();
    }
}
string settingsWindow::qcolorToStr (QColor color)
{
    string res = "rgb(";
    res += to_string (color.red()) + ", ";
    res += to_string (color.green()) + ", ";
    res += to_string (color.blue()) + ")";
    return res;
}