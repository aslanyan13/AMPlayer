#include "lyricswindow.h"
#include "ui_lyricswindow.h"

LyricsWindow::LyricsWindow(QWidget *parent) : QWidget(parent), ui(new Ui::LyricsWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Lyrics");
    this->setStyleSheet("background: #121212; color: silver;");

    timer = new QTimer();
    timer->setInterval(10);
    timer->start();

    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);
    fontAwesome.setStyleStrategy(QFont::PreferAntialias);

    lyricsLabel = new QLabel(this);
    lyricsLabel->setGeometry(0, 30, this->width(), this->height() - 30);
    lyricsLabel->setStyleSheet("padding: 20px; color: " + fontColor + "; background: " + backgroundColor + "; font-size: " + QString::number(fontSize) + "px;");
    lyricsLabel->setText("No Lyrics");
    lyricsLabel->setWordWrap(true);
    lyricsLabel->setAlignment(Qt::AlignCenter);
    lyricsLabel->show();

    hint = new QLabel(this);
    hint->setGeometry(0, this->height() - 30, this->width(), 30);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: gray; font-size: 10px; padding: 5px 10px;");
    hint->setWordWrap(true);
    hint->setText("*Hint: To add lyrics press plus button");
    hint->show();

    addLyricsBtn = new QPushButton(this);
    addLyricsBtn->setStyleSheet("color: silver; font-size: 14px; border: 0px solid black;");
    addLyricsBtn->setCursor(Qt::PointingHandCursor);
    addLyricsBtn->setGeometry(10, 5, 20, 20);
    addLyricsBtn->setFont(fontAwesome);
    addLyricsBtn->setText("\uf067");

    QLabel * fontSizeHint = new QLabel("Font size: ", this);
    fontSizeHint->setStyleSheet("font-size: 12px; color: silver");
    fontSizeHint->setGeometry(40, 5, 50, 20);

    fontSizeLine = new QLineEdit(this);
    fontSizeLine->setStyleSheet("padding: 0px 2px; font-size: 12px; border: 1px solid silver; background-color: #141414; color: silver;");
    fontSizeLine->setText(QString::number(fontSize));
    fontSizeLine->setGeometry(100, 5, 50, 20);
    fontSizeLine->show();

    QLabel * textColorHint = new QLabel("Font: ", this);
    textColorHint->setStyleSheet("font-size: 12px; color: silver");
    textColorHint->setGeometry(180, 5, 30, 20);

    textColorBtn = new QPushButton(this);
    textColorBtn->setStyleSheet("background: " + fontColor + "; border: 2px solid silver;");
    textColorBtn->setGeometry(220, 5, 20, 20);
    textColorBtn->setCursor(Qt::PointingHandCursor);
    textColorBtn->show();

    QLabel * bgColorHint = new QLabel("Background: ", this);
    bgColorHint->setStyleSheet("font-size: 12px; color: silver");
    bgColorHint->setGeometry(260, 5, 90, 20);

    bgColorBtn = new QPushButton(this);
    bgColorBtn->setStyleSheet("background: " + backgroundColor + "; border: 2px solid silver;");
    bgColorBtn->setGeometry(340, 5, 20, 20);
    bgColorBtn->setCursor(Qt::PointingHandCursor);
    bgColorBtn->show();

    connect (fontSizeLine, &QLineEdit::textChanged, [&]() {
        if (fontSizeLine->text().toInt() != qSNaN()) {
            fontSize = fontSizeLine->text().toInt();
            reloadStyles();
            fontSizeLine->setStyleSheet("padding: 0px 2px; font-size: 12px; border: 1px solid silver; background-color: #141414; color: silver;");
        }
        else {
            fontSizeLine->setStyleSheet("padding: 0px 2px; font-size: 12px; border: 1px solid silver; background-color: #141414; color: red;");
        }
    });
    connect (textColorBtn, &QPushButton::pressed, [=]() {
        QColor newColor = QColorDialog::getColor(QColor(), this);
        fontColor = qcolor2qstring(newColor);
        reloadStyles();
    });
    connect (bgColorBtn, &QPushButton::pressed, [=]() {
        QColor newColor = QColorDialog::getColor(QColor(), this);
        backgroundColor = qcolor2qstring(newColor);
        reloadStyles();
    });

    connect(timer, &QTimer::timeout, [=]() {
        if (lyrics.size() == 0 && lyricsLabel->text() != "No Lyrics") {
            lyricsLabel->setText("No Lyrics");
            hint->show();
        }
        else if (lyrics.size() > 0)
        {
            if (hint->isVisible())
                hint->hide();

            if (lineCounter != lyrics.size()) {
                std::vector<std::pair<float, QString>>::iterator iter = lyrics.begin() + lineCounter;

                double pos = getPosition();
                if (iter->first <= pos + offset)
                {
                     lyricsLabel->setText(iter->second);
                     lineCounter++;
                }
            }
        }
    });
}

LyricsWindow::~LyricsWindow()
{
    delete ui;
}
