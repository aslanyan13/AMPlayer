#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QFontDatabase>
#include <QFont>

#include "bass.h"

#include <math.h>
#include <vector>

const float PI = 3.14159f;

namespace Ui {
class VisualizationWindow;
}

class VisualizationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit VisualizationWindow(QWidget *parent = nullptr, HSTREAM * chan = nullptr);
    ~VisualizationWindow();

    QPushButton * closeBtn;
private:
    bool transparentBg = false;
    bool alwaysOnTop = false;

    QWidget * titlebarWidget;

    Ui::VisualizationWindow * ui;

    int mode = 2;

    HSTREAM * channel;

    QTimer * drawTimer;

    QLabel * windowTitle;

    QPushButton * minimizeBtn;

    QPoint lastMousePosition;
    bool moving;


    void paintEvent(QPaintEvent * event);
    void mousePressEvent (QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent * event);

};

#endif // VISUALIZATIONWINDOW_H
