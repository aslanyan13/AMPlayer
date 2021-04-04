#include "mainwindow.h"
#include "ui_mainwindow.h"

QColor mainColor(255, 37, 79);
QColor nextColor(255, 37, 79);
string mainColorStr = "rgb(255, 37, 79)";
int colorChangeSpeed = 20;

bool isOrderChanged = false;

QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0)
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
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint); // Window transparency
    this->setStyleSheet("QMainWindow { background-color: #101010; } QInputDialog, QInputDialog * { background-color: #101010; color: silver; }");
    this->setWindowTitle("AMPlayer v1.0a");
    this->setMouseTracking(true);
    this->centralWidget()->setMouseTracking(true);

    // Album cover load
    QImageReader reader(QDir::currentPath() + "/Images/cover-placeholder2.png");
    // QImageReader reader (":/Images/cover-placeholder.png");
    cover = reader.read();

    if (cover.isNull()) {
        coverLoaded = false;
        cover = QImage (":/Images/cover-placeholder.png");
        cout << reader.errorString().toStdString() << endl;
        writeLog ("Can't load cover! " + reader.errorString());
    }

    settingsWin = new settingsWindow();

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
    windowTitle->setObjectName("windowTitle");
    windowTitle->setText("AMPlayer v1.0a");

    QPushButton * menuBtn = new QPushButton(this);
    menuBtn->setFont(fontAwesome);
    menuBtn->setGeometry(10, 10, 15, 15);
    menuBtn->setStyleSheet("font-size: 15px; border: 0px solid silver; background-color: #101010; color: gray;");
    menuBtn->setCursor(Qt::PointingHandCursor);
    menuBtn->setText(QString::fromStdWString(L"⬤"));
    menuBtn->show();

    closeBtn = new QPushButton(this);
    closeBtn->setFont(fontAwesome);
    closeBtn->setGeometry(775, 10, 15, 15);
    closeBtn->setStyleSheet("font-size: 15px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setText(QString::fromStdWString(L"⬤"));
    closeBtn->show();

    minimizeBtn = new QPushButton(this);
    minimizeBtn->setFont(fontAwesome);
    minimizeBtn->setGeometry(750, 10, 15, 15);
    minimizeBtn->setStyleSheet("font-size: 15px; border: 0px solid silver; background-color: #101010; color: silver;");
    minimizeBtn->setCursor(Qt::PointingHandCursor);
    minimizeBtn->setText(QString::fromStdWString(L"⬤"));
    minimizeBtn->show();

    pauseBtn = new QPushButton(this);
    pauseBtn->setFont(fontAwesome);
    pauseBtn->setGeometry(380, 285, 50, 50);
    pauseBtn->setStyleSheet("font-size: 36px; border: 0px solid silver; background-color: #101010; color: silver;");
    pauseBtn->setText("\uf04b");
    pauseBtn->setCursor(Qt::PointingHandCursor);
    pauseBtn->show();

    QPushButton * forwardBtn = new QPushButton(this);
    forwardBtn->setFont(fontAwesome);
    forwardBtn->setGeometry(445, 290, 40, 40);
    forwardBtn->setStyleSheet("vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #101010; color: silver;");
    forwardBtn->setText("\uf04e");
    forwardBtn->setCursor(Qt::PointingHandCursor);
    forwardBtn->show();

    QPushButton * backwardBtn = new QPushButton(this);
    backwardBtn->setFont(fontAwesome);
    backwardBtn->setGeometry(325, 290, 40, 40);
    backwardBtn->setStyleSheet("vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #101010; color: silver;");
    backwardBtn->setText("\uf04a");
    backwardBtn->setCursor(Qt::PointingHandCursor);
    backwardBtn->show();

    repeatBtn = new QPushButton(this);
    repeatBtn->setFont(fontAwesome);
    repeatBtn->setGeometry(500, 291, 30, 30);
    repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    repeatBtn->setText("\uf363");
    repeatBtn->setCursor(Qt::PointingHandCursor);
    repeatBtn->show();

    equoBtn = new QPushButton(this);
    equoBtn->setFont(fontAwesome);
    equoBtn->setGeometry(235, 291, 30, 30);
    equoBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    equoBtn->setText("\uf3f1");
    equoBtn->setCursor(Qt::PointingHandCursor);
    equoBtn->show();

    shuffleBtn = new QPushButton(this);
    shuffleBtn->setFont(fontAwesome);
    shuffleBtn->setGeometry(275, 291, 30, 30);
    shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    shuffleBtn->setText("\uf074");
    shuffleBtn->setCursor(Qt::PointingHandCursor);
    shuffleBtn->show();

    QPushButton * remoteBtn = new QPushButton(this);
    remoteBtn->setFont(fontAwesome);
    remoteBtn->setGeometry(155, 291, 30, 30);
    remoteBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    remoteBtn->setText("\uf3cd");
    remoteBtn->setCursor(Qt::PointingHandCursor);
    remoteBtn->show();

    timerBtn = new QPushButton (this);
    timerBtn->setFont(fontAwesome);
    timerBtn->setGeometry(195, 291, 30, 30);
    timerBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    timerBtn->setText("\uf017");
    timerBtn->setCursor(Qt::PointingHandCursor);
    timerBtn->show();

    audio3dBtn = new QPushButton(this);
    audio3dBtn->setFont(fontAwesome);
    audio3dBtn->setGeometry(540, 291, 30, 30);
    audio3dBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    audio3dBtn->setText("\uf1b2");
    audio3dBtn->setCursor(Qt::PointingHandCursor);
    audio3dBtn->show();

    visualBtn = new QPushButton(this);
    visualBtn->setFont(fontAwesome);
    visualBtn->setGeometry(580, 291, 30, 30);
    visualBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    visualBtn->setText("\uf26c");
    visualBtn->setCursor(Qt::PointingHandCursor);
    visualBtn->show();

    volumeBtn = new QPushButton (this);
    volumeBtn->setFont(fontAwesome);
    volumeBtn->setGeometry(620, 291, 30, 30);
    volumeBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
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

    QLabel * songInfo = new QLabel(this);
    songInfo->setText("Genre, 44.1k MP3");
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
    timecode->setStyleSheet("font-size: 10px; border: 1px solid silver; background-color: #101010; color: silver;");

    playlistsBar = new QTabBar(this);
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
                                "QTabBar::tab { height: 25px; background-color: #101010; padding: 0px 20px; min-width: 80px; max-width: 150px; border: 0px solid silver; border-bottom: 1px solid silver; border-right: 4px solid #101010; color: silver; }" \
                                "QTabBar::tear { border: 0px solid black; }");

    playlistsBar->show();


    playlistWidget = new QListWidget (this);

    QScrollBar *vbar = playlistWidget->verticalScrollBar();
    vbar->setStyle( new QCommonStyle );
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #101010; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar *hbar = playlistWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #101010; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    playlistWidget->setDragDropMode(QAbstractItemView::DragDrop);
    playlistWidget->setDefaultDropAction(Qt::MoveAction);


    playlistWidget->setGeometry(15, 455, 775, 160);
    playlistWidget->lower();
    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; padding-left: 15px; font-size: 14px; /*border: 1px solid silver;*/ border-radius: 5px; border-top-left-radius: 0px; background-color: #141414; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");

    playlistWidget->show();

    searchSong = new QLineEdit(this);
    searchSong->setStyleSheet("QLineEdit { padding: 0px 20px; font-size: 12px; border: 0px solid silver; border-bottom: 1px solid #101010; background-color: #141414; color: silver; }" \
                              "QLineEdit:focus { /* border-bottom: 1px solid " + QString::fromStdString(mainColorStr) + "; */ }");
    searchSong->setGeometry(15, 430, 775, 25);
    searchSong->setPlaceholderText("Search song");
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

    volumeSlider = new QSlider (Qt::Vertical, this);
    volumeSlider->setMaximum(100);
    volumeSlider->setMinimum(0);
    volumeSlider->setValue(100);
    volumeSlider->setCursor(Qt::PointingHandCursor);
    volumeSlider->setGeometry(625, 141, 20, 150);
    volumeSlider->setStyleSheet("QSlider::groove:vertical {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 0px 7px;"\
                                "}" \
                               "QSlider::handle:vertical {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "height: 5px; " \
                                    "margin: -2px -5px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::add-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}" \
                                "QSlider::sub-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0 7px;" \
                                    "background: silver; " \
                                "}");

    volumeSlider->hide();

    connect (searchSong, SIGNAL(textChanged(const QString &)), this, SLOT(search(const QString &)));

    connect (playlistsBar, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT(playlistsBarContextMenu (const QPoint&)));
    connect (playlistsBar, SIGNAL (tabBarClicked(int)), this, SLOT(changeCurrentPlaylist (int)));
    connect (playlistsBar, SIGNAL (tabCloseRequested(int)), this, SLOT(removePlaylist (int)));

    connect (playlistWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(setActive(QListWidgetItem *)));
    connect (playlistWidget->model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(rowsMoved(QModelIndex, int, int, QModelIndex, int)));

    connect (menuBtn, SIGNAL(clicked()), this, SLOT(menuContext()));

    connect (minimizeBtn, SIGNAL(clicked()), this, SLOT(slot_minimize()));
    connect (closeBtn, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect (forwardBtn, SIGNAL(clicked()), this, SLOT(forward()));
    connect (backwardBtn, SIGNAL(clicked()), this, SLOT(backward()));
    connect (pauseBtn, SIGNAL(clicked()), this, SLOT(pause()));
    connect (repeatBtn, SIGNAL(clicked()), this, SLOT(changeRepeat()));
    connect (shuffleBtn, SIGNAL(clicked()), this, SLOT(changeShuffle()));
    connect (audio3dBtn, SIGNAL(clicked()), this, SLOT(audio3D()));
    connect (equoBtn, SIGNAL(clicked()), this, SLOT(equalizer()));
    connect (visualBtn, SIGNAL(clicked()), this, SLOT(visualizations()));

    connect (volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));

    drawPlaylist();
    drawAllPlaylists();
}

MainWindow::~MainWindow()
{
    QString uptime = QString::fromStdString(seconds2string((clock() - starttime) / 1000));
    writeLog("Exiting...");
    writeLog("Uptime: " + uptime + " (" + QString::number(clock() - starttime) + "ms)");

    logfile.close();

    if (searchSong->text() == "")
        playlists[currentPlaylistName] = playlist;

    XMLreader->writePlaylists(playlists);
    delete ui;
}

void MainWindow::menuContext () {
    QMenu myMenu;
    myMenu.setStyleSheet("background-color: #101010; color: silver");

    myMenu.addAction("Open file(s)");
    myMenu.addAction("Open Folder");
    myMenu.addSeparator();
    myMenu.addAction("Settings");

    QAction * selectedItem = myMenu.exec(this->mapToGlobal(QPoint(20, 20)));

    if (selectedItem)
    {
        if (selectedItem->text() == "Open file(s)") {
            openFile();
        }
        if (selectedItem->text() == "Open Folder") {
            openFolder();
        }
        if (selectedItem->text() == "Settings") {
            settings();
        }
    }
}

bool MainWindow::openFile ()
{
    searchSong->setText("");

    QFileDialog dialog(this);
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("Audio files (*.mo3 *.mp3 *.mp2 *.mp1 *.ogg *.aif *.wav)"));
    QStringList fileNames;

    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    for (int i = 0; i < fileNames.length(); i++)
    {
        qDebug() << fileNames[i];
        Song temp(fileNames[i]);

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

        if (artist == L"") {
            artist = L"Unknown Artist";
        }

        if (title == L"") {
            temp.setNameFromPath();
        } else {
            temp.setName(QString::fromStdWString(artist) + " - " + QString::fromStdWString(title));
        }

        playlist.push_back(temp);
    }

    // "Audio Files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.umx;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif)\0All files\0*.*\0\0";

    playlists[currentPlaylistName] = playlist;

    drawPlaylist();
    if (playlist.size() == 1)
        setActive (0);

    return true;
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
    qDebug() << "moved";
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
        BASS_ChannelStop(channel);
        channel = NULL;
        songTitle->setText("");
        pauseBtn->setText("\uf04b");
        clearPrerenderedFft();
        songPosition->setText("00:00");
        songDuration->setText("00:00");
    }

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
    QStringList musicFiles = directory.entryList(QStringList() << "*.mp3" << "*.mo3" << "*.wav" << "*.mp2" << "*.mp1", QDir::Files | QDir::Readable);

    writeLog("Folder opened: " + folder);
    writeLog("Folder contains: " + QString::number (musicFiles.length()) + " files");

    int duplicatesCount = 0;
    for (int i = 0; i < musicFiles.length(); i++)
    {
        QString fullpath = folder + "/" + musicFiles[i];

        Song temp(fullpath);

        TagLib::FileRef f(fullpath.toStdWString().c_str());

        wstring artist = L"", title = L"";

        // If file not load failed
        if (!f.isNull()) {
            artist = f.tag()->artist().toCWString();
            title = f.tag()->title().toCWString();
        }

        writeLog("Opened file: " + musicFiles[i]);

        if (artist == L"") {
            artist = L"Unknown Artist";
        }

        if (title == L"") {
            temp.setNameFromPath();
        } else {
            temp.setName(QString::fromStdWString(artist) + " - " + QString::fromStdWString(title));
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
    QString name;

    do {
        bool ok;
        name = QInputDialog::getText(this, tr("Playlist Name"),  tr("Playlist name:"), QLineEdit::Normal, NULL, &ok);

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

    drawAllPlaylists();
}

void MainWindow::removePlaylist (int index) {
    QString playlistName = playlistsBar->tabText(index);

    if (playlists[playlistName].size() > 0) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText("Are you sure to delete \'" + playlistName + "\'playlist?");
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
            BASS_ChannelStop(channel);
            channel = NULL;
            pauseBtn->setText("\uf04b");
            songTitle->setText("");
            clearPrerenderedFft();
            songPosition->setText("00:00");
            songDuration->setText("00:00");
        }
    }

    // Deleting playlist and setting first playlist as current
    writeLog("Removed playlist: " + playlistName);

    if (playlistName == "Default") playlists["Default"] = vector<Song>();
    else {
        playlists.erase(playlistName);
    }

    currentPlaylistName = "Default";
    playlist = playlists[currentPlaylistName];

    drawPlaylist();
    drawAllPlaylists();
}
void MainWindow::search(const QString & text)
{
    cout << text.toStdString() << endl;
    writeLog("Searching query: " + text);

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

    for (auto &playlist : playlists)
    {
        playlistsBar->addTab(playlist.first);
    }
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
        QListWidgetItem *  songItem = new QListWidgetItem(playlistWidget);
        QString path = playlist[i].path;
        QString name = playlist[i].getName();

        songItem->setData(Qt::UserRole, i);
        // songItem->setText(QString::number(i + 1) + ". " + name);
        songItem->setText(name);

        playlistWidget->addItem(songItem);
   }
}
void MainWindow::setTitle () {
    QString name = playlists[currentPlaylistName][currentID].getName();

    if (name.length() > 32)
        name = name.mid(0, 32) + "...";

    songTitle->setText(name);
}
void MainWindow::changeCurrentPlaylist (int index) {
    if (playlistsBar->tabText(index) == "") {
        playlistsBar->removeTab(index);
        return;
    }

    searchSong->setText("");
    playlists[currentPlaylistName] = playlist;

    writeLog("\nPlaylist changed to: " + playlistsBar->tabText(index));

    playlist = playlists[playlistsBar->tabText(index)];
    currentPlaylistName = playlistsBar->tabText(index);

    drawPlaylist();
    // drawAllPlaylists();
}

void MainWindow::playlistsBarContextMenu (const QPoint& point) {
    int tabIndex = playlistsBar->tabAt(point);
    qDebug() << tabIndex;

    QPoint globalPos = playlistsBar->mapToGlobal(point);

    QMenu myMenu;
    myMenu.setStyleSheet("background-color: #101010; color: silver");
    myMenu.addAction("Create Playlist");
    myMenu.addAction("Rename");
    myMenu.addAction("Remove");

    QAction* selectedItem = myMenu.exec(globalPos);

    if (selectedItem)
    {
        if (selectedItem->text() == "Create Playlist") {
            createPlaylist();
        }
        if (selectedItem->text() == "Remove") {
            removePlaylist(tabIndex);
        }
    }
    else
    {
        // nothing was chosen
    }
}

void MainWindow::setActive(QListWidgetItem * item) {
    setActive (playlistWidget->currentRow());
}
void MainWindow::setActive (int index) {
    paused = true;
    pauseBtn->setText("\uf04c"); // Set symbol to pause

    playingSongPlaylist = currentPlaylistName;

    QString path;

    if (searchSong->text() == "")  {
        currentID = index;
        path = playlist[currentID].path;
    }
    else {
        Song temp(playlist[index].path);
        currentID = std::find(playlists[currentPlaylistName].begin(), playlists[currentPlaylistName].end(), temp) - playlists[currentPlaylistName].begin();
        path = playlists[currentPlaylistName][currentID].path;
    }

    if (channel != NULL)
        BASS_StreamFree(channel);

    channel = BASS_StreamCreateFile(FALSE, path.toStdWString().c_str(), 0, 0, 0);

    prerenderFft();

    BASS_ChannelPlay(channel, false);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

    QString pos = QString::fromStdString(seconds2string(getPosition()));
    songPosition->setText(pos);
    QString len = QString::fromStdString(seconds2string(getDuration()));
    songDuration->setText(len);

    playlistWidget->setCurrentRow(index);
    writeLog("Song changed to: " + playlistWidget->currentItem()->text());

    setTitle();
}
void MainWindow::forward () {
    if (channel == NULL || playlist.size() == 1)
        return;

    writeLog("Forward button pressed");

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlist.size();
        } while (songID == currentID);

        currentID = songID;
    }
    else currentID++;

    if (currentID >= playlist.size()) currentID = 0;

    setActive(currentID);
}
void MainWindow::backward () {
    if (channel == NULL || playlist.size() == 1)
        return;

    writeLog("Backward button pressed");

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlist.size();
        } while (songID == currentID);

        currentID = songID;
    }
    else currentID--;

    if (currentID < 0) currentID = playlist.size() - 1;

    setActive(currentID);
}
void MainWindow::pause()
{
    if (playlistWidget->currentRow() != -1 && channel == NULL) {
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
    }
    else {
        writeLog("Song paused");
        BASS_ChannelPause(channel);
        pauseBtn->setText("\uf04b"); // Set symbol to play
    }
}
void MainWindow::changeVolume (int vol)
{
    volume = vol / 100.0f;
    writeLog("Volume changed to: " + QString::number(vol));
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
}
void MainWindow::changeRepeat () {
    if (shuffle == true)
        changeShuffle();

    repeat = !repeat;

    if (repeat) {
        writeLog("Repeat enabled");
        repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
    }
    else {
        writeLog("Repeat disabled");
        repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    }
}
void MainWindow::changeShuffle () {
    if (repeat == true)
        changeRepeat();

    shuffle = !shuffle;

    if (shuffle) {
        writeLog("Shuffle enabled");
        shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
    }
    else {
        writeLog("Shuffle disabled");
        shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: silver;");
    }
}

string MainWindow::seconds2string (float seconds) {
    int hours = (seconds > 3600) ? (int)seconds / 3600 : 0;
    int minutes = (seconds > 60) ? ((int)seconds - hours * 3600) / 60 : 0;
    int secs = (int)seconds % 60;

    string result = "";
    if (hours != 0)
    {
        if (hours < 10) result += "0";
        result += to_string (hours);
        result += ":";
    }

    if (minutes < 10) result += "0";
    result += to_string (minutes);
    result += ":";

    if (secs < 10) result += "0";
    result += to_string (secs);

    return result;
}

void MainWindow::updateTime() {
    if (isOrderChanged)
    {
        reorderPlaylist();
        isOrderChanged = false;
    }

    if (channel != NULL) {
        QString pos = QString::fromStdString(seconds2string(getPosition()));
        songPosition->setText(pos);

        if (getPosition() >= getDuration())
        {
            writeLog("Song ended");

            if (repeat) {
                writeLog("Song restarted");
                BASS_StreamFree(channel);
                channel = BASS_StreamCreateFile(FALSE, playlist[currentID].path.toStdWString().c_str(), 0, 0, 0);
                BASS_ChannelPlay(channel, true);
                BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
            }
            else if (shuffle && playlist.size() > 1) {
                int songID;
                do {
                    songID = rand() % playlist.size();
                } while (songID == currentID);

                currentID = songID;

                playlistWidget->setCurrentRow(currentID);

                BASS_StreamFree(channel);
                channel = BASS_StreamCreateFile(FALSE, playlist[currentID].path.toStdWString().c_str(), 0, 0, 0);

                prerenderFft();

                BASS_ChannelPlay(channel, true);
                BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

                QString pos = QString::fromStdString(seconds2string(getPosition()));
                songPosition->setText(pos);
                QString len = QString::fromStdString(seconds2string(getDuration()));
                songDuration->setText(len);

                setTitle();
            } else {
                writeLog("Changing to next song");
                forward();
            }
        }
    }

    colorChange();
    repaint();
}

void MainWindow::paintEvent(QPaintEvent * event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::transparent, 0));

    if (!cover.isNull() && coverLoaded) {
        QGraphicsBlurEffect * blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(10);
        QImage cover2 = cover.scaled(250, 250, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        cover2 = applyEffectToImage(cover2, blur);

        QBrush brush(cover2);
        QTransform transform;

        transform.translate(25, -220);
        brush.setTransform(transform);
        painter.setBrush(brush);
        painter.drawRoundedRect(275, 30, 250, 250, 10, 10);

        // QRadialGradient gradient(400, 155, 180);
        // gradient.setColorAt(0, QColor(20, 20, 20, 200));
        // gradient.setColorAt(1, QColor(20, 20, 20, 255));

        // QBrush brush2(gradient);

        painter.setBrush(QBrush(QColor(16, 16, 16, 230)));
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
            if (i * (getDuration() / 140) <= getPosition())
                p.fillPath(path, color);
            else
                p.fillPath(path, QColor (192, 192, 192));
            p.drawPath(path);
        }
    } else {
        for (int i = 0; i < 140; i++) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(QRectF(50 + 5 * i, 350 + 10 + (20 - prerenderedFft[i]) / 2, 3, prerenderedFft[i]), 2, 2);
            QPen pen(Qt::transparent, 0);
            p.setPen(pen);

            if (i * (getDuration() / 140) <= getPosition())
                p.fillPath(path, color);
            else
                p.fillPath(path, QColor (192, 192, 192));

            p.drawPath(path);
        }
    }
}

void MainWindow::prerenderFft ()
{   
    if (channel == NULL) return;

    writeLog("Prerendering fft...");

    int k = 0;
    float time = getDuration();

    int avgLen = 16;

    float vol = volume;
    changeVolume(0);
    BASS_ChannelPlay(channel, FALSE);

    float fft[1024];
    for (float i = 0; i < time; i+= time / 140, k++)
    {
        BASS_ChannelGetData(channel, fft, BASS_DATA_FFT2048);

        float max = 0;
        for (int j = 1; j <= avgLen; j++)
            max += sqrt(fft[j]) * 3 * 40 - 4;

        max /= avgLen;
        max *= 3;

        if (max <= 3) max = 3;
        else if (max > 40) max = 40;

        prerenderedFft[(int)k] = max;

        BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, i), BASS_POS_BYTE);
    }

    BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, 0), BASS_POS_BYTE);
    changeVolume(vol * 100);

    writeLog("Prerendering fft ended");
}

void MainWindow::mouseMoveEvent (QMouseEvent * event) {
    float mouseX = event->pos().x();
    float mouseY = event->pos().y();

    if (mouseX > 50 && mouseX < 750 && mouseY > 350 && mouseY < 390) {
        this->setCursor(Qt::PointingHandCursor);

        timecode->repaint();
        timecode->clear();
        timecode->setGeometry (mouseX + 10, mouseY + 10, 50, 20);
        timecode->setText(QString::fromStdString(seconds2string(((mouseX - 50) / (700.0f)) * getDuration())));
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

    if (mouseX >= 15 && mouseX <= 790 && mouseY >= 430 && mouseY <= 590)
    {
        playlistWidget->raise();
    }
    else {
        playlistWidget->lower();
    }

    if (volumeBtn->underMouse()) volumeSlider->show();

    if (!volumeBtn->underMouse() && !volumeSlider->underMouse()) volumeSlider->hide();

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

            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, newtime), BASS_POS_BYTE);
            writeLog("Song position changed to: " + QString::fromStdString(seconds2string(newtime)) + " (" + QString::number(newtime) + "s)");
        }
        else if (event->button() == Qt::RightButton) {
            liveSpec = !liveSpec;
            writeLog((liveSpec) ? "Live spectrum enabled" : "Live spectrum disabled");
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

    if (mouseX > 50 && mouseX < 750 && mouseY > 350 && mouseY < 390) {
        if (event->angleDelta().ry() < 0)
        {
            double new_time = getPosition() - 3;
            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, new_time), BASS_POS_BYTE);
        } else {
            double new_time = getPosition() + 3;
            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, new_time), BASS_POS_BYTE);
        }
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
        });
    }
}
void MainWindow::reloadStyles () {
    closeBtn->setStyleSheet("font-size: 15px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
    volumeSlider->setStyleSheet("QSlider::groove:vertical {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #141414;"\
                                    "margin: 0px 7px;"\
                                "}" \
                               "QSlider::handle:vertical {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "height: 5px; " \
                                    "margin: -2px -5px; " \
                                    "border-radius: 20px; " \
                                "}"
                                "QSlider::add-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0px 7px;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}" \
                                "QSlider::sub-page:vertical {" \
                                    "border-radius: 20px;" \
                                    "margin: 0 7px;" \
                                    "background: silver; " \
                                "}");

    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; padding-left: 15px; font-size: 14px; /*border: 1px solid silver;*/ border-radius: 5px; border-top-left-radius: 0px; background-color: #141414; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");

    QScrollBar *vbar = playlistWidget->verticalScrollBar();
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #101010; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar *hbar = playlistWidget->horizontalScrollBar();
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
                                "QTabBar::tab { height: 25px; background-color: #101010; padding: 0px 20px; min-width: 80px; max-width: 150px; border: 0px solid silver; border-bottom: 1px solid silver; border-right: 4px solid #101010; color: silver; }" \
                                "QTabBar::tear { border: 0px solid black; }");

    if (shuffle) shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
    if (repeat) repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #101010; color: " + tr(mainColorStr.c_str()) + ";");
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
}
