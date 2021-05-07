#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

VisualizationWindow::VisualizationWindow(QWidget * parent, HSTREAM * chan) : QWidget(parent), channel(chan), ui(new Ui::VisualizationWindow)
{
    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);
    fontAwesome.setStyleStrategy(QFont::PreferAntialias);

    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint/* | Qt::WindowStaysOnTopHint*/); // Window transparency
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    drawTimer = new QTimer(this);
    drawTimer->setInterval(1000 / 60);
    drawTimer->start();

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);
    horizontalLayout->setMargin(0);

    titlebarWidget = new QWidget(this);
    titlebarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titlebarWidget->setLayout(horizontalLayout);
    titlebarWidget->setGeometry(0, 0, this->size().width(), 30);
    titlebarWidget->setStyleSheet("background: #101010; color: silver; border: 2px solid black;");

    windowTitle = new QLabel(titlebarWidget);
    windowTitle->setGeometry(0, 0, this->size().width(), 30);
    windowTitle->setAlignment(Qt::AlignCenter);
    QFont font = windowTitle->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
    windowTitle->setFont(font);
    windowTitle->setText("Visualizations");

    closeBtn = new QPushButton(titlebarWidget);
    closeBtn->setFont(fontAwesome);
    closeBtn->setToolTip("Close");
    closeBtn->setGeometry(this->size().width() - 25, 7, 15, 15);
    closeBtn->setStyleSheet("QPushButton { font-size: 18px; border: 0px solid silver; background-color: #101010; color: silver; }");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setText(QString::fromStdWString(L"\uf00d"));
    closeBtn->show();

    minimizeBtn = new QPushButton(titlebarWidget);
    minimizeBtn->setFont(fontAwesome);
    minimizeBtn->setToolTip("Minimize");
    minimizeBtn->setGeometry(this->size().width() - 50, 7, 15, 15);
    minimizeBtn->setStyleSheet("QPushButton { font-size: 13px; border: 0px solid silver; background-color: #101010; color: silver; }");
    minimizeBtn->setCursor(Qt::PointingHandCursor);
    minimizeBtn->setText(QString::fromStdWString(L"\uf2d1"));
    minimizeBtn->show();

    connect (drawTimer, &QTimer::timeout, [=]() {
        repaint();

        if (this->underMouse())
            titlebarWidget->show();
        else
            titlebarWidget->hide();
    });

    connect (closeBtn, &QPushButton::pressed, [=]() {
        this->close();
    });
    connect (minimizeBtn, &QPushButton::pressed, [=]() {
        setWindowState(Qt::WindowMinimized);
    });
}

VisualizationWindow::~VisualizationWindow()
{
    delete ui;
}

void VisualizationWindow::paintEvent(QPaintEvent * event) {
    Q_UNUSED(event);

    const QSize winSize = this->size();

    QPainter painter(this);

    if (!transparentBg)
    {
        painter.setPen(QPen(QColor(0, 0, 0)));
        painter.setBrush(QBrush(QColor(0, 0, 0)));
        painter.drawRect(0, 0, winSize.width(), winSize.height());
    }

    if (*channel == NULL)
        return;

    if (mode == 0)
    {
        painter.setPen(QPen(QColor(255, 255, 255)));
        painter.setBrush(QBrush(QColor(0, 0, 0)));

        painter.drawEllipse((winSize.width() / 2) - 75, (winSize.height() / 2) - 75, 150, 150);

        float angle = PI;
        int points = 3600;

        for (int i = 0; i < points; i++)
        {
            float x1 = (winSize.width() / 2) + 75 * sin(angle);
            float y1 = (winSize.height() / 2) + 75 * cos(angle);

            float x2 = (winSize.width() / 2) + 75 * sin(angle) * (i / (float)points + 1.0);
            float y2 = (winSize.height() / 2) + 75 * cos(angle) * (i / (float)points + 1.0);

            painter.drawLine(x1, y1, x2, y2);
            angle -= PI / (points / 2);
        }
    }
    else if (mode == 1) {
        float fft[1024];

        BASS_ChannelGetData(*channel, fft, BASS_DATA_FFT2048);

        int points = 64;

        float margin = 4;

        float blockHeight = 6;
        float blockWidth = (winSize.width() - points * margin) / points;

        for (int i = 0; i < points; i++)
        {
            int maxBlocksN = (winSize.height() - 0.6 * winSize.height()) / (margin + blockHeight);
            int blocksN = sqrt(fft[i + 1]) * maxBlocksN * 3 - 4;

            if (maxBlocksN < blocksN) blocksN = maxBlocksN;

            for (int j = 0; j < blocksN; j++)
            {
                float x = margin + i * (margin + blockWidth);
                float y = winSize.height() - j * (margin + blockHeight) - winSize.height() / 2;

                // painter.setPen(QPen(QColor(255 * i / points, 0, 255 - 255 * i / points, 255 - 120 * j / blocksN)));
                // painter.setBrush(QBrush(QColor(255 * i / points, 0, 255 - 255 * i / points, 255 - 120 * j / blocksN)));
                painter.setPen(QPen(QColor(255 * i / points, 255 - 255 * i / points, 0, 255 - 120 * j / blocksN)));
                painter.setBrush(QBrush(QColor(255 * i / points, 255 - 255 * i / points, 0, 255 - 120 * j / blocksN)));
                painter.drawRect(x, y, blockWidth, blockHeight);
            }

            for (int j = 1; j < blocksN; j++)
            {
                // painter.setPen(QPen(QColor(255 * i / points, 0, 255 - 255 * i / points, 100 - 80 * j / blocksN)));
                // painter.setBrush(QBrush(QColor(255 * i / points, 0, 255 - 255 * i / points, 100 - 80 * j / blocksN)));
                painter.setPen(QPen(QColor(255 * i / points, 255 - 255 * i / points, 0, 100 - 80 * j / blocksN)));
                painter.setBrush(QBrush(QColor(255 * i / points, 255 - 255 * i / points, 0, 100 - 80 * j / blocksN)));

                float x = margin + i * (margin + blockWidth);
                float y = j * (margin + blockHeight) + winSize.height() / 2;

                painter.drawRect(x, y, blockWidth, blockHeight);
            }
        }
    }
    else if (mode == 2) {
        float fft[1024];

        BASS_ChannelGetData(*channel, fft, BASS_DATA_FFT2048);

        std::vector <int> peaks;

        for (int j = 0; j < 100; j++)
        {
            int max = sqrt(fft[1]) * 3 * winSize.height() - 4;
            for (int k = 2; k < 512; k++)
            {
                int value = sqrt(fft[k]) * 3 * winSize.height() - 4;

                if (value < 0) value = 0;
                if (value > winSize.height()) value = winSize.height();

                if (value > max && count(peaks.begin(), peaks.end(), value) == 0)
                    max = value;
            }
            peaks.push_back(max);
        }

        for (int i = 0; i < 512; i++)
        {
            int blocksH = sqrt(fft[i + 1]) * winSize.height() * 3 - 4;

            if (blocksH < 0) blocksH = 0;
            if (blocksH > winSize.height()) blocksH = winSize.height();

            if (count(peaks.begin(), peaks.end(), blocksH) != 0) {
                peaks.erase(std::find(peaks.begin(), peaks.end(), blocksH));
                painter.setPen(QPen(QColor(255, 0, 0)));
                painter.setBrush(QBrush(QColor(255, 0, 0)));
            } else {
                painter.setPen(QPen(QColor(255, 255, 255)));
                painter.setBrush(QBrush(QColor(255, 255, 255)));
            }


            painter.drawLine(i, winSize.height() - blocksH, i, (winSize.height() - blocksH) + blocksH);
        }

        painter.setPen(QPen(QColor(255, 0, 0)));
        painter.setBrush(QBrush(QColor(255, 0, 0)));
        painter.drawLine(0, winSize.height() - (winSize.height() * 0.05f), winSize.width(), winSize.height() - (winSize.height() * 0.05f));
    }
}

void VisualizationWindow::mouseMoveEvent (QMouseEvent * event) {
    // if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
    //    return;

    if(event->buttons().testFlag(Qt::LeftButton) && moving) {
        this->move(this->pos() + (event->pos() - lastMousePosition));
    }
}

void VisualizationWindow::mousePressEvent (QMouseEvent * event) {
    // if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
    //    return;

    if(event->button() == Qt::LeftButton) {
        moving = true;
        lastMousePosition = event->pos();
    }
}
