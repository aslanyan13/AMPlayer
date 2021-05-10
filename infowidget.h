#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <QSize>
#include <QDesktopWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

namespace Ui {
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget();

    void setName (QString n) { this->name = n; };
    void setDuration (QString dur) { this->duration = dur; };
    void setInfo (QString i) { this->info = i; };
    void setCover (QImage c) { this->cover = c; };

    void popup(int duration);
    void reset() { if (timer != nullptr) timer->stop(); }

private:
    Ui::InfoWidget * ui;

    QTimer * timer = nullptr;

    QString name;
    QString duration;
    QString info;
    QImage cover;

    QLabel * bg;
    QLabel * trackName;
    QLabel * trackInfo;

    void mouseMoveEvent (QMouseEvent * event) {
        if (this->underMouse() || bg->underMouse()) {
            timer->stop();

            QPropertyAnimation * animation = new QPropertyAnimation(this, "windowOpacity");

            animation->setDuration(200);
            animation->setStartValue(this->windowOpacity());
            animation->setEndValue(0);
            animation->start();
        }
    };
};

#endif // INFOWIDGET_H
