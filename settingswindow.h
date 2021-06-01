#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QFont>
#include <QSlider>
#include <QList>
#include <QTabWidget>
#include <QTabBar>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QColor>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

#include <iostream>
#include <string>

#include "customslider.h"

using namespace std;

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QWidget
{
    Q_OBJECT

public:
    QPushButton * colorBtns[8];

    string * mainColorStr = nullptr;
    QColor * mainColor = nullptr;
    bool * colorChanger = nullptr;
    int * colorChangeSpeed = nullptr;

    string qcolorToStr (QColor color);
    QColor colors[16];

    explicit settingsWindow(QWidget *parent = nullptr);
    ~settingsWindow();

    void init();
    void reloadStyles();

private slots:
    void slot_close() { this->close(); };
private:
    Ui::settingsWindow *ui;

    CustomSlider * colorSpeedSlider;
};

#endif // SETTINGSWINDOW_H
