#include "infowidget.h"
#include "ui_infowidget.h"

InfoWidget::InfoWidget(QWidget *parent) : QWidget(parent), ui(new Ui::InfoWidget)
{
    ui->setupUi(this);

    this->setWindowOpacity(0);
    this->setMouseTracking(true);
    this->setGeometry(0, 0, QApplication::desktop()->screenGeometry().width(), 80);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint | Qt::Drawer);
    this->setAttribute(Qt::WA_TranslucentBackground);

    bg = new QLabel(this);
    bg->setMouseTracking(true);
    bg->setGeometry(0, 0, QApplication::desktop()->screenGeometry().width(), 80);
    bg->setStyleSheet("background: rgba(0, 0, 0, 0.5);");
    bg->show();

    QGraphicsDropShadowEffect * textShadow = new QGraphicsDropShadowEffect(this);
    textShadow->setBlurRadius(10);
    textShadow->setColor(QColor(0, 0, 0));
    textShadow->setOffset(0, 0);

    trackName = new QLabel (this);
    trackName->setMouseTracking(true);
    trackName->setGeometry(0, 5, QApplication::desktop()->screenGeometry().width(), 45);
    trackName->setStyleSheet("color: white; font-size: 22px;");
    trackName->setAlignment(Qt::AlignCenter);
    trackName->setGraphicsEffect(textShadow);
    trackName->show();

    trackInfo = new QLabel (this);
    trackInfo->setMouseTracking(true);
    trackInfo->setGeometry(0, 50, QApplication::desktop()->screenGeometry().width(), 20);
    trackInfo->setStyleSheet("color: white; font-size: 16px;");
    trackInfo->setAlignment(Qt::AlignCenter);
    trackInfo->setGraphicsEffect(textShadow);
    trackInfo->show();
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::popup(int duration) {
    trackName->setText("♫ " + this->name + " [" + this->duration +  "] " + " ♫");
    trackInfo->setText(this->info);

    QPropertyAnimation * animation = new QPropertyAnimation(this, "windowOpacity");

    animation->setDuration(1000);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(duration);
    timer->start();
    connect(timer, &QTimer::timeout, [=]() {
        QPropertyAnimation * animation = new QPropertyAnimation(this, "windowOpacity");

        animation->setDuration(1000);
        animation->setStartValue(this->windowOpacity());
        animation->setEndValue(0);
        animation->start();
    });
}
