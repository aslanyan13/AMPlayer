#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QColor>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

#include <string>

using namespace std;

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QWidget
{
    Q_OBJECT

public:
    string * mainColorStr = nullptr;
    QColor * mainColor = nullptr;

    explicit settingsWindow(QWidget *parent = nullptr);
    ~settingsWindow();

    void init();
private slots:
    void slot_close() { this->close(); };
private:
    Ui::settingsWindow *ui;

    void mouseMoveEvent (QMouseEvent * event);
    void mousePressEvent (QMouseEvent * event);
    string qcolorToStr (QColor color);

    QPoint lastMousePosition;
    bool moving;

    QWidget * titlebarWidget;
    QLabel * windowTitle;
    QPushButton * closeBtn;
    QPushButton * minimizeBtn;

    QColor colors[16];
};

#endif // SETTINGSWINDOW_H
