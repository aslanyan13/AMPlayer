#include "mainwindow.h"
#include "ui_mainwindow.h"

QColor mainColor(255, 37, 79);
QColor nextColor(255, 37, 79);
string mainColorStr = "rgb(255, 37, 79)";
int colorChangeSpeed = 20;

bool isOrderChanged = false;

QImage applyEffectToImage(QImage src, QGraphicsEffect * effect, int extent = 0)
{
    if(src.isNull()) return QImage();   // No need to do anything else!
    if(!effect) return src;             // No need to do anything else!

    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);

    QImage res(src.size()+QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);

    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));

    return res;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QApplication::setActiveWindow(this);

    startWidget = new StartWidget();
    startWidget->popup();

    if (QtWin::isCompositionEnabled())
        QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
    else
        QtWin::resetExtendedFrame(this);

    QWinTaskbarButton * windowsTaskbarButton = new QWinTaskbarButton(this);    //Create the taskbar button which will show the progress
    windowsTaskbarButton->setWindow(this->windowHandle());    //Associate the taskbar button to the progress bar, assuming that the progress bar is its own window

    taskbarProgress = windowsTaskbarButton->progress();
    taskbarProgress->show();

    trayIcon = new QSystemTrayIcon(QIcon(":/Images/cover-placeholder.png"));
    trayIcon->setVisible(false);
    trayIcon->setToolTip("AMPlayer");
    connect (trayIcon, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showWindow();
        }
        if (reason == QSystemTrayIcon::Context)
            trayContext();
    });

    removeControlServer = new QWebSocketServer("Remote Control Server", QWebSocketServer::NonSecureMode, this);

    httpServer = new QTcpServer(this);
    httpServerPort = rand() % 10000 + 10000;

    starttime = clock();

    if (logging) {
        int current = time(NULL);

        logfile.setFileName(QDir::currentPath() + "/logs/log_" + QString::number(current) + ".txt");

        if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Can't open log file";
            logging = false;
        } else {
            writeLog ("Program initialized");
        }
    }

    configFile.setFileName(QDir::currentPath() + "/config.cfg");

    if (!configFile.open(QIODevice::ReadWrite | QIODevice::Text))
        qDebug() << "Error open config file!";
    else {
        configLoaded = true;
        qDebug() << "Config file opened!";
    }

    // Reading playlists from XML file
    XMLreader = new PlaylistReader(QDir::currentPath() + "/XML/playlists.xml");
    XMLreader->readPlaylists(playlists);

    if (playlists.size() > 0)
    {
        writeLog ("Playlists loaded from XML file");
        currentPlaylistName = playlists.begin()->first; // Setting current playlist name
        playlist = playlists[currentPlaylistName];      // Setting current playlist
    }
    else {
        writeErrorLog ("Can't load/open playlists XML file");
        currentPlaylistName = "Default";       // Setting current playlist name
        playlist = playlists ["Default"];      // Setting current playlist
    }

    clearPrerenderedFft();

    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);
    fontAwesome.setStyleStrategy(QFont::PreferAntialias);

    channel = NULL;

    timer = new QTimer();
    timer->setInterval(1000 / 60); // 60 FPS
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start();

    ui->setupUi(this);

    // Window settings
    this->setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint); // Window transparency
    this->setStyleSheet("QMainWindow { background-color: #101010; }"
                        "QInputDialog, QInputDialog * { background-color: #101010; color: silver; }"
                        "QToolTip { background-color: #101010; color: silver; font-size: 12px; border: 1px solid silver; }");

    this->setWindowTitle("AMPlayer");
    this->setMouseTracking(true);
    this->centralWidget()->setMouseTracking(true);

    QRect rec = QApplication::desktop()->screenGeometry();

    // Album cover load
    QImageReader reader(QDir::currentPath() + "/Images/cover-placeholder2.png");
    cover = reader.read();

    if (cover.isNull()) {
        coverLoaded = false;
        cover = QImage (":/Images/cover-placeholder.png");

        qDebug() << reader.errorString();
        writeLog ("Can't load cover! " + reader.errorString());
    }

    settingsWin = new settingsWindow();

    equalizerWin = new equalizerWindow();
    equalizerWin->mainColorStr = &mainColorStr;
    equalizerWin->reloadStyles();

    visualWin = new VisualizationWindow(nullptr, &channel);

    marksWin = new QWidget();
    marksWin->setMaximumSize(300, 350);
    marksWin->setMinimumSize(300, 350);
    marksWin->setWindowIcon(QIcon(":/Images/cover-placeholder.png"));
    marksWin->setStyleSheet("background: #101010; color: silver;");
    marksWin->setWindowFlags(Qt::Drawer);
    marksWin->setWindowTitle("Marks");
    marksWin->setGeometry(this->pos().x() + (this->size().width() - 300) / 2, this->pos().y() + (this->size().height() - 350) / 2, 300, 350);

    marksList = new QListWidget(marksWin);
    marksList->setGeometry(10, 10, 280, 290);
    marksList->setContextMenuPolicy(Qt::CustomContextMenu);
    marksList->setStyleSheet("QListWidget { font-size: 13px; border: 1px solid transparent; outline: none; background-color: #141414; color: silver; padding: 5px; }"
                             "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }"
                             "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");
    marksList->show();


    QPushButton * addMarkBtn = new QPushButton(marksWin);
    addMarkBtn->setFont(fontAwesome);
    addMarkBtn->setDisabled(false);
    addMarkBtn->setGeometry(10, 290, 280, 30);
    addMarkBtn->setCursor(Qt::PointingHandCursor);
    addMarkBtn->setStyleSheet("QPushButton { font-size: 14px; background: #181818; color: silver; border: 0px solid black; }");
    addMarkBtn->setText("\uf067");

    QLabel * hint = new QLabel(marksWin);
    hint->setGeometry(10, 330, 280, 20);
    hint->setStyleSheet("font-size: 10px; color: gray;");
    hint->setAlignment(Qt::AlignVCenter);
    hint->setText("*Hint: Double-click to jump to mark, right click to edit");

    initTimerWindow();

    connect (addMarkBtn, SIGNAL(clicked()), this, SLOT(addMark()));
    connect (marksList, &QListWidget::customContextMenuRequested, [=](const QPoint& point) {
        int itemIndex = marksList->indexAt(point).row();
        qDebug() << itemIndex;

        QPoint globalPos = marksList->mapToGlobal(point);

        QMenu myMenu;

        myMenu.addAction("Edit");
        myMenu.addAction("Remove");

        myMenu.setStyleSheet("QMenu { icon-size: 8px; background-color: #101010; color: silver; }");

        QAction * selectedItem = myMenu.exec(globalPos);

        if (selectedItem)
        {
            if (selectedItem->text() == "Edit") {

            }
            if (selectedItem->text() == "Remove")
                removeMark();
        }
    });

    connect (marksList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem * item) {
        QString itemText = item->text();
        int start = itemText.indexOf('[');
        int end = itemText.mid(start + 1).indexOf(']');

        QString timecode = itemText.mid(start + 1, end);

        setPosition(qstring2seconds(timecode));
    });

    infoWidget = new InfoWidget();
    infoWidget->show();

    lyricsWin = new LyricsWindow();
    lyricsWin->channel = &channel;

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);
    horizontalLayout->setMargin(0);

    titlebarWidget = new QWidget(this);
    titlebarWidget->setObjectName("titlebarWidget");
    titlebarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titlebarWidget->setLayout(horizontalLayout);
    titlebarWidget->setGeometry(0, 0, 800, 30);
    titlebarWidget->setStyleSheet("color: silver;");

    windowTitle = new QLabel(titlebarWidget);
    windowTitle->setGeometry(0, 0, 800, 30);
    windowTitle->setAlignment(Qt::AlignCenter);
    QFont font = windowTitle->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
    windowTitle->setFont(font);
    windowTitle->setText("AMPlayer");

    QPushButton * menuBtn = new QPushButton(this);
    menuBtn->setFont(fontAwesome);
    menuBtn->setToolTip("Menu");
    menuBtn->setGeometry(10, 10, 15, 15);
    menuBtn->setStyleSheet("QPushButton { font-size: 16px; border: 0px solid silver; background-color: #101010; color: gray; }");
    menuBtn->setCursor(Qt::PointingHandCursor);
    menuBtn->setText(QString::fromStdWString(L"\uf0c9"));
    menuBtn->show();

    closeBtn = new QPushButton(this);
    closeBtn->setFont(fontAwesome);
    closeBtn->setToolTip("Close (Ctrl+Q)");
    closeBtn->setGeometry(775, 11, 15, 15);
    closeBtn->setStyleSheet("QPushButton { font-size: 18px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setText(QString::fromStdWString(L"\uf00d"));
    closeBtn->show();

    minimizeBtn = new QPushButton(this);
    minimizeBtn->setFont(fontAwesome);
    minimizeBtn->setToolTip("Minimize");
    minimizeBtn->setGeometry(750, 10, 15, 15);
    minimizeBtn->setStyleSheet("QPushButton { font-size: 13px; border: 0px solid silver; background-color: #101010; color: silver; }");
    minimizeBtn->setCursor(Qt::PointingHandCursor);
    minimizeBtn->setText(QString::fromStdWString(L"\uf2d1"));
    minimizeBtn->show();

    pauseBtn = new QPushButton(this);
    pauseBtn->setFont(fontAwesome);
    pauseBtn->setGeometry(380, 285, 50, 50);
    pauseBtn->setToolTip("Play/Pause (Space)");
    pauseBtn->setStyleSheet("QPushButton { font-size: 36px; border: 0px solid silver; background-color: #101010; color: silver; }");
    pauseBtn->setText("\uf04b");
    pauseBtn->setCursor(Qt::PointingHandCursor);
    pauseBtn->show();

    QPushButton * forwardBtn = new QPushButton(this);
    forwardBtn->setFont(fontAwesome);
    forwardBtn->setGeometry(455, 290, 40, 40);
    forwardBtn->setToolTip("Next Track (Ctrl + Right)");
    forwardBtn->setStyleSheet("QPushButton { vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #101010; color: silver; }");
    forwardBtn->setText("\uf04e");
    forwardBtn->setCursor(Qt::PointingHandCursor);
    forwardBtn->show();

    QPushButton * backwardBtn = new QPushButton(this);
    backwardBtn->setFont(fontAwesome);
    backwardBtn->setGeometry(315, 290, 40, 40);
    backwardBtn->setToolTip("Previous Track (Ctrl + Left)");
    backwardBtn->setStyleSheet("QPushButton { vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #101010; color: silver; }");
    backwardBtn->setText("\uf04a");
    backwardBtn->setCursor(Qt::PointingHandCursor);
    backwardBtn->show();

    repeatBtn = new QPushButton(this);
    repeatBtn->setFont(fontAwesome);
    repeatBtn->setGeometry(510, 291, 30, 30);
    repeatBtn->setToolTip("Track Repeat");
    repeatBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    repeatBtn->setText("\uf363");
    repeatBtn->setCursor(Qt::PointingHandCursor);
    repeatBtn->show();

    shuffleBtn = new QPushButton(this);
    shuffleBtn->setFont(fontAwesome);
    shuffleBtn->setGeometry(265, 291, 30, 30);
    shuffleBtn->setToolTip("Playlist Shuffle");
    shuffleBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    shuffleBtn->setText("\uf074");
    shuffleBtn->setCursor(Qt::PointingHandCursor);
    shuffleBtn->show();

    equoBtn = new QPushButton(this);
    equoBtn->setFont(fontAwesome);
    equoBtn->setGeometry(225, 291, 30, 30);
    equoBtn->setToolTip("Equalizer");
    equoBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    equoBtn->setText("\uf3f1");
    equoBtn->setCursor(Qt::PointingHandCursor);
    equoBtn->show();

    remoteBtn = new QPushButton(this);
    remoteBtn->setFont(fontAwesome);
    remoteBtn->setGeometry(105, 291, 30, 30);
    remoteBtn->setToolTip("Remote Control");
    remoteBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    remoteBtn->setText("\uf3cd");
    remoteBtn->setCursor(Qt::PointingHandCursor);
    remoteBtn->setMouseTracking(true);
    remoteBtn->show();

    lyricsBtn = new QPushButton(this);
    lyricsBtn->setFont(fontAwesome);
    lyricsBtn->setGeometry(145, 291, 30, 30);
    lyricsBtn->setToolTip("Lyrics");
    lyricsBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    lyricsBtn->setText("\uf20a");
    lyricsBtn->setCursor(Qt::PointingHandCursor);
    lyricsBtn->setMouseTracking(true);
    lyricsBtn->show();

    timerBtn = new QPushButton (this);
    timerBtn->setFont(fontAwesome);
    timerBtn->setGeometry(185, 291, 30, 30);
    timerBtn->setToolTip("Timer");
    timerBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    timerBtn->setText("\uf017");
    timerBtn->setCursor(Qt::PointingHandCursor);
    timerBtn->setMouseTracking(true);
    timerBtn->show();

    marksBtn = new QPushButton(this);
    marksBtn->setFont(fontAwesome);
    marksBtn->setDisabled(true);
    marksBtn->setGeometry(550, 291, 30, 30);
    marksBtn->setToolTip("Marks (Ctrl + M)");
    marksBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }"
                            "QPushButton:disabled { color: gray; }");
    marksBtn->setText("\uf02e");
    marksBtn->setCursor(Qt::PointingHandCursor);
    marksBtn->show();

    visualBtn = new QPushButton(this);
    visualBtn->setFont(fontAwesome);
    visualBtn->setGeometry(590, 291, 30, 30);
    visualBtn->setToolTip("Visualizations");
    visualBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    visualBtn->setText("\uf26c");
    visualBtn->setCursor(Qt::PointingHandCursor);
    visualBtn->show();

    volumeBtn = new QPushButton (this);
    volumeBtn->setFont(fontAwesome);
    volumeBtn->setGeometry(630, 291, 30, 30);
    volumeBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    volumeBtn->setCursor(Qt::PointingHandCursor);
    volumeBtn->setText("\uf028");
    volumeBtn->setMouseTracking(true);
    volumeBtn->show();

    songTitle = new QLabel(this);
    songTitle->setText("");
    songTitle->setGeometry(200, 220, 400, 30);
    songTitle->setAlignment(Qt::AlignCenter);
    songTitle->setStyleSheet("/* border: 1px solid silver; */ font-size: 20px; color: silver;");
    songTitle->show();

    songInfo = new QLabel(this);
    songInfo->setText("");
    songInfo->setGeometry(200, 255, 400, 16);
    songInfo->setAlignment(Qt::AlignCenter);
    songInfo->setStyleSheet("/* border: 1px solid silver; */ color: gray;");
    songInfo->show();

    timecode = new QLabel (this);
    timecode->raise();
    timecode->setGeometry(0, 0, 0, 0);
    timecode->setAlignment(Qt::AlignCenter);
    timecode->setMouseTracking(true);
    timecode->installEventFilter(this);
    timecode->setStyleSheet("padding: 3px 5px; font-size: 11px; border: 1px solid silver; background-color: #101010; color: silver;");

    playlistsBar = new QTabBar(this);
    playlistsBar->setFont(fontAwesome);
    playlistsBar->setDocumentMode(true);
    playlistsBar->setDrawBase(false);
    playlistsBar->setExpanding (false);
    playlistsBar->setMouseTracking(true);
    playlistsBar->setGeometry(15, 405, 775, 25);
    playlistsBar->lower();
    playlistsBar->setContextMenuPolicy(Qt::CustomContextMenu);
    playlistsBar->setStyleSheet("QTabBar { height: 25px; font-size: 12px; border: 0px solid silver; background-color: #101010; color: silver; }" \
                                "QTabBar::tab:selected { background-color: #141414; color: " + QString::fromStdString(mainColorStr) + ";}" \
                                "QTabBar::tab:last { border-right: 0px solid #101010; } " \
                                "QTabBar::scroller { width: 40px; }" \
                                "QTabBar::close-button { padding: 4px; image: url(images/close.png); }" \
                                "QTabBar QToolButton { border: 0px solid black; color: silver; background-color: #101010; }" \
                                "QTabBar::tab { height: 25px; background-color: #101010; padding: 0px 20px; max-width: 150px; border: 0px solid silver; border-bottom: 1px solid silver; border-right: 4px solid #101010; color: silver; }" \
                                "QTabBar::tear { border: 0px solid black; }");

    playlistsBar->show();


    playlistWidget = new QListWidget (this);
    playlistWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    playlistWidget->setMouseTracking(true);
    playlistWidget->installEventFilter(this);
    playlistWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScrollBar * vbar = playlistWidget->verticalScrollBar();
    vbar->setStyle( new QCommonStyle );
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #101010; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar * hbar = playlistWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #101010; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    playlistWidget->setDragDropMode(QAbstractItemView::DragDrop);
    playlistWidget->setDefaultDropAction(Qt::MoveAction);
    playlistWidget->setGeometry(15, 460, 775, 160);
    playlistWidget->lower();
    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; padding-left: 15px; font-size: 14px; /*border: 1px solid silver;*/ border-radius: 5px; border-top-left-radius: 0px; background-color: #141414; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");
    playlistWidget->show();

    searchSong = new QLineEdit(this);
    searchSong->setMouseTracking(true);
    searchSong->setFont(fontAwesome);
    searchSong->setStyleSheet("QMenu { background-color: #101010; color: silver; }"
                              "QLineEdit { padding: 0px 20px; font-size: 12px; border: 0px solid silver; border-bottom: 1px solid #101010; background-color: #141414; color: silver; }");
    searchSong->setGeometry(15, 430, 775, 30);
    searchSong->setPlaceholderText("\uf002 Search song");
    searchSong->show();

    songPosition = new QLabel(this);
    songPosition->setMouseTracking(true);
    songPosition->setText("00:00");
    songPosition->setGeometry(15, 363, 200, 20);
    songPosition->setAlignment(Qt::AlignLeft);
    songPosition->setStyleSheet("/* border: 1px solid silver; background-color: #101010; */ color: gray;");
    songPosition->show();

    songDuration = new QLabel(this);
    songDuration->setMouseTracking(true);
    songDuration->setText("00:00");
    songDuration->setGeometry(605, 363, 180, 20);
    songDuration->setAlignment(Qt::AlignRight);
    songDuration->setStyleSheet("/* border: 1px solid silver; background-color: #101010; */ color: gray;");
    songDuration->show();

    volumeSlider = new CustomSlider (Qt::Horizontal, this);
    volumeSlider->setMaximum(100);
    volumeSlider->setMinimum(0);
    volumeSlider->setValue(100);
    volumeSlider->setCursor(Qt::PointingHandCursor);
    volumeSlider->setGeometry(670, 302, 115, 20);
    volumeSlider->setStyleSheet("QSlider::groove:horizontal {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 7px 0px;"\
                                "}" \
                               "QSlider::handle:horizontal {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "width: 5px; " \
                                    "margin: -5px -2px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::sub-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0px;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}" \
                                "QSlider::add-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0px;" \
                                    "background: silver; " \
                                "}");

    volumeSlider->hide();

    connect (lyricsWin->addLyricsBtn, &QPushButton::clicked, [=]() {
        loadLyrics(&playlists[playingSongPlaylist][currentID]);
        if (currentPlaylistName == playingSongPlaylist && searchSong->text() == "")
            playlist[currentID] = playlists[playingSongPlaylist][currentID];
    });
    connect (lyricsWin, &LyricsWindow::closed, [=]() {
        lyricsEnabled = false;
        lyricsBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    });

    connect (equalizerWin->enabledCheckBox, &QCheckBox::stateChanged, [=](int state) {
        equoEnabled = (state == Qt::Checked);

        if (currentID == -1) return;

        Song temp = playlists[playingSongPlaylist][currentID];
        double pos = getPosition();

        BASS_StreamFree(channel);

        if (equoEnabled) {
            createStream(channel, temp.path, BASS_STREAM_DECODE | BASS_STREAM_PRESCAN | BASS_SAMPLE_FX);
            equalizerWin->channel = &channel;
            equalizerWin->init();
            equoBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
        } else {
            createStream(channel, temp.path, 0);

            equoBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
        }

        if (muted) BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0);
        else BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

        setPosition(pos);
    });

    // If visualizations window closed
    connect (visualWin->closeBtn, &QPushButton::pressed, [=]() {
        qDebug() << "Closed!";
        visualWindowOpened = false;
        visualBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
        this->setFocus();
    });

    connect (searchSong, SIGNAL(textChanged(const QString &)), this, SLOT(search(const QString &)));
    connect (searchSong, &QLineEdit::returnPressed, [=]() {
        lastTrackID = currentID;
        lastPlaylistName = playingSongPlaylist;
        playingSongPlaylist = currentPlaylistName;
        setActive(0);
    });

    connect (playlistsBar, &QTabBar::tabBarClicked, [=](int index) {
        if (playlistsBar->tabText(index) == "\uf067")
        {
            playlistCreating = true;
            createPlaylist();
        }
    });
    connect (playlistsBar, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT(playlistsBarContextMenu (const QPoint&)));
    connect (playlistsBar, SIGNAL (currentChanged(int)), this, SLOT(changeCurrentPlaylist (int)));
    connect (playlistsBar, SIGNAL (tabCloseRequested(int)), this, SLOT(removePlaylist (int)));

    connect (playlistWidget, &QListWidget::customContextMenuRequested, [=](const QPoint& point) {
        if (playlistWidget->itemAt(point)) {
            int itemIndex = playlistWidget->itemAt(point)->data(Qt::UserRole).toInt();
            qDebug() << itemIndex;

            QPoint globalPos = playlistWidget->mapToGlobal(point);

            QMenu myMenu;

            QAction playAction;
            playAction.setText("Play");
            playAction.setObjectName("play");
            playAction.setIconVisibleInMenu(true);
            myMenu.addAction(&playAction);

            myMenu.addSeparator();

            QAction editAction;
            editAction.setText("Edit");
            editAction.setObjectName("edit");
            editAction.setIconVisibleInMenu(true);
            myMenu.addAction(&editAction);

            myMenu.addAction("Remove");

            myMenu.setStyleSheet("QMenu { icon-size: 8px; background-color: #101010; color: silver; }"
                                 "QMenu::item#play { icon-size: 8px; }"
                                 "QMenu::item#edit { icon-size: 12px; }"
                                 "QMenu::item { background: transparent; }");

            QAction * selectedItem = myMenu.exec(globalPos);

            if (selectedItem)
            {
                if (selectedItem->text() == "Play") {
                    lastPlaylistName = playingSongPlaylist;
                    playingSongPlaylist = currentPlaylistName;
                    lastTrackID = currentID;
                    setActive(itemIndex);
                }
                if (selectedItem->text() == "Remove")
                    removeFile();
            }
        }
    });
    connect (playlistWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(setActive(QListWidgetItem *)));
    connect (playlistWidget->model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(rowsMoved(QModelIndex, int, int, QModelIndex, int)));

    connect (menuBtn, SIGNAL(clicked()), this, SLOT(menuContext()));
    connect (lyricsBtn, &QPushButton::pressed, [=]() {
        lyricsWin->show();
        lyricsWin->raise();
        lyricsWin->setFocus();

        lyricsEnabled = true;
        reloadStyles();
    });
    connect (minimizeBtn, SIGNAL(clicked()), this, SLOT(slot_minimize()));
    connect (closeBtn, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect (forwardBtn, SIGNAL(clicked()), this, SLOT(forward()));
    connect (backwardBtn, SIGNAL(clicked()), this, SLOT(backward()));
    connect (pauseBtn, SIGNAL(clicked()), this, SLOT(pause()));
    connect (volumeBtn, &QPushButton::clicked, [=]() {
        muted = !muted;

        if (muted) {
            BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0);
            volumeBtn->setText("\uf026");
        } else {
            BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
            volumeBtn->setText("\uf028");
        }
    });
    connect (repeatBtn, SIGNAL(clicked()), this, SLOT(changeRepeat()));
    connect (shuffleBtn, SIGNAL(clicked()), this, SLOT(changeShuffle()));
    connect (marksBtn, SIGNAL(clicked()), this, SLOT(marksShow()));
    connect (equoBtn, SIGNAL(clicked()), this, SLOT(equalizer()));
    connect (timerBtn, SIGNAL(clicked()), this, SLOT(showSongTimer()));

    connect (remoteBtn, SIGNAL(clicked()), this, SLOT(remoteControl()));

    connect (visualBtn, SIGNAL(clicked()), this, SLOT(visualizations()));
    connect (volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));

    drawPlaylist();
    drawAllPlaylists();

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));

    QShortcut * fileOpenShortcut = new QShortcut(QKeySequence(tr("Ctrl+O")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * folderOpenShortcut = new QShortcut(QKeySequence("Ctrl+Shift+O"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * liveSpectrumShortcut = new QShortcut(QKeySequence("Ctrl+Shift+L"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * backwardShortcut = new QShortcut(QKeySequence("Ctrl+Left"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * forwardShortcut = new QShortcut(QKeySequence("Ctrl+Right"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * volumeUpShortcut = new QShortcut(QKeySequence("Ctrl+Up"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * volumeDownShortcut = new QShortcut(QKeySequence("Ctrl+Down"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * pauseShortcut = new QShortcut(QKeySequence("Space"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * exitShortcut = new QShortcut(QKeySequence("Ctrl+Q"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * jumpShortcut = new QShortcut(QKeySequence("Ctrl+J"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * loopShortcut = new QShortcut(QKeySequence("Ctrl+L"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * forward3secShortcut = new QShortcut(QKeySequence("Right"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * backward3secShortcut = new QShortcut(QKeySequence("Left"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * removeFileShortcut = new QShortcut(QKeySequence("Delete"), playlistWidget, nullptr, nullptr, Qt::WidgetShortcut);
    QShortcut * removeMarkShortcut = new QShortcut(QKeySequence("Delete"), marksWin);
    QShortcut * removePlaylistShortcut = new QShortcut(QKeySequence("Ctrl+Delete"), this, nullptr, nullptr, Qt::ApplicationShortcut);
    QShortcut * playFileShortcut = new QShortcut(QKeySequence(Qt::Key_Return), playlistWidget, nullptr, nullptr, Qt::WidgetShortcut);
    QShortcut * marksShortcut = new QShortcut(QKeySequence("Ctrl+M"), this, nullptr, nullptr, Qt::ApplicationShortcut);

    connect (fileOpenShortcut, &QShortcut::activated, [=]() {
        openFile();
    });
    connect (folderOpenShortcut, &QShortcut::activated, [=]() {
        openFolder();
    });
    connect (liveSpectrumShortcut, &QShortcut::activated, [=]() {
        liveSpec = !liveSpec;
    });
    connect (backwardShortcut, &QShortcut::activated, [=]() {
        backward();
    });
    connect (forwardShortcut, &QShortcut::activated, [=]() {
        forward();
    });
    connect (volumeUpShortcut, &QShortcut::activated, [=]() {
        changeVolume(volume * 100.0f + 5);
    });
    connect (volumeDownShortcut, &QShortcut::activated, [=]() {
        changeVolume(volume * 100.0f - 5);
    });
    connect (pauseShortcut, &QShortcut::activated, [=]() {
        pause();
    });
    connect (exitShortcut, &QShortcut::activated, [=]() {
        this->close();
    });
    connect (jumpShortcut, &QShortcut::activated, [=]() {
        jumpTo();
    });
    connect (loopShortcut, SIGNAL(activated()), this, SLOT(makeLoop()));
    connect (forward3secShortcut, &QShortcut::activated, [=]() {
        double new_time = getPosition() + 3;
        BASS_ChannelPause(channel);
        setPosition(new_time);
    });
    connect (backward3secShortcut, &QShortcut::activated, [=]() {
        double new_time = getPosition() - 3;
        BASS_ChannelPause(channel);
        setPosition(new_time);
    });
    connect (removeFileShortcut, &QShortcut::activated, [=]() {
        removeFile();
    });
    connect (removePlaylistShortcut, &QShortcut::activated, [=]() {
        removePlaylist(getPlaylistIndexByName(currentPlaylistName), false);
    });
    connect (removeMarkShortcut, &QShortcut::activated, [=]() {
        removeMark();
    });
    connect (playFileShortcut, &QShortcut::activated, [=]() {
        if (playlistWidget->currentRow() != -1) {
            lastTrackID = currentID;
            lastPlaylistName = playingSongPlaylist;
            playingSongPlaylist = currentPlaylistName;
            setActive(playlistWidget->currentRow());
        }
    });
    connect (marksShortcut, &QShortcut::activated, this, &MainWindow::marksShow);

    QTimer::singleShot(3000, [=]() {
        this->show();
        this->raise();
        this->setFocus();

        if (configLoaded) {
            QTextStream in(&configFile);

            int index = in.readLine().toInt();
            QString path = in.readLine();
            QString playlistName = in.readLine();

            double position = in.readLine().toDouble();

            qDebug() << index << playlistName << position << path;

            if (playlistName == "" || playlists.find(playlistName) == playlists.end()) return;
            if (index > playlists[playlistName].size()) return;
            if (playlists[playlistName][index].path != path) return;

            lastPlaylistName = playingSongPlaylist;
            playingSongPlaylist = playlistName;
            lastTrackID = currentID;

            muted = true;
            setActive(index);
            pause();
            muted = false;
            BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
            playlistsBar->setCurrentIndex(getPlaylistIndexByName(playlistName));
            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, position), BASS_POS_BYTE);
        }
    });
    QTimer::singleShot(5000, startWidget, &StartWidget::close);

    this->installEventFilter(this);
    this->centralWidget()->installEventFilter(this);
    this->setAcceptDrops(true);
    this->setAttribute( Qt::WA_AcceptDrops, true);
    this->centralWidget()->setAcceptDrops(true);
    this->centralWidget()->setAttribute(Qt::WA_AcceptDrops, true);

    dropWidget = new QLabel("\uf574", this);
    dropWidget->setGeometry(0, 0, 800, 630);
    dropWidget->setFont(fontAwesome);
    dropWidget->installEventFilter(this);
    dropWidget->setAcceptDrops(true);
    dropWidget->setAlignment(Qt::AlignCenter);
    dropWidget->setStyleSheet("font-size: 60px; color: silver; background: rgba(16, 16, 16, 0.5);");
    dropWidget->hide();
}

MainWindow::~MainWindow()
{
    remove("cover.png"); // Remove Cover file
    removeDir("C:/amptemp");
    removeDir("C:/amptemp2");

    QString uptime = seconds2qstring((clock() - starttime) / 1000);
    writeLog("Exiting...");
    writeLog("Uptime: " + uptime + " (" + QString::number(clock() - starttime) + "ms)");

    logfile.close();

    if (searchSong->text() == "")
        playlists[currentPlaylistName] = playlist;

    XMLreader->writePlaylists(playlists);

    configFile.resize(0);
    QTextStream out(&configFile);
    out << currentID << "\n";
    if (currentID != -1)
        out << playlists[playingSongPlaylist][currentID].path;
    out <<  "\n";
    out << playingSongPlaylist << "\n";
    out << getPosition();

    delete ui;
}
// Program menu
void MainWindow::menuContext () {
    QMenu myMenu;
    myMenu.setStyleSheet("background-color: #121212; color: silver");

    QAction openFileAction;
    openFileAction.setText("Open File(s)");
    openFileAction.setIconVisibleInMenu(true);
    openFileAction.setShortcut(QKeySequence("Ctrl+O"));
    openFileAction.setShortcutVisibleInContextMenu(true);
    myMenu.addAction(&openFileAction);

    QAction openFolderAction;
    openFolderAction.setText("Open Folder");
    openFolderAction.setIconVisibleInMenu(true);
    openFolderAction.setShortcut(QKeySequence("Ctrl+Shift+O"));
    openFolderAction.setShortcutVisibleInContextMenu(true);
    myMenu.addAction(&openFolderAction);

    myMenu.addSeparator();
    myMenu.addAction("Settings");
    myMenu.addAction("About AMPlayer");
    myMenu.addSeparator();
    myMenu.addAction("Exit");

    QAction * selectedItem = myMenu.exec(this->mapToGlobal(QPoint(20, 20)));

    if (selectedItem)
    {
        if (selectedItem->text() == "Open File(s)") {
            openFile();
        }
        if (selectedItem->text() == "Open Folder") {
            openFolder();
        }
        if (selectedItem->text() == "Settings") {
            settings();
        }
        if (selectedItem->text() == "Exit") {
            slot_close();
        }
    }
}
void MainWindow::trayContext () {
    auto pos = QCursor::pos();

    QMenu myMenu;
    myMenu.setStyleSheet("padding-left: 10px; padding-top: 3px; icon-size: 12px; background-color: #121212; color: silver");

    QAction windowAction;
    windowAction.setIcon(QIcon(QPixmap(":/Images/window-maximize-regular.png")));
    windowAction.setText("Show window");
    windowAction.setObjectName("show-window");
    windowAction.setIconVisibleInMenu(true);
    myMenu.addAction(&windowAction);

    myMenu.addSeparator();

    QAction playAction;
    playAction.setIcon(paused ? QIcon(QPixmap(":/Images/pause-solid.png")) : QIcon(QPixmap(":/Images/play-solid.png")));
    playAction.setText(paused ? "Pause" : "Play");
    playAction.setObjectName("play");
    playAction.setIconVisibleInMenu(true);
    myMenu.addAction(&playAction);

    QAction backwardAction;
    backwardAction.setIcon(QIcon(QPixmap(":/Images/backward-solid.png")));
    backwardAction.setText("Backward");
    backwardAction.setObjectName("backward");
    backwardAction.setIconVisibleInMenu(true);
    myMenu.addAction(&backwardAction);

    QAction forwardAction;
    forwardAction.setIcon(QIcon(QPixmap(":/Images/forward-solid.png")));
    forwardAction.setText("Forward");
    forwardAction.setObjectName("forward");
    forwardAction.setIconVisibleInMenu(true);
    myMenu.addAction(&forwardAction);

    myMenu.addSeparator();

    QAction marksAction;
    marksAction.setText("Marks");
    marksAction.setIcon(QIcon(QPixmap(":/Images/bookmark-regular.png")));
    marksAction.setIconVisibleInMenu(true);
    myMenu.addAction(&marksAction);

    QAction repeatAction;
    repeatAction.setText("Repeat");
    repeatAction.setIcon(QIcon(QPixmap(":/Images/repeat.png")));
    repeatAction.setCheckable(true);
    repeatAction.setChecked(repeat);
    repeatAction.setIconVisibleInMenu(true);
    myMenu.addAction(&repeatAction);

    QAction shuffleAction;
    shuffleAction.setText("Shuffle");
    shuffleAction.setIcon(QIcon(QPixmap(":/Images/random-solid.png")));
    shuffleAction.setCheckable(true);
    shuffleAction.setChecked(shuffle);
    shuffleAction.setIconVisibleInMenu(true);
    myMenu.addAction(&shuffleAction);

    myMenu.addSeparator();

    QAction equoAction;
    equoAction.setText("Equalizer");
    equoAction.setIcon(QIcon(QPixmap(":/Images/sliders-h-solid.png")));
    equoAction.setCheckable(true);
    equoAction.setIconVisibleInMenu(true);
    myMenu.addAction(&equoAction);

    QAction visualAction;
    visualAction.setText("Visualization");
    visualAction.setIcon(QIcon(QPixmap(":/Images/tv-solid.png")));
    visualAction.setCheckable(true);
    visualAction.setIconVisibleInMenu(true);
    myMenu.addAction(&visualAction);

    QAction remoteAction;
    remoteAction.setText("Remote control");
    remoteAction.setIcon(QIcon(QPixmap(":/Images/mobile-solid.png")));
    remoteAction.setCheckable(true);
    remoteAction.setIconVisibleInMenu(true);
    myMenu.addAction(&remoteAction);

    myMenu.addSeparator();

    QAction exitAction;
    exitAction.setText("Exit");
    exitAction.setIcon(QIcon(":/Images/times-solid.png"));
    exitAction.setCheckable(true);
    exitAction.setIconVisibleInMenu(true);
    myMenu.addAction(&exitAction);

    QAction * selectedItem = myMenu.exec(pos);

    if (selectedItem)
    {
        if (selectedItem->text() == "Show window") {
            showWindow();
        }
        if (selectedItem->text() == "Play" || selectedItem->text() == "Pause") {
            pause();
        }
        if (selectedItem->text() == "Backward") {
            backward();
        }
        if (selectedItem->text() == "Forward") {
            forward();
        }
        if (selectedItem->text() == "Marks")
            marksWin->show();
        if (selectedItem->text() == "Repeat")
            changeRepeat();
        if (selectedItem->text() == "Shuffle")
            changeShuffle();
        if (selectedItem->text() == "Equalizer") {
            equalizer();
        }
        if (selectedItem->text() == "Visualization") {
            visualizations();
        }
        if (selectedItem->text() == "Remote control") {
            remoteControl();
        }
        if (selectedItem->text() == "Exit")
            this->close();
    }
}
// Open File
bool MainWindow::openFile ()
{
    searchSong->setText("");

    QFileDialog dialog(this);
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);

    QString formats = "*.tta *.spx *.ape *.mov *.ac3 *.asf *.webm *.wma *.adt *.adts *.wm *.wmv *.3g2 *.3gp *.flac *.aac *.mkv *.mka *.m4a *.m4b *.m4r *.mpa *.mpc *.mpeg *.m4p *.mp4 *.mp3 *.mp2 *.mp1 *.opus *.oga *.ogg *.aif *.aiff *.aifc *.wav *.avi";
    QStringList formatsList = formats.split(" ");
    formatsList.sort();
    QString sortedFormats = formatsList.join(" ");

    dialog.setNameFilter("Audio files (" + sortedFormats + ")");

    QStringList fileNames;

    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    for (int i = 0; i < fileNames.length(); i++)
    {
        Song temp(fileNames[i]);
        temp.countDuration();

        TagLib::FileRef f(fileNames[i].toStdWString().c_str());

        wstring artist = L"", title = L"";

        // If file not load failed
        if (!f.isNull()) {
            artist = f.tag()->artist().toCWString();
            title = f.tag()->title().toCWString();
        }

        writeLog("Opened file: " + fileNames[i]);

        if (count(playlist.begin(), playlist.end(), temp) != 0)
        {
            continue;
        }

        if (artist == L"" && title.find('-') == wstring::npos) {
            artist = L"Unknown Artist";
        } else if (artist == L"" && title.find('-') != wstring::npos) {
            artist = L"";
        }

        if (title == L"") {
            temp.setNameFromPath();
        } else {
            if (artist != L"")
                temp.setName(QString::fromStdWString(artist) + " - " + QString::fromStdWString(title));
            else
                temp.setName(QString::fromStdWString(title));
        }
        playlist.push_back(temp);
    }

    playlists[currentPlaylistName] = playlist;

    drawPlaylist();
    if (playlist.size() == 1)
    {
        lastPlaylistName = playingSongPlaylist;
        playingSongPlaylist = currentPlaylistName;
        setActive(0);
    }
    return true;
}
void MainWindow::loadLyrics(Song * song) {
    QString lrcFile = QFileDialog::getOpenFileName(lyricsWin, "Select lyrics file", QDir::homePath(), "Lyrics file (*.lrc)");
    song->setLrcFile(lrcFile);
    lyricsWin->lyrics = song->lyrics;
    lyricsWin->countCurrentLine();
}
void MainWindow::reorderPlaylist () {
    vector <Song> neworder;

    for (int i = 0; i < playlist.size(); i++)
    {
        int index = playlistWidget->item(i)->data(Qt::UserRole).toInt();
        neworder.push_back (playlist[index]);
    }

    if (channel != NULL)
    {
        QString currentPath = playlist[currentID].path;
        Song temp(currentPath);
        currentID = std::find(neworder.begin(), neworder.end(), temp) - neworder.begin();
    }

    playlist = neworder;
    playlists[currentPlaylistName] = playlist;

    drawPlaylist();
}
void MainWindow::rowsMoved(QModelIndex, int, int, QModelIndex, int) {
    reorderPlaylist();
}

void MainWindow::removeFile() {
    if (playlistWidget->currentRow() == -1) return;

    int index = playlistWidget->currentItem()->data(Qt::UserRole).toInt();

    writeLog(playlistWidget->currentItem()->text() + " removed from " + currentPlaylistName);

    Song temp(playlist[index].path);
    int globalIndex = std::find(playlists[currentPlaylistName].begin(), playlists[currentPlaylistName].end(), temp) - playlists[currentPlaylistName].begin();

    if (globalIndex == currentID)
    {
        BASS_StreamFree(channel);
        channel = NULL;
        marksList->clear();
        marksWin->close();
        songTitle->setText("");
        songInfo->setText("");
        marksBtn->setDisabled(true);
        pauseBtn->setText("\uf04b");
        songPosition->setText("00:00");
        songDuration->setText("00:00");
        coverLoaded = false;
        cover = QImage (":/Images/cover-placeholder.png");
        currentID = -1;
        playingSongPlaylist = "";
        clearPrerenderedFft();
        lyricsWin->lyrics = vector<pair<float, QString>>();
    }
    else if (globalIndex < currentID)
        currentID--;

    playlist.erase(playlist.begin() + index);
    playlists[currentPlaylistName].erase(playlists[currentPlaylistName].begin() + globalIndex);
    drawPlaylist();

    if (index == (int)playlist.size()) index--;
    playlistWidget->setCurrentRow(index);
}

bool MainWindow::openFolder () {
    searchSong->setText("");

    QString folder = QFileDialog::getExistingDirectory(0, ("Select Folder with Songs"), QDir::homePath());

    if (folder == "")
        return false;

    QDir directory(folder);
    QString formats = "*.tta *.spx *.ape *.mov *.ac3 *.asf *.webm *.wma *.adt *.adts *.wm *.wmv *.3g2 *.3gp *.flac *.aac *.mkv *.mka *.m4a *.m4b *.m4r *.mpa *.mpc *.mpeg *.m4p *.mp4 *.mp3 *.mp2 *.mp1 *.opus *.oga *.ogg *.aif *.aiff *.aifc *.wav *.avi";
    auto formatsList = formats.split(" ");
    formatsList.sort();
    QStringList musicFiles = directory.entryList(formatsList, QDir::Files | QDir::Readable);

    writeLog("Folder opened: " + folder);
    writeLog("Folder contains: " + QString::number (musicFiles.length()) + " files");

    int duplicatesCount = 0;
    for (int i = 0; i < musicFiles.length(); i++)
    {
        QString fullpath = folder + "/" + musicFiles[i];

        Song temp(fullpath);
        temp.countDuration();

        TagLib::FileRef f(fullpath.toStdWString().c_str());

        wstring artist = L"", title = L"";

        // If file not load failed
        if (!f.isNull()) {
            artist = f.tag()->artist().toCWString();
            title = f.tag()->title().toCWString();
        }

        writeLog("Opened file: " + musicFiles[i]);

        if (artist == L"" && title.find('-') == wstring::npos) {
            artist = L"Unknown Artist";
        } else if (artist == L"" && title.find('-') != wstring::npos) {
            artist = L"";
        }

        if (title == L"") {
            temp.setNameFromPath();
        } else {
            if (artist != L"")
                temp.setName(QString::fromStdWString(artist) + " - " + QString::fromStdWString(title));
            else
                temp.setName(QString::fromStdWString(title));
        }

        if (count(playlist.begin(), playlist.end(), temp) != 0)
        {
            duplicatesCount++;
            continue;
        }

        playlist.push_back(temp);
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Files adding");

    if (musicFiles.length() > 0 && duplicatesCount != musicFiles.length()) {
        msgBox.setText(tr(to_string(musicFiles.length() - duplicatesCount).c_str()) + " new files added to playlist successfully!");
    }
    else if (duplicatesCount == musicFiles.length() && musicFiles.length() > 0) {
        msgBox.setText ("0 new files added!");
    }
    else if (musicFiles.length() == 0) {
        msgBox.setText("There are no audio files in this folder!");
    }

    msgBox.setStyleSheet("background-color: #101010; color: silver;");
    msgBox.exec();

    drawPlaylist();
    playlists[currentPlaylistName] = playlist;

    return true;
}
void MainWindow::createPlaylist ()
{
    QString name = "";
    QString tempPlaylistName = currentPlaylistName;

    do {
        bool ok;
        name = QInputDialog::getText(this, tr("Playlist Name"),  tr("Playlist name:"), QLineEdit::Normal, NULL, &ok);
        name = name.simplified();

        if (ok && !name.isEmpty())
        {
            if (playlists.find(name) != playlists.end()) {
                QMessageBox msgBox;
                msgBox.setText("Playlists \'" + name + "\' already exists!");
                msgBox.setStyleSheet("background-color: #101010; color: silver;");
                msgBox.exec();
            } else {
                writeLog("New playlist created: " + name);
                playlists[name] = vector <Song>();
                break;
            }
        } else {
            break;
        }
    } while (true);

    if (name == "") {
        playlistsBar->setCurrentIndex(getPlaylistIndexByName(tempPlaylistName));
        qDebug() << currentPlaylistName;
    } else
        drawAllPlaylists();
}
void MainWindow::renamePlaylist(int index) {
    QString newName = "";
    QString oldName = playlistsBar->tabText(index);
    QString currentName = currentPlaylistName;

    do {
        bool ok;
        newName = QInputDialog::getText(this, tr("Playlist Name"),  tr("Playlist name:"), QLineEdit::Normal, oldName, &ok);
        newName = newName.simplified();

        if (ok && !newName.isEmpty())
        {
            if (playlists.find(newName) != playlists.end()) {
                QMessageBox msgBox;
                msgBox.setText("Playlists \'" + newName + "\' already exists!");
                msgBox.setStyleSheet("background-color: #101010; color: silver;");
                msgBox.exec();
            } else {
                renamingPlaylist = true;
                fifo_map <QString, vector <Song>> tempPlaylists;

                for (auto & playlist : playlists) {
                    if (playlist.first == oldName)
                        tempPlaylists[newName] = playlist.second;
                    else
                        tempPlaylists[playlist.first] = playlist.second;
                }

                playlists.clear();
                playlists.insert(tempPlaylists.begin(), tempPlaylists.end());

                if (oldName == playingSongPlaylist) playingSongPlaylist = newName;
                if (oldName == currentPlaylistName) currentPlaylistName = newName;

                break;
            }
        } else {
            break;
        }
    } while (true);

    if (newName == "") {
        playlistsBar->setCurrentIndex(getPlaylistIndexByName(oldName));
        qDebug() << currentPlaylistName;
        renamingPlaylist = false;
        return;
    } else
        drawAllPlaylists();

    if (oldName == currentName)
        playlistsBar->setCurrentIndex(getPlaylistIndexByName(newName));
    else
        playlistsBar->setCurrentIndex(getPlaylistIndexByName(currentName));

    renamingPlaylist = false;
}
void MainWindow::removePlaylist (int index, bool clear) {
    QString playlistName = playlistsBar->tabText(index);

    if (playlists[playlistName].size() > 0) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText("Are you sure to " + tr((clear) ? "clear" : "delete") + " \'" + playlistName + "\'playlist?");
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.setInformativeText("Playlist contains " + QString::fromStdString(to_string(playlists[playlistName].size())) + " songs.");
        int ret = msgBox.exec();

        if (ret == QMessageBox::No)
            return;
    }

    if (channel != NULL) {
        bool currentSongInPlaylist = false;

        for (auto iter = playlist.begin(); iter != playlist.end(); iter++)
            if (iter->path == playlist[currentID].path && playingSongPlaylist == playlistName)
                currentSongInPlaylist = true;

        if (currentSongInPlaylist)
        {
            BASS_StreamFree(channel);
            channel = NULL;
            marksList->clear();
            marksWin->close();
            pauseBtn->setText("\uf04b");
            songTitle->setText("");
            songInfo->setText("");
            songPosition->setText("00:00");
            songDuration->setText("00:00");
            marksBtn->setDisabled(true);
            coverLoaded = false;
            cover = QImage (":/Images/cover-placeholder.png");
            currentID = -1;
            playingSongPlaylist = "";
            clearPrerenderedFft();
            lyricsWin->lyrics = vector<pair<float, QString>>();
        }
    }

    // Deleting playlist and setting first playlist as current
    writeLog("Removed playlist: " + playlistName);

    if (playlistName == "Default") playlists["Default"] = vector<Song>();
    else {
        if (!clear) {
            playlists.erase(playlistName);

            if (playingSongPlaylist == "")
                currentPlaylistName = "Default";
            else
                currentPlaylistName = playingSongPlaylist;
        } else {
            playlists[playlistName] = vector<Song>();
        }
    }

    playlist = playlists[currentPlaylistName];

    int playlistIndex = getPlaylistIndexByName(currentPlaylistName);

    drawPlaylist();
    drawAllPlaylists();
    playlistsBar->setCurrentIndex(playlistIndex);
}
void MainWindow::search(const QString & text)
{
    qDebug() << text;
    writeLog("Searching query: " + text);

    if (text.length() == 1 && currentPlaylistName == playingSongPlaylist)
        playlists[playingSongPlaylist][currentID] = playlist[currentID];

    searchInPlaylist(text);
    drawPlaylist();
}
void MainWindow::drawAllPlaylists ()
{
    int i = 0;
    while (playlistsBar->count() != 0)
    {
        playlistsBar->removeTab(i);
        i++;

        if (i >= playlistsBar->count())
            i = 0;
    }

    i = 0;
    for (auto &playlist : playlists)
    {
        float totalDur = 0;
        for (int j = 0; j < playlist.second.size(); j++)
            totalDur += playlist.second[j].getDuration();

        playlistsBar->addTab(playlist.first);
        playlistsBar->setTabToolTip(i, "Playlist: " + playlist.first +
                                     "\nContains: " + QString::number(playlist.second.size()) + " songs" +
                                     "\nTotal duration: " + seconds2qstring(totalDur));
        i++;
    }

    playlistsBar->addTab("\uf067");
}
void MainWindow::searchInPlaylist(const QString & text) {
    if (text != "")
        playlistWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    else
        playlistWidget->setDragDropMode(QAbstractItemView::DragDrop);

    playlist = playlists[currentPlaylistName];

    for (int i = 0; i < (int)playlist.size(); i++) {
        QString name = playlist[i].getName();
        name = name.toLower();

        QString query = text.toLower();

        if (name.indexOf(query) == -1) {
            playlist.erase(playlist.begin() + i);
            i--;
        }
    }
}
void MainWindow::drawPlaylist() {
    playlistWidget->clear();

    for (int i = 0; i < (int)playlist.size(); i++)
    {
        QListWidgetItem * songItem = new QListWidgetItem(playlistWidget);
        QString name = playlist[i].getName();

        songItem->setData(Qt::UserRole, i);
        songItem->setText("[" + seconds2qstring(playlist[i].getDuration()) + "] " + name);

        songItem->setToolTip("Name: " + name +
                             "\nDuration: " + seconds2qstring(playlist[i].getDuration()) +
                             "\nFormat: " + playlist[i].getFormat() +
                             "\nSize: " + playlist[i].getFileSizeMB());

        playlistWidget->addItem(songItem);

        if (currentID != -1 && playlists[playingSongPlaylist][currentID] == playlist[i] && currentPlaylistName == playingSongPlaylist)
            playlistWidget->setCurrentRow(i);
   }
}
void MainWindow::setTitle () {
    QString name = playlists[playingSongPlaylist][currentID].getName();

    sendMessageToRemoteDevices("name|" + name);

    if (name.length() > 32)
        name = name.mid(0, 32) + "...";

    songTitle->setText(name);
}
void MainWindow::changeCurrentPlaylist (int index) {
    QString tabContent = playlistsBar->tabText(index);

    if (tabContent == "") {
        playlistsBar->removeTab(index);
        return;
    }

    if (tabContent == "\uF067")
    {
        if (playlistCreating) {
            playlistsBar->setCurrentIndex(getPlaylistIndexByName(currentPlaylistName));
            playlistCreating = false;
            return;
        }
        index--;
        playlistsBar->setCurrentIndex(index);
        return;
    }

    if (playlists.find(tabContent) == playlists.end() && tabContent != "\uF067")
        return;

    searchSong->setText("");
    playlists[currentPlaylistName] = playlist;

    writeLog("\nPlaylist changed to: " + playlistsBar->tabText(index));

    playlist = playlists[playlistsBar->tabText(index)];
    currentPlaylistName = playlistsBar->tabText(index);

    float start = clock();

    drawPlaylist();

    qDebug() << clock() - start;
}

void MainWindow::playlistsBarContextMenu (const QPoint& point) {
    int tabIndex = playlistsBar->tabAt(point);
    qDebug() << tabIndex;

    QPoint globalPos = playlistsBar->mapToGlobal(point);

    QMenu myMenu;
    myMenu.setStyleSheet("QMenu { background-color: #101010; color: silver; }"
                         "QMenu::item:disabled { color: gray; }");
    myMenu.addAction("Create Playlist");

    QAction renameAction;
    renameAction.setText("Rename");
    renameAction.setDisabled(playlistsBar->tabText(tabIndex) == "Default" ? true : false);
    myMenu.addAction(&renameAction);

    myMenu.addAction("Clear");

    QAction removeAction;
    removeAction.setText("Remove");
    removeAction.setDisabled(playlistsBar->tabText(tabIndex) == "Default" ? true : false);
    myMenu.addAction(&removeAction);

    QAction* selectedItem = myMenu.exec(globalPos);

    if (selectedItem)
    {
        if (selectedItem->text() == "Create Playlist") {
            createPlaylist();
        }
        if (selectedItem->text() == "Rename")
            renamePlaylist(tabIndex);
        if (selectedItem->text() == "Remove") {
            removePlaylist(tabIndex, false);
        }
        if (selectedItem->text() == "Clear")
            removePlaylist(tabIndex, true);
    }
}
void MainWindow::removeDir(QString dir)
{
    QDir tempdir(dir);

    //First delete any files in the current directory
    QFileInfoList files = tempdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    for(int file = 0; file < files.count(); file++)
    {
        tempdir.remove(files.at(file).fileName());
    }

    //Now recursively delete any child directories
    QFileInfoList dirs = tempdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    for(int dir = 0; dir < dirs.count(); dir++)
    {
        this->removeDir(dirs.at(dir).absoluteFilePath());
    }

    //Finally, remove empty parent directory
    tempdir.rmdir(tempdir.path());
}

void MainWindow::setActive(QListWidgetItem * item) {
    lastTrackID = currentID;
    lastPlaylistName = playingSongPlaylist;
    playingSongPlaylist = currentPlaylistName;
    setActive (playlistWidget->currentRow());
}
void MainWindow::setActive(int index) {
    removeDir("C:/amptemp");
    removeLoop();

    marksBtn->setDisabled(false);

    bool isSameTrack = false;

    paused = true;
    pauseBtn->setText("\uf04c"); // Set symbol to pause

    if (playingSongPlaylist == lastPlaylistName && index == lastTrackID)
        isSameTrack = true;

    Song temp;

    if (searchSong->text() == "" || playingSongPlaylist != currentPlaylistName)
    {
        temp = playlists[playingSongPlaylist][index];
    }
    else if (searchSong->text() != "") {
        temp = playlist[index];
    }

    lyricsWin->lyrics = temp.lyrics;
    lyricsWin->resetLineCounter();

    if (!QFile::exists(temp.path))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("File not found!");
        msgBox.setText("File \"" + temp.path + "\" deleted or moved!");
        msgBox.setStyleSheet("background-color: #101010; color: silver;");
        msgBox.exec();

        playlist.erase(playlist.begin() + index);
        int globalIndex = std::find(playlists[playingSongPlaylist].begin(), playlists[playingSongPlaylist].end(), temp) - playlists[playingSongPlaylist].begin();
        playlists[currentPlaylistName].erase(playlists[currentPlaylistName].begin() + globalIndex);
        drawPlaylist();

        if (globalIndex < currentID)
            currentID--;

        playlistWidget->setCurrentRow(currentID);
        searchSong->setText("");

        return;
    }

    currentID = std::find(playlists[playingSongPlaylist].begin(), playlists[playingSongPlaylist].end(), temp) - playlists[playingSongPlaylist].begin();
    temp = playlists[playingSongPlaylist][currentID];
    cover = temp.getCover();

    TagLib::FileRef file(temp.path.toStdWString().c_str());

    if (!file.isNull()) {
        QString bitrate = QString::number(file.audioProperties()->bitrate()) + " kbps";
        QString sampleRate = QString::number(file.audioProperties()->sampleRate() / 1000.0f) + " khz";

        songInfo->setText("Genre, " + bitrate + " " + sampleRate + ", " + temp.getFormat());
    } else {
        songInfo->setText("Genre, " + temp.getFormat());
    }

    coverLoaded = true;
    if (cover.isNull()) // Audio file don't contain cover, load custom cover
    {
        coverLoaded = false;
        cover = QImage (":/Images/cover-placeholder.png");
    }

    BASS_StreamFree(channel);

    if (equoEnabled) {
        createStream(channel, temp.path, BASS_STREAM_DECODE);
        equalizerWin->channel = &channel;
        equalizerWin->init();

    } else {
        createStream(channel, temp.path, 0);
    }

    if (!isSameTrack)
    {
        clearPrerenderedFft();

        auto func = std::bind(&MainWindow::prerenderFft, this, temp.path);

        std::thread thr(func);
        thr.detach();

        auto coverFadeIn = std::bind(&MainWindow::coverBackgroundPopup, this);

        std::thread thr2 (coverFadeIn);
        thr2.detach();
    }

    BASS_ChannelPlay(channel, false);

    if (muted) BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0);
    else BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

    QString pos = seconds2qstring(getPosition());
    songPosition->setText(pos);

    QString len = seconds2qstring(getDuration());
    songDuration->setText(len);

    taskbarProgress->setMinimum(0);
    taskbarProgress->setMaximum(getDuration());
    taskbarProgress->setValue(0);
    taskbarProgress->show();

    searchSong->setText("");

    if (currentPlaylistName == playingSongPlaylist)
    {
        playlistWidget->setCurrentRow(currentID);
        writeLog("Song changed to: " + playlistWidget->currentItem()->text());
    }

    setTitle();
    reloadStyles();

    infoWidget->reset();
    infoWidget->setName(temp.getName());
    infoWidget->setInfo(songInfo->text());
    infoWidget->setDuration(len);
    infoWidget->popup(3000);

    drawMarksList();

    trayIcon->setToolTip("Now playing: " + temp.getName());
    this->setWindowTitle(temp.getName());

    temp.countDuration();
}
void MainWindow::forward () {
    if (channel == NULL || playlists[playingSongPlaylist].size() == 1)
        return;

    writeLog("Forward button pressed");

    lastTrackID = currentID;

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlists[playingSongPlaylist].size();
        } while (songID == currentID);

        currentID = songID;
    }
    else currentID++;

    if (currentID >= playlists[playingSongPlaylist].size()) currentID = 0;

    setActive(currentID);
}
void MainWindow::backward () {
    if (channel == NULL || playlists[playingSongPlaylist].size() == 1)
        return;

    writeLog("Backward button pressed");

    lastTrackID = currentID;

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlists[playingSongPlaylist].size();
        } while (songID == currentID);

        currentID = songID;
    }
    else currentID--;

    if (currentID < 0) currentID = playlists[playingSongPlaylist].size() - 1;

    setActive(currentID);
}
void MainWindow::pause()
{
    if (playlistWidget->currentRow() != -1 && channel == NULL) {
        lastTrackID = currentID;
        lastPlaylistName = playingSongPlaylist;
        playingSongPlaylist = currentPlaylistName;
        setActive(playlistWidget->currentRow());
        return;
    }

    if (playlistWidget->currentRow() == -1 && channel == NULL)
        return;

    paused = !paused;

    if (paused) {
        writeLog("Song resumed");
        BASS_ChannelPlay(channel, false);
        pauseBtn->setText("\uf04c"); // Set symbol to pause

        auto func = std::bind(&MainWindow::coverBackgroundPopup, this);

        std::thread thr(func);
        thr.detach();
    }
    else {
        writeLog("Song paused");
        BASS_ChannelPause(channel);
        pauseBtn->setText("\uf04b"); // Set symbol to play

        auto func = std::bind(&MainWindow::coverBackgroundHide, this);

        std::thread thr(func);
        thr.detach();
    }

    sendMessageToRemoteDevices("pause|" + QString::number(paused));
}
void MainWindow::changeVolume (int vol)
{
    if (muted)
    {
        volumeBtn->setText("\uf028");
        muted = false;
    }
    volumeSlider->setValue(vol);
    volume = vol / 100.0f;

    if (volume > 1.0) volume = 1.0;
    if (volume < 0.0) volume = 0;

    writeLog("Volume changed to: " + QString::number(vol));
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
    sendMessageToRemoteDevices("vol|" + QString::number(volume * 100));
}
void MainWindow::changeRepeat () {
    if (shuffle == true)
        changeShuffle();

    repeat = !repeat;

    if (repeat) {
        writeLog("Repeat enabled");
        repeatBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    }
    else {
        writeLog("Repeat disabled");
        repeatBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    }
}
void MainWindow::changeShuffle () {
    if (repeat == true)
        changeRepeat();

    shuffle = !shuffle;

    if (shuffle) {
        writeLog("Shuffle enabled");
        shuffleBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    }
    else {
        writeLog("Shuffle disabled");
        shuffleBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
    }
}
// Seconds (float) to QString
QString MainWindow::seconds2qstring (float seconds) {
    int hours = (seconds >= 3600) ? (int)seconds / 3600 : 0;
    int minutes = (seconds >= 60) ? ((int)seconds - hours * 3600) / 60 : 0;
    int secs = (int)seconds % 60;

    QString result = "";
    if (hours != 0)
    {
        if (hours < 10) result += "0";
        result += QString::number (hours);
        result += ":";
    }

    if (minutes < 10) result += "0";
    result += QString::number (minutes);
    result += ":";

    if (secs < 10) result += "0";
    result += QString::number (secs);

    return result;
}
// Convert QString to Seconds (double)
double MainWindow::qstring2seconds (QString time) {
    time = time.simplified();

    if (time.count(':') == 1 && time[0] != '-') {
        int minutes = time.mid(0, time.indexOf(':')).toInt();
        int seconds = time.mid(time.indexOf(':') + 1).toInt();

        return minutes * 60.0 + seconds;
    }
    else if (time.count(':') == 2 && time[0] != '-') {
        int hours = time.mid(0, time.indexOf(':')).toInt();
        int minutes = time.mid(time.indexOf(':') + 1, time.lastIndexOf(':')).toInt();
        int seconds = time.mid(time.lastIndexOf(':') + 1).toInt();

        return hours * 3600 + minutes * 60.0 + seconds;
    }
    else if (time[0] == '-') {
        return getDuration() - qstring2seconds(time.mid(1));
    }
    else if (time[time.length() - 2] == "m" && time[time.length() - 1] == "s") {
        return time.mid(0, time.length() - 2).toDouble() / 1000;
    }
    else if (time[time.length() - 1] == "s") {
        return time.mid(0, time.length() - 1).toDouble();
    }
    else if (time[time.length() - 1] == "m") {
        return time.mid(0, time.length() - 1).toDouble() * 60;
    }
    else if (time[time.length() - 1] == "h") {
        return time.mid(0, time.length() - 1).toDouble() * 3600;
    }
    // Check if mark name used
    else if (time[0] == '"' && time[time.length() - 1] == '"') {
        QString markTag = time.mid(1, time.length() - 2);

        qDebug() << markTag;

        for (auto &mark : playlists[playingSongPlaylist][currentID].marks) {
            if (mark.second == markTag)
                return mark.first;
        }

        return -1;
    }
}
void MainWindow::makeLoop() {
    QDialog dialog(this);
    dialog.setStyleSheet("background-color: #101010; color: silver;");

    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    QLineEdit * loopStartInput = new QLineEdit(&dialog);
    loopStartInput->setText("00:00");
    loopStartInput->setFocus();
    form.addRow("Loop start (A): ", loopStartInput);

    QLineEdit * loopEndInput = new QLineEdit(&dialog);
    loopEndInput->setText("00:00");
    form.addRow("Loop end (B): ", loopEndInput);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        int start = qstring2seconds(loopStartInput->text());
        int end = qstring2seconds(loopEndInput->text());

        if (start == 0 && end >= (int)getDuration())
            removeLoop();
        else if (end - start == 0)
            removeLoop();
        else {
            looped = true;
            loopStart = start;
            loopEnd = end;
            if (getPosition() < start || getPosition() > end)
                setPosition(start);
        }
    } else {
        removeLoop();
    }
}
void MainWindow::updateTime() {
    static float songPos = getPosition();   // Static variable of song position
    static int counter = 0;                 // Passed frames counter

    sendMessageToRemoteDevices("pos|" + QString::number(songPos/getDuration()));

    if (paused) {
        taskbarProgress->resume();
        taskbarProgress->setValue(songPos);
    }
    else {
        taskbarProgress->setValue(songPos);
        taskbarProgress->stop();
    }
    if (isOrderChanged)
    {
        reorderPlaylist();
        isOrderChanged = false;
    }

    if (channel != NULL) {
        // If song stay in same position, increase counter by 1
        if (songPos == getPosition())
            counter++;

        // if counter value equals 5 (5 frames (~80ms) passed) and position still same, then
        // the scrolling stopped, so continue playing (if track not paused) and reset counter
        if (counter >= 5 && songPos == getPosition() && paused) {
            BASS_ChannelPlay(channel, false);
            counter = 0;
        }
        else
            songPos = getPosition();

        QString pos = seconds2qstring(getPosition());
        songPosition->setText(pos);

        if (looped && getPosition() > loopEnd) {
            setPosition(loopStart);
        }

        if (!looped && round(getPosition()) >= round(getDuration()))
        {
            writeLog("Song ended");

            if (timerStarted && songTimerMode == 1)
                songTimerCounter--;

            if (songTimerCounter == 0)
                songTimerEnded();

            if (repeat) {
                lastTrackID = currentID;
                lastPlaylistName = playingSongPlaylist;
                setActive(currentID);
            }
            else if (shuffle && playlist.size() > 1) {
                lastTrackID = currentID;

                int songID;
                do {
                    songID = rand() % playlists[playingSongPlaylist].size();
                } while (songID == currentID);

                currentID = songID;
                setActive(currentID);
            } else {
                writeLog("Changing to next song");
                forward();
            }
        }
    }

    colorChange();
    if (!this->isHidden())
        repaint();
}

void MainWindow::paintEvent(QPaintEvent * event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::transparent, 0));

    if (!cover.isNull() && coverLoaded) {
        QGraphicsBlurEffect * blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(3);

        QImage cover2 = cover.scaled(250, 250, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // Bluring background
        if (coverBgBlur)
            cover2 = applyEffectToImage(cover2, blur);

        QBrush brush(cover2);
        QTransform transform;

        transform.translate(25, -220);
        brush.setTransform(transform);
        painter.setBrush(brush);
        painter.drawRoundedRect(275, 30, 250, 250, 10, 10);

        QRadialGradient gradient(400, 155, 200);
        gradient.setColorAt(0, QColor(16, 16, 16, coverBgOpacity));
        gradient.setColorAt(1, QColor(16, 16, 16, 255));

        QBrush brush2(gradient);
        painter.setBrush(brush2);

        // painter.setBrush(QBrush(QColor(16, 16, 16, coverBgOpacity)));
        painter.drawRect(275, 30, 250, 250);
    }

    if (!cover.isNull()) {
        cover = cover.scaled(150, 150, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        QBrush brush(cover);
        QTransform transform;

        transform.translate(175, -90);
        brush.setTransform(transform);
        painter.setBrush(brush);
        painter.drawRoundedRect(325, 60, 150, 150, 5, 5);
    }

    QColor color = mainColor;

    vector <int> marksPos;
    if (currentID != -1 && !renamingPlaylist) {
        for (auto &mark : playlists[playingSongPlaylist][currentID].marks) {
            marksPos.push_back((mark.first / getDuration() * 140) + 1);
        }
    }

    if (liveSpec) {
        float fft[1024];
        BASS_ChannelGetData(channel, fft, BASS_DATA_FFT2048);

        for (int i = 0; i < 140; i++) {
            int h = sqrt(fft[i + 1]) * 3 * 40 - 4;
            if (h < 3) h = 3;
            if (h > 40) h = 40;

            if (channel == NULL) h = 3;

            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(QRectF(50 + 5 * i, 350 + 10 + (20 - h) / 2, 3, h), 2, 2);
            QPen pen(Qt::transparent, 0);
            p.setPen(pen);

            if (std::count(marksPos.begin(), marksPos.end(), i) != 0) {
                p.fillPath(path, QColor (255, 255, 255));
            } else {
                if (looped && (i * (getDuration() / 140) < loopStart || i * (getDuration() / 140) > loopEnd))
                    p.fillPath(path, QColor (128, 128, 128));
                else if ((i - 1) * (getDuration() / 140) <= getPosition())
                    p.fillPath(path, color);
                else
                    p.fillPath(path, QColor (192, 192, 192));
            }

            p.drawPath(path);
        }
    }
    else {
        for (int i = 0; i < 140; i++) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(QRectF(50 + 5 * i, 350 + 10 + (20 - prerenderedFft[i]) / 2, 3, prerenderedFft[i]), 2, 2);
            QPen pen(Qt::transparent, 0);
            p.setPen(pen);

            if (std::count(marksPos.begin(), marksPos.end(), i) != 0) {
                p.fillPath(path, QColor (255, 255, 255));
            } else {
                if (looped && (i * (getDuration() / 140) < loopStart || i * (getDuration() / 140) > loopEnd))
                    p.fillPath(path, QColor (128, 128, 128));
                else if ((i - 1) * (getDuration() / 140) <= getPosition())
                    p.fillPath(path, color);
                else
                    p.fillPath(path, QColor (192, 192, 192));
            }

            p.drawPath(path);
        }
    }
}

void MainWindow::prerenderFft(QString file)
{   
    HSTREAM tempStream;
    createStream(tempStream, file, BASS_STREAM_PRESCAN | BASS_ASYNCFILE);

    writeLog("Prerendering fft...");

    QWORD len = BASS_ChannelGetLength(tempStream, BASS_POS_BYTE); // the length in bytes
    float time = BASS_ChannelBytes2Seconds(tempStream, len);      // the length in seconds

    int avgLen = 1024;

    BASS_ChannelSetAttribute(tempStream, BASS_ATTRIB_VOL, 0);
    BASS_ChannelPlay(tempStream, FALSE);

    float fft[1024];
    float tempfft[140];

    qDebug() << playingSongPlaylist << currentID;

    int k = 0;
    for (float i = 0; i < time; i += time / 140, k++)
    {
        BASS_ChannelGetData(tempStream, fft, BASS_DATA_FFT2048);

        vector <float> peaks;

        for (int j = 0; j < 30; j++)
        {
            float max = sqrt(fft[1]) * 3 * 40 - 4;
            if (max < 0) max = 0;
            if (max > 40) max = 40;

            for (int l = 2; l <= avgLen; l++)
            {
                float value = sqrt(fft[l]) * 3 * 40 - 4;

                if (value < 0) value = 0;
                if (value > 40) value = 40;

                if (value > max && std::find(peaks.begin(), peaks.end(), value) == peaks.end())
                    max = value;
            }

            if (std::find(peaks.begin(), peaks.end(), max) == peaks.end())
                peaks.push_back(max);
        }

        float avgMax = 0;
        if (peaks.size() != 0)
            avgMax = accumulate(peaks.begin(), peaks.end(), 0.0) / peaks.size();

        if (avgMax <= 3) avgMax = 3;
        else if (avgMax > 40) avgMax = 40;

        tempfft[k] = avgMax;

        do {
            int value = tempfft[k] / 10;
            if (value < 1) value = 1;

            prerenderedFft[k] += value;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (currentID >= this->playlists[playingSongPlaylist].size())
                return;
            if (currentID == -1 || this->playlists[playingSongPlaylist][currentID].path != file) {
                clearPrerenderedFft();
                BASS_StreamFree(tempStream);
                return;
            }
        } while (prerenderedFft[k] < tempfft[k]);

        if (currentID >= this->playlists[playingSongPlaylist].size())
            return;

        if (currentID == -1 || this->playlists[playingSongPlaylist][currentID].path != file) {
            clearPrerenderedFft();
            BASS_StreamFree(tempStream);
            return;
        }

        BASS_ChannelSetPosition(tempStream, BASS_ChannelSeconds2Bytes(tempStream, i), BASS_POS_BYTE);
    }

    BASS_StreamFree(tempStream);
    writeLog("Prerendering fft ended");
}

void MainWindow::mouseMoveEvent (QMouseEvent * event) {
    float mouseX = event->pos().x();
    float mouseY = event->pos().y();

    if (mouseX > 50 && mouseX < 750 && mouseY > 350 && mouseY < 390) {
        this->setCursor(Qt::PointingHandCursor);

        timecode->repaint();
        timecode->clear();
        timecode->move(mouseX + 10, mouseY + 10);

        double pos = ((mouseX - 50) / (700.0f)) * getDuration();

        if (currentID != -1 && playlists[playingSongPlaylist][currentID].marks.find(pos) != playlists[playingSongPlaylist][currentID].marks.end())
            timecode->setText(seconds2qstring(pos) + " (" + playlists[playingSongPlaylist][currentID].marks.find(pos)->second + ")");
        else
            timecode->setText(seconds2qstring(pos));

        timecode->adjustSize();
        timecode->show();
    }
    else {
        this->setCursor(Qt::ArrowCursor);
        timecode->hide();
    }

    if (mouseX >= 15 && mouseX <= 790 && mouseY >= 405 && mouseY <= 430)
    {
        playlistsBar->raise();
    } else {
        playlistsBar->lower();
    }

    if (mouseX >= 15 && mouseX <= 790 && mouseY >= 430 && mouseY <= 620)
    {
        playlistWidget->raise();
        searchSong->raise();
    }
    else {
        playlistWidget->lower();
        searchSong->lower();
    }

    if (timerBtn->underMouse() && timerStarted) {
        if (songTimerMode == 0)
            timerBtn->setToolTip("Time remaining: " + seconds2qstring(songTimer->remainingTime() / 1000));
        if (songTimerMode == 1)
            timerBtn->setToolTip("Tracks remaining: " + QString::number(songTimerCounter));
    } else {
        timerBtn->setToolTip("Timer");
    }

    if (remoteBtn->underMouse() && remoteServerEnabled)
        remoteBtn->setToolTip("Remote Control. " + QString::number(remoteDevices.size()) + " devices connected!");

    if (volumeBtn->underMouse())
    {
        volumeBtn->setToolTip("Volume: " + QString::number(volume * 100));
        volumeSliderToggled = true;
    }
    if (volumeSliderToggled && mouseX >= 630 && mouseX <= 785 && mouseY >= 291 && mouseY <= 321) volumeSlider->show();
    else {
        volumeSlider->hide();
        volumeSliderToggled = false;
    }

    if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->buttons().testFlag(Qt::LeftButton) && moving) {
        this->move(this->pos() + (event->pos() - lastMousePosition));
    }
}
void MainWindow::mousePressEvent (QMouseEvent * event) {
    float mouseX = event->pos().x();
    float mouseY = event->pos().y();

    if (mouseX > 50 && mouseX < 750 && mouseY > 350 && mouseY < 390) {
        if (event->button() == Qt::LeftButton)
        {
            if (getPosition() == getDuration())
            {
                BASS_ChannelStop(channel);
                BASS_ChannelPlay(channel, true);
            }

            double newtime = ((mouseX - 50) / (700.0f)) * getDuration();

            setPosition(newtime);
            taskbarProgress->setValue(newtime);
            writeLog("Song position changed to: " + seconds2qstring(newtime) + " (" + QString::number(newtime) + "s)");
        }
        else if (event->button() == Qt::RightButton) {
            timecode->hide();

            QMenu myMenu;
            myMenu.setMouseTracking(true);
            myMenu.setStyleSheet("QMenu { background-color: #121212; color: silver }");

            QAction * jumpAction = new QAction("Jump to...", this);
            jumpAction->setShortcut(QKeySequence("Ctrl+J"));
            jumpAction->setShortcutVisibleInContextMenu(true);
            jumpAction->setCheckable(true);
            jumpAction->setChecked(looped);
            myMenu.addAction(jumpAction);

            QAction * loopAction = new QAction("Make loop (A-B)", this);
            loopAction->setShortcut(QKeySequence("Ctrl+L"));
            loopAction->setShortcutVisibleInContextMenu(true);
            loopAction->setCheckable(true);
            loopAction->setChecked(looped);
            myMenu.addAction(loopAction);

            myMenu.addSeparator();

            QAction * specAction = new QAction("Live spectrum", this);
            specAction->setShortcut(QKeySequence("Ctrl+Shift+L"));
            specAction->setShortcutVisibleInContextMenu(true);
            specAction->setCheckable(true);
            specAction->setChecked(liveSpec);
            myMenu.addAction(specAction);

            QAction * selectedItem = myMenu.exec(mapToGlobal(QPoint(mouseX, mouseY)));

            if (selectedItem)
            {
                if (selectedItem->text() == "Jump to...")
                    jumpTo();
                if (looped && selectedItem->text() == "Make loop (A-B)")
                    removeLoop();
                else if (!looped && selectedItem->text() == "Make loop (A-B)") {
                    makeLoop();
                }
                if (selectedItem->text() == "Live spectrum")
                {
                    liveSpec = !liveSpec;
                    writeLog((liveSpec) ? "Live spectrum enabled" : "Live spectrum disabled");
                }
            }
        }
    }

    if (!titlebarWidget->underMouse() && !windowTitle->underMouse())
        return;

    if(event->button() == Qt::LeftButton) {
        moving = true;
        lastMousePosition = event->pos();
    }


}
void MainWindow::wheelEvent(QWheelEvent * event) {
    float mouseX = event->position().x();
    float mouseY = event->position().y();

    double new_time = getPosition();

    if (mouseX > 50 && mouseX < 750 && mouseY > 350 && mouseY < 390) {
        // Pause channel while scrolling (for better perfomance)
        BASS_ChannelPause(channel);

        if (event->angleDelta().ry() < 0)
        {
            new_time -= 3;
        }
        else if (event->angleDelta().ry() > 0) {
            new_time += 3;
        }

        setPosition(new_time);
        taskbarProgress->setValue(new_time);
    }  
}
void MainWindow::settings () {
    bool * colorChangePtr = &colorChanging;
    settingsWin->colorChanger = colorChangePtr;
    settingsWin->mainColor = &mainColor;
    settingsWin->mainColorStr = &mainColorStr;
    settingsWin->colorChangeSpeed = &colorChangeSpeed;

    settingsWin->init();
    settingsWin->raise();
    settingsWin->setFocus();
    settingsWin->show();
    settingsWin->move (this->pos().x() + 200, this->pos().y() + 150);

    for (int i = 0; i < 7; i++)
    {
        connect(settingsWin->colorBtns[i], &QPushButton::pressed, [=] () {
            reloadStyles();
            equalizerWin->reloadStyles();
        });
    }
}
void MainWindow::visualizations () {
    visualWin->raise();
    visualWin->setFocus();
    visualWin->show();

    visualWindowOpened = true;
    visualBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
}
void MainWindow::reloadStyles () {
    closeBtn->setStyleSheet("QPushButton { font-size: 18px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    volumeSlider->setStyleSheet("QSlider::groove:horizontal {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 7px 0px;"\
                                "}" \
                               "QSlider::handle:horizontal {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "width: 5px; " \
                                    "margin: -5px -2px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::sub-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0px;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}" \
                                "QSlider::add-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0px;" \
                                    "background: silver; " \
                                "}");

    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; padding-left: 15px; font-size: 14px; /*border: 1px solid silver;*/ border-radius: 5px; border-top-left-radius: 0px; background-color: #141414; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");

    QScrollBar * vbar = playlistWidget->verticalScrollBar();
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #101010; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar * hbar = playlistWidget->horizontalScrollBar();
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #101010; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    playlistsBar->setStyleSheet("QTabBar { height: 25px; font-size: 12px; border: 0px solid silver; background-color: #101010; color: silver; }" \
                                "QTabBar::tab:selected { background-color: #141414; color: " + QString::fromStdString(mainColorStr) + ";}" \
                                "QTabBar::tab:last { border-right: 0px solid #101010; } " \
                                "QTabBar::scroller { width: 40px; }" \
                                "QTabBar::close-button { padding: 4px; image: url(images/close.png); }" \
                                "QTabBar QToolButton { border: 0px solid black; color: silver; background-color: #101010; }" \
                                "QTabBar::tab { height: 25px; background-color: #101010; padding: 0px 20px; max-width: 150px; border: 0px solid silver; border-bottom: 1px solid silver; border-right: 4px solid #101010; color: silver; }" \
                                "QTabBar::tear { border: 0px solid black; }");

    if (shuffle) shuffleBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (repeat) repeatBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (remoteServerEnabled) remoteBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (visualWindowOpened)
        visualBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (equoEnabled)
        equoBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (lyricsEnabled)
        lyricsBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");
    if (timerStarted)
        timerBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + "; }");

    marksList->setStyleSheet("QListWidget { font-size: 13px; border: 1px solid transparent; outline: none; background-color: #141414; color: silver; padding: 5px; }"
                             "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }"
                             "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");
}

void MainWindow::colorChange()
{
    if (!colorChanging) return;

    static int dr = 0;
    static int dg = 0;
    static int db = 0;

    if (mainColor == nextColor) {
        nextColor = settingsWin->colors[rand() % 7];

        dr = (mainColor.red() - nextColor.red()) / colorChangeSpeed;
        dg = (mainColor.green() - nextColor.green()) / colorChangeSpeed;
        db = (mainColor.blue() - nextColor.blue()) / colorChangeSpeed;

    } else {
        if (dr < 0) {
            dr++;
            mainColor.setRed(mainColor.red() + colorChangeSpeed);
        }
        if (dr > 0) {
            dr--;
            mainColor.setRed(mainColor.red() - colorChangeSpeed);
        }

        if (dg < 0) {
            dg++;
            mainColor.setGreen(mainColor.green() + colorChangeSpeed);
        }
        if (dg > 0) {
            dg--;
            mainColor.setGreen(mainColor.green() - colorChangeSpeed);
        }

        if (db < 0) {
            db++;
            mainColor.setBlue(mainColor.blue() + colorChangeSpeed);
        }
        if (db > 0) {
            db--;
            mainColor.setBlue(mainColor.blue() - colorChangeSpeed);
        }

        if (dr == 0) mainColor.setRed(nextColor.red());
        if (dg == 0) mainColor.setGreen(nextColor.green());
        if (db == 0) mainColor.setBlue(nextColor.blue());

        mainColorStr = settingsWin->qcolorToStr(mainColor);
    }

    reloadStyles ();
    equalizerWin->reloadStyles();
}

//           --- Web Socket Functions ---
void MainWindow::remoteControl() {
    if (!remoteServerEnabled) {
        if (removeControlServer->listen(QHostAddress::Any)) {
            qDebug() << "Remote control socket started listening in port " << removeControlServer->serverPort();

            remoteServerEnabled = true;
            reloadStyles();

            connect(removeControlServer, &QWebSocketServer::newConnection, this, &MainWindow::remoteDeviceConnect);
        }
        else {
            qDebug() << "Failed to start socket! " << removeControlServer->errorString();
        }

        if (httpServer->listen(QHostAddress::Any, httpServerPort)) {
            qDebug() << "Http server started with port" << httpServerPort;

            connect(httpServer, &QTcpServer::newConnection, this, &MainWindow::httpNewConnection);
        }
        else
        {
            qDebug() << "Failed to start http server!";
        }
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Remote Control Server");

    QString localAddr = getLocalAddress();
    if (localAddr != "localhost")
        msgBox.setText("Remote control server address: " + getLocalAddress() + ":" + QString::number(httpServerPort));
    else
        msgBox.setText("Internet connection disabled/not avalible. By the way, you can use " + localAddr + ":" + QString::number(httpServerPort) + " address on this device for control.");

    msgBox.setStyleSheet("background-color: #101010; color: silver;");
    msgBox.exec();
}
void MainWindow::remoteDeviceConnect () {
    qDebug() << "New connection";

    auto socket = removeControlServer->nextPendingConnection();
    socket->setParent(this);

    connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::getRemoteCommands);
    connect(socket, &QWebSocket::disconnected, this, &MainWindow::deviceDisconnected);

    remoteDevices << socket;

    trayIcon->showMessage("Device connected", "New remote control device connected!", QSystemTrayIcon::Information, 1000);

    if (channel != NULL) {
        QString name = playlists[playingSongPlaylist][currentID].getName();

        sendMessageToRemoteDevices("name|" + name);
        sendMessageToRemoteDevices("vol|" + QString::number(volume * 100));
        sendMessageToRemoteDevices("pos|" + QString::number(getPosition()/getDuration()));
    }
}


void MainWindow::getRemoteCommands (const QString & command) {
    qDebug() << command;

    if (command.contains('|')) {
        if (command.mid(0, command.indexOf('|')) == "pos") {
            QString pos = command.mid(command.indexOf('|') + 1);
            double new_time = getDuration() * (pos.toDouble() / 1000.0f);

            qDebug() << "Pos: " << new_time;
            setPosition(new_time);
            taskbarProgress->setValue(new_time);
        }
        if (command.mid(0, command.indexOf('|')) == "vol") {
            QString vol = command.mid(command.indexOf('|') + 1);
            qDebug() << "Pos: " << vol;
            changeVolume(vol.toDouble());
        }
    }
    else {
        if (command == "shuffle") changeShuffle();
        if (command == "pause") pause();
        if (command == "backward") backward();
        if (command == "forward") forward();
        if (command == "repeat") changeRepeat();
    }
}

void MainWindow::httpNewConnection() {
    QFile htmlFile("index.html");

    if (!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error!";
    }

    qDebug() << "New http server connection!";

    auto socket = httpServer->nextPendingConnection();

    QTextStream in(&htmlFile);
    QString line = "";

    while (!in.atEnd()) {
       line += in.readLine();
    }

    line.replace("<serverip>", getLocalAddress());
    line.replace("<serverport>", QString::number(removeControlServer->serverPort()));

    QString html = QString::fromStdString("HTTP/1.0 200 Ok\r\n"
                   "Content-Type: text/html; charset=\"utf-8\"\r\n"
                   "\r\n") + line;

    socket->write(html.toStdString().c_str());
    socket->flush();
    socket->waitForBytesWritten(3000);

    QTimer::singleShot(3000, [=]() {
        qDebug() << "Connection closed!";
        socket->close();
    });
}
void MainWindow::deviceDisconnected()
{
    QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
    QTextStream(stdout) << getIdentifier(pClient) << " disconnected!\n";
    if (pClient)
    {
        remoteDevices.removeAll(pClient);
        pClient->deleteLater();
    }
}
//           --- Marks Functions ---
void MainWindow::addMark() {
    QDialog dialog(marksWin);
    dialog.setStyleSheet("background-color: #101010; color: silver;");

    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    QLineEdit * timecodeInput = new QLineEdit(&dialog);
    timecodeInput->setText("00:00");
    timecodeInput->setFocus();
    form.addRow("Timecode: ", timecodeInput);

    QLineEdit * descInput = new QLineEdit(&dialog);
    descInput->setText("");
    form.addRow("Description: ", descInput);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        double timecode = qstring2seconds(timecodeInput->text());

        if (timecode > getDuration() || timecode < 0)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error");
            msgBox.setText("Timecode out of range!");
            msgBox.setStyleSheet("background-color: #101010; color: silver;");
            msgBox.exec();
            return;
        }

        playlists[playingSongPlaylist][currentID].marks[timecode] = descInput->text();
        if (currentPlaylistName == playingSongPlaylist)
            playlist[currentID].marks = playlists[playingSongPlaylist][currentID].marks;
    }

    drawMarksList();
}
void MainWindow::removeMark() {
    if (marksList->currentRow() == -1) return;

    QString itemText = marksList->currentItem()->text();
    int start = itemText.indexOf('[');
    int end = itemText.mid(start + 1).indexOf(']');

    QString timecode = itemText.mid(start + 1, end);

    playlists[playingSongPlaylist][currentID].marks.erase(qstring2seconds(timecode));
    if (currentPlaylistName == playingSongPlaylist)
        playlist[currentID].marks = playlists[playingSongPlaylist][currentID].marks;

    drawMarksList();
}
void MainWindow::editMark() {

}
void MainWindow::drawMarksList () {
    marksList->clear();

    for (auto & mark : playlists[playingSongPlaylist][currentID].marks)
        marksList->addItem("[" + seconds2qstring(mark.first) + "] " + mark.second);
}

//           --- Song timer Functions ---
void MainWindow::initTimerWindow() {
    songTimer = new QTimer(this);

    timerWin = new QWidget();
    timerWin->setWindowFlags(Qt::Drawer);
    timerWin->setWindowTitle("Timer");
    timerWin->setGeometry(0, 0, 300, 180);
    timerWin->move(this->pos().x() + (this->width() - timerWin->width()) / 2, this->pos().y() + (this->height() - timerWin->height()) / 2);
    timerWin->setMaximumSize(300, 180);
    timerWin->setMinimumSize(300, 180);
    timerWin->setStyleSheet("background-color: #101010; color: silver;");

    QFormLayout form(timerWin);

    QRadioButton * timerModeRadio = new QRadioButton[2];
    QLineEdit * timerModeLines = new QLineEdit[2];
    for (int i = 0; i < 2; i++) {
        timerModeRadio[i].setGeometry(15, 15 + i * 50, 275, 15);
        timerModeLines[i].setGeometry(15, 35 + i * 50, 275, 20);
        timerModeLines[i].setDisabled(true);

        connect (&timerModeRadio[i], &QRadioButton::clicked, [=]() {
            timerModeLines[i].setDisabled(false);
            timerModeLines[i].setFocus();
            timerModeLines[i].selectAll();
            songTimerMode = i;
        });

        form.addRow(&timerModeRadio[i]);
        form.addRow(&timerModeLines[i]);
    }

    timerModeRadio[0].setText("Timer by time");
    timerModeRadio[1].setText("Timer by songs count");

    timerModeLines[0].setText("00:00");
    timerModeLines[1].setText("3");

    QPushButton * timerStart = new QPushButton("Start");
    timerStart->setGeometry(15, timerWin->height() - 45, 130, 30);
    QPushButton * timerStop = new QPushButton("Stop");
    timerStop->setGeometry(155, timerWin->height() - 45, 130, 30);

    QLabel * remainingLabel = new QLabel("Remaining: 00:00");
    remainingLabel->setGeometry(15, timerWin->height() - 75, 275, 20);
    remainingLabel->hide();

    form.addRow(remainingLabel);
    form.addRow(timerStart, timerStop);

    connect (timerStart, &QPushButton::clicked, [=]() {
        if (songTimerMode == -1) return;

        if (timerStarted) {
            timerStarted = false;

            if (songTimerMode == 0)
                songTimer->stop();
            if (songTimerMode == 1)
                songTimerCounter = -1;
        }

        if (songTimerMode == 0) {
            songTimer->setSingleShot(true);
            songTimer->setInterval(qstring2seconds(timerModeLines[0].text()) * 1000);
            songTimer->start();

            connect (songTimer, &QTimer::timeout, this, &MainWindow::songTimerEnded);
        }
        if (songTimerMode == 1) {
            songTimerCounter = timerModeLines[1].text().toInt();
        }

        timerStarted = true;
        reloadStyles();
    });

    connect (timerStop, &QPushButton::clicked, [=]() {
        if (songTimerMode == -1) return;

        if (songTimerMode == 0)
            songTimer->stop();

        timerBtn->setStyleSheet("QPushButton { font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver; }");
        timerStarted = false;
    });
}
