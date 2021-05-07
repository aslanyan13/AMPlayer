#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>
#include <QMouseEvent>

class CustomSlider : public QSlider
{
protected:
    void mousePressEvent (QMouseEvent * event)
    {
        QSlider::mousePressEvent(event);

        if (event->button() == Qt::LeftButton)
        {
            if (orientation() == Qt::Vertical)
                setValue(minimum() + ((maximum() - minimum()) * (height() - event->y())) / height()) ;
            else
                setValue(minimum() + ((maximum() - minimum()) * event->x()) / width()) ;

            event->accept();
        }
    }
public:
    CustomSlider(Qt::Orientation orientation, QWidget * parent = nullptr) : QSlider(orientation, parent) {};
};

#endif // CUSTOMSLIDER_H
