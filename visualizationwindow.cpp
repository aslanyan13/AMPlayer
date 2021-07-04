#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

VisualizationWindow::VisualizationWindow(QWidget * parent, HSTREAM * chan) : QWidget(parent), channel(chan), ui(new Ui::VisualizationWindow)
{
    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);
    fontAwesome.setStyleStrategy(QFont::PreferAntialias);

    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    this->setWindowTitle("Visualizations");
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

    previousBtn = new QPushButton(this);
    previousBtn->setFont(fontAwesome);
    previousBtn->setToolTip("Previous");
    previousBtn->setStyleSheet("QPushButton { font-size: 24px; border: 0px solid silver; background-color: #101010; color: silver; border-top-right-radius: 5px; border-bottom-right-radius: 5px; }");
    previousBtn->setGeometry(0, this->height() / 2 - 15, 30, 30);
    previousBtn->setCursor(Qt::PointingHandCursor);
    previousBtn->setText(QString::fromStdWString(L"\uf104"));
    previousBtn->hide();

    nextBtn = new QPushButton(this);
    nextBtn->setFont(fontAwesome);
    nextBtn->setToolTip("Next");
    nextBtn->setStyleSheet("QPushButton { font-size: 24px; border: 0px solid silver; background-color: #101010; color: silver; border-top-left-radius: 5px; border-bottom-left-radius: 5px; }");
    nextBtn->setGeometry(this->width() - 30, this->height() / 2 - 15, 30, 30);
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setText(QString::fromStdWString(L"\uf105"));
    nextBtn->hide();

    connect (previousBtn, &QPushButton::pressed, [=]() {
        mode--;
        if (mode < 0) mode = 2;
    });
    connect (nextBtn, &QPushButton::pressed, [=]() {
        mode++;
        if (mode > 2) mode = 0;
    });

    connect (drawTimer, &QTimer::timeout, [=]() {
        if (this->isHidden()) return;

        repaint();

        if (this->underMouse()) {
            titlebarWidget->show();
            previousBtn->show();
            nextBtn->show();
        }
        else {
            titlebarWidget->hide();
            previousBtn->hide();
            nextBtn->hide();
        }
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
        globalPeaks = std::vector<int>();
        float fft[2048];

        BASS_ChannelGetData(*channel, fft, BASS_DATA_FFT4096);

        painter.setPen(QPen(QColor(255, 255, 255)));
        painter.setBrush(QBrush(QColor(0, 0, 0)));

        painter.drawEllipse((winSize.width() / 2) - 75, (winSize.height() / 2) - 75, 150, 150);

        float angle = PI;
        int points = 1800;

        for (int i = 0; i < points / 2; i++)
        {
            int freq = sqrt(fft[i]) * 3 * 100 - 4;
            if (freq < 0) freq = 0;
            if (freq > 100) freq = 100;

            float x1 = (winSize.width() / 2) + 75 * sin(angle);
            float y1 = (winSize.height() / 2) + 75 * cos(angle);

            float x2 = (winSize.width() / 2) + 75 * sin(angle) * (freq / 100.0f + 1.0);
            float y2 = (winSize.height() / 2) + 75 * cos(angle) * (freq / 100.0f + 1.0);

            painter.drawLine(x1, y1, x2, y2);

            x1 = (winSize.width() / 2) - 75 * sin(angle);
            y1 = (winSize.height() / 2) + 75 * cos(angle);

            x2 = (winSize.width() / 2) - 75 * sin(angle) * (freq / 100.0f + 1.0);
            y2 = (winSize.height() / 2) + 75 * cos(angle) * (freq / 100.0f + 1.0);

            painter.drawLine(x1, y1, x2, y2);

            angle -= PI / (points / 2);
        }
    }
    else if (mode == 1) {
        globalPeaks = std::vector<int>();
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

        float sum = 0;
        for (int i = 1; i <= 1024; i++)
            sum += sqrt(fft[i]) * 3 * this->height() - 4;

        sum /= 256;
        if (sum < 0) sum = 0;
        if (sum > this->height()) sum = this->height();

        globalPeaks.push_back(sum);
        if (globalPeaks.size() > this->width())
            globalPeaks.erase(globalPeaks.begin());

        for (int i = 0; i < globalPeaks.size(); i++)
        {
            painter.setPen(QPen(QColor(255, 255, 255)));
            painter.setBrush(QBrush(QColor(255, 255, 255)));
            painter.drawRect(i, (this->height() - globalPeaks[i]) / 2, 1, globalPeaks[i]);
        }
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
