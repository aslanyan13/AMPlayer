#include "startwidget.h"
#include "ui_startwidget.h"

StartWidget::StartWidget(QWidget *parent) : QWidget(parent), ui(new Ui::StartWidget)
{    
    this->setWindowOpacity(0);
    this->setWindowFlags(Qt::Drawer | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ui->setupUi(this);
}

StartWidget::~StartWidget()
{
    delete ui;
}

void StartWidget::popup() {
    this->show();

    QPropertyAnimation * animation = new QPropertyAnimation(this, "windowOpacity");

    animation->setDuration(300);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
}
