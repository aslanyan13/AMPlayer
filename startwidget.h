#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPropertyAnimation>

namespace Ui {
class StartWidget;
}

class StartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartWidget(QWidget *parent = nullptr);
    ~StartWidget();

    void popup();
private slots:
private:
    Ui::StartWidget *ui;


    void mousePressEvent (QMouseEvent * event) {
        //if (event->button() == Qt::LeftButton)
            // this->close();
    }
};

#endif // STARTWIDGET_H
