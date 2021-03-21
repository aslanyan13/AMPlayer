#include "mainwindow.h"
#include "ui_mainwindow.h"

QColor mainColor(255, 37, 79);
QColor nextColor(255, 37, 79);
string mainColorStr = "rgb(255, 37, 79)";

// rgb(255, 37, 79)  - Raspberry color
// rgb(37, 255, 20)  - Neon green color
// rgb(255, 0, 0)    - Red
// rgb(91, 192, 222) - Blue

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    fifo_map <QString, vector<Song>> test;

    test["Test 3"] = vector<Song> ();
    test["Test 5"] = vector<Song> ();
    test["Test 4"] = vector<Song> ();
    test["Test 2"] = vector<Song> ();
    test["Test 1"] = vector<Song> ();

    cout << test.find("Test 5")->first.toStdString() << endl;

    // Reading playlists from XML file
    XMLreader = new PlaylistReader(QDir::currentPath() + "/XML/playlists.xml");
    XMLreader->readPlaylists(playlists);

    if (playlists.size() > 0)
    {
        currentPlaylistName = playlists.begin()->first; // Settings current playlist name
        playlist = playlists[currentPlaylistName];      // Setting current playlist
    }
    else {
        currentPlaylistName = "Default"; // Settings current playlist name
        playlist = playlists["Default"];      // Setting current playlist
    }

    clearPrerenderedFft();

    int id = QFontDatabase::addApplicationFont(":/Font Awesome 5 Pro Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont fontAwesome(family);

    channel = NULL;

    timer = new QTimer();
    timer->setInterval(1);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start();

    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint); // Window transparency
    this->setStyleSheet("QMainWindow { background-color: #141414; } QInputDialog, QInputDialog * { background-color: #141414; color: silver; }");
    this->setWindowTitle("AMPlayer v1.0a");

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
    menuBtn->setGeometry(10, 10, 30, 30);
    menuBtn->setStyleSheet("font-size: 20px; border: 0px solid silver; background-color: #141414; color: silver;");
    menuBtn->setCursor(Qt::PointingHandCursor);
    menuBtn->setText("\uf013");
    menuBtn->show();

    closeBtn = new QPushButton(this);
    closeBtn->setFont(fontAwesome);
    closeBtn->setGeometry(760, 10, 30, 30);
    closeBtn->setStyleSheet("font-size: 24px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setText("\uf00d");
    closeBtn->show();

    minimizeBtn = new QPushButton(this);
    minimizeBtn->setFont(fontAwesome);
    minimizeBtn->setGeometry(725, 8, 30, 30);
    minimizeBtn->setStyleSheet("font-size: 16px; border: 0px solid silver; background-color: #141414; color: silver;");
    minimizeBtn->setCursor(Qt::PointingHandCursor);
    minimizeBtn->setText("\uf2d1");
    minimizeBtn->show();

    QImageReader reader(":/Images/cover-placeholder.png");
    cover = reader.read();

    if (cover.isNull()) {
        cout << reader.errorString().toStdString() << endl;
    }

    pauseBtn = new QPushButton(this);
    pauseBtn->setFont(fontAwesome);
    pauseBtn->setGeometry(380, 285, 50, 50);
    pauseBtn->setStyleSheet("font-size: 36px; border: 0px solid silver; background-color: #141414; color: silver;");
    pauseBtn->setText("\uf04c"); // f04b - play
    pauseBtn->setCursor(Qt::PointingHandCursor);
    pauseBtn->show();

    QPushButton * forwardBtn = new QPushButton(this);
    forwardBtn->setFont(fontAwesome);
    forwardBtn->setGeometry(445, 290, 40, 40);
    forwardBtn->setStyleSheet("vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #141414; color: silver;");
    forwardBtn->setText("\uf04e");
    forwardBtn->setCursor(Qt::PointingHandCursor);
    forwardBtn->show();

    QPushButton * backwardBtn = new QPushButton(this);
    backwardBtn->setFont(fontAwesome);
    backwardBtn->setGeometry(325, 290, 40, 40);
    backwardBtn->setStyleSheet("vertical-align: middle; border: 0px solid silver; font-size: 26px; background-color: #141414; color: silver;");
    backwardBtn->setText("\uf04a");
    backwardBtn->setCursor(Qt::PointingHandCursor);
    backwardBtn->show();

    repeatBtn = new QPushButton(this);
    repeatBtn->setFont(fontAwesome);
    repeatBtn->setGeometry(500, 291, 30, 30);
    repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    repeatBtn->setText("\uf363");
    repeatBtn->setCursor(Qt::PointingHandCursor);
    repeatBtn->show();

    equoBtn = new QPushButton(this);
    equoBtn->setFont(fontAwesome);
    equoBtn->setGeometry(235, 291, 30, 30);
    equoBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    equoBtn->setText("\uf3f1");
    equoBtn->setCursor(Qt::PointingHandCursor);
    equoBtn->show();

    shuffleBtn = new QPushButton(this);
    shuffleBtn->setFont(fontAwesome);
    shuffleBtn->setGeometry(275, 291, 30, 30);
    shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    shuffleBtn->setText("\uf074");
    shuffleBtn->setCursor(Qt::PointingHandCursor);
    shuffleBtn->show();

    metronomeBtn = new QPushButton(this);
    metronomeBtn->setFont(fontAwesome);
    metronomeBtn->setGeometry(195, 291, 30, 30);
    metronomeBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    metronomeBtn->setText("\uf001");
    metronomeBtn->setCursor(Qt::PointingHandCursor);
    metronomeBtn->show();

    audio3dBtn = new QPushButton(this);
    audio3dBtn->setFont(fontAwesome);
    audio3dBtn->setGeometry(540, 291, 30, 30);
    audio3dBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    audio3dBtn->setText("\uf1b2");
    audio3dBtn->setCursor(Qt::PointingHandCursor);
    audio3dBtn->show();

    visualBtn = new QPushButton(this);
    visualBtn->setFont(fontAwesome);
    visualBtn->setGeometry(580, 291, 30, 30);
    visualBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
    visualBtn->setText("\uf26c");
    visualBtn->setCursor(Qt::PointingHandCursor);
    visualBtn->show();

    songTitle = new QLabel(this);
    songTitle->setText("");
    songTitle->setGeometry(200, 220, 400, 30);
    songTitle->setAlignment(Qt::AlignCenter);
    songTitle->setStyleSheet("/* border: 1px solid silver; */ font-size: 20px; background-color: #141414; color: silver;");
    songTitle->show();

    QLabel * songInfo = new QLabel(this);
    // songInfo->setText("Genre, 44.1k MP3");
    songInfo->setGeometry(200, 255, 400, 16);
    songInfo->setAlignment(Qt::AlignCenter);
    songInfo->setStyleSheet("/* border: 1px solid silver; */ background-color: #141414; color: gray;");
    songInfo->show();

    timecode = new QLabel (this);
    timecode->raise();
    timecode->setGeometry(0, 0, 0, 0);
    timecode->setAlignment(Qt::AlignCenter);
    timecode->setMouseTracking(true);
    timecode->installEventFilter(this);
    timecode->setStyleSheet("font-size: 10px; border: 1px solid silver; background-color: #141414; color: silver;");

    playlistWidget = new QListWidget (this);

    QScrollBar *vbar = playlistWidget->verticalScrollBar();
    vbar->setStyle( new QCommonStyle );
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #141414; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar *hbar = playlistWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #141414; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    this->playlistWidget->setMouseTracking(true);
    playlistWidget->setGeometry(15, 400, 570, 160);
    playlistWidget->lower();
    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; font-size: 14px; /*border: 1px solid silver; */ border-radius: 5px; background-color: #181818; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");
    playlistWidget->show();

    playlistsWidget = new QListWidget (this);
    playlistsWidget->setGeometry (600, 400, 185, 160);
    playlistsWidget->lower();
    playlistsWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; font-size: 14px; /*border: 1px solid silver; */ border-radius: 5px; background-color: #181818; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");

    vbar = playlistsWidget->verticalScrollBar();
    vbar->setStyle( new QCommonStyle );
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #141414; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    hbar = playlistsWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #141414; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    playlistsWidget->show();

    QPushButton * addSongBtn = new QPushButton (this);
    addSongBtn->setFont(fontAwesome);
    addSongBtn->setGeometry(15, 570, 20, 20);
    addSongBtn->setText("\uf067");
    addSongBtn->setCursor(Qt::PointingHandCursor);
    addSongBtn->setStyleSheet("/*border-radius: 5px;*/ font-size: 14px; border: 0px solid silver; background-color: #141414; color: silver;");
    addSongBtn->show();

    QMenu * songAddingMenu = new QMenu(this);
    songAddingMenu->setFont(fontAwesome);
    songAddingMenu->setGeometry(15, 530, 120, 40);
    songAddingMenu->setStyleSheet("/*border-radius: 5px;*/ font-size: 12px; border: 0px solid silver; background-color: #141414; color: silver;");

    QAction * addSongAct = new QAction(songAddingMenu);
    addSongAct->setText("\uf067 Add song");
    songAddingMenu->addAction(addSongAct);

    songAddingMenu->addSeparator();

    QAction * addFolderAct = new QAction (songAddingMenu);
    addFolderAct->setText("\uf07c Add Folder");
    songAddingMenu->addAction(addFolderAct);

    QPushButton * removeSongBtn = new QPushButton (this);
    removeSongBtn->setFont(fontAwesome);
    removeSongBtn->setGeometry(45, 570, 20, 20);
    removeSongBtn->setText("\uf068");
    removeSongBtn->setCursor(Qt::PointingHandCursor);
    removeSongBtn->setStyleSheet("/*border-radius: 5px; */ font-size: 14px; border: 0px solid silver; background-color: #141414; color: silver;");
    removeSongBtn->show();

    searchSong = new QLineEdit(this);
    searchSong->setStyleSheet("QLineEdit { padding: 0px 5px; font-size: 12px; border: 0px solid silver; border-bottom: 1px solid silver; background-color: #181818; color: silver; }" \
                              "QLineEdit:focus { border-bottom: 1px solid " + QString::fromStdString(mainColorStr) + "; }");
    searchSong->setGeometry(335, 570, 250, 20);
    searchSong->setPlaceholderText("Search song (not working!)");
    searchSong->show();

    QPushButton * createPlaylistBtn = new QPushButton(this);
    createPlaylistBtn->setFont(fontAwesome);
    createPlaylistBtn->setGeometry(600, 570, 20, 20);
    createPlaylistBtn->setText("\uf067");
    createPlaylistBtn->setCursor(Qt::PointingHandCursor);
    createPlaylistBtn->setStyleSheet("/*border-radius: 5px;*/ font-size: 14px; border: 0px solid silver; background-color: #141414; color: silver;");
    createPlaylistBtn->show();

    QPushButton * removePlaylistBtn = new QPushButton (this);
    removePlaylistBtn->setFont(fontAwesome);
    removePlaylistBtn->setGeometry(630, 570, 20, 20);
    removePlaylistBtn->setText("\uf068");
    removePlaylistBtn->setCursor(Qt::PointingHandCursor);
    removePlaylistBtn->setStyleSheet("/*border-radius: 5px; */ font-size: 14px; border: 0px solid silver; background-color: #141414; color: silver;");
    removePlaylistBtn->show();

    songPosition = new QLabel(this);
    songPosition->setMouseTracking(true);
    songPosition->setText("00:00");
    songPosition->setGeometry(15, 363, 200, 20);
    songPosition->setAlignment(Qt::AlignLeft);
    songPosition->setStyleSheet("/* border: 1px solid silver; background-color: #141414; */ color: gray;");
    songPosition->show();

    songDuration = new QLabel(this);
    songDuration->setMouseTracking(true);
    songDuration->setText("00:00");
    songDuration->setGeometry(605, 363, 180, 20);
    songDuration->setAlignment(Qt::AlignRight);
    songDuration->setStyleSheet("/* border: 1px solid silver; background-color: #141414; */ color: gray;");
    songDuration->show();

    volumeSlider = new QSlider (Qt::Horizontal, this);
    volumeSlider->setMaximum(100);
    volumeSlider->setMinimum(0);
    volumeSlider->setValue(100);
    volumeSlider->setCursor(Qt::PointingHandCursor);
    volumeSlider->setGeometry(633, 301, 150, 20);
    volumeSlider->setStyleSheet("QSlider::groove:horizontal {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #181818;"\
                                    "margin: 7px 0;"\
                                "}" \
                               "QSlider::handle:horizontal {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "width: 5px; " \
                                    "margin: -5px -2px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ " \
                                    "border-radius: 20px; "\
                                "}"
                                "QSlider::sub-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}");

    volumeSlider->show();

    connect (addSongBtn, &QPushButton::clicked, [=](){
        songAddingMenu->popup(this->mapToGlobal(QPoint(15, 530)));
    });
    connect (addSongAct, SIGNAL(triggered()), this, SLOT(openFile()));
    connect (addFolderAct, SIGNAL(triggered()), this, SLOT(openFolder()));

    connect (playlistWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(setActive(QListWidgetItem *)));
    connect (playlistsWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(changeCurrentPlaylist(QListWidgetItem *)));

    connect (menuBtn, SIGNAL(clicked()), this, SLOT(settings()));

    connect (removeSongBtn, SIGNAL(clicked()), this, SLOT(removeFile()));
    connect (createPlaylistBtn, SIGNAL(clicked()), this, SLOT(createPlaylist()));
    connect (removePlaylistBtn, SIGNAL(clicked()), this, SLOT(removePlaylist()));

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
    connect (metronomeBtn, SIGNAL(clicked()), this, SLOT(metronome()));

    connect (volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));

    this->setMouseTracking(true);
    this->centralWidget()->setMouseTracking(true);

    drawPlaylist();
    drawAllPlaylists();
}

MainWindow::~MainWindow()
{
    playlists[currentPlaylistName] = playlist;
    XMLreader->writePlaylists(playlists);
    delete ui;
}
bool MainWindow::openFile ()
{
    HWND win = NULL;
    wchar_t file[MAX_PATH] = L"";

    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = win;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFile = file;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    ofn.lpstrTitle = L"Select a file to play";
    ofn.lpstrFilter = L"Audio Files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.umx;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif)\0All files\0*.*\0\0";

    if (!GetOpenFileName(&ofn)) return false;
    else {
        // Changing file format to Qt format
        for (int i = 0; i < MAX_PATH; i++)
        {
            if  (file[i] == L'\\') file[i] = L'/';
        }

        Song temp(file);

        // Checking if file already exist
        if (count(playlist.begin(), playlist.end(), temp) != 0)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Add file");
            msgBox.setText("File already exists in playlist!");
            msgBox.setStyleSheet("background-color: #141414; color: silver;");
            msgBox.exec();
            return true;
        }

        temp.setName(file);
        temp.setNameFromPath();
        wcout << temp.path << endl;

        int iterIndex = current - playlist.begin();

        playlist.push_back(temp);
        cout << playlist.size() << endl;

        current = playlist.begin() + iterIndex;

        drawPlaylist();
        if (playlist.size() == 1)
            setActive (0);
    }

    return true;
}
void MainWindow::removeFile() {
    int index = playlistWidget->currentItem()->data(Qt::UserRole).toInt();
    // cout << index << endl;

    if (index == -1) return;

    if (index == current - playlist.begin())
    {
        BASS_ChannelStop(channel);
        channel = NULL;
        songTitle->setText("");
        if (!paused) pause();
        clearPrerenderedFft();
        songPosition->setText("00:00");
        songDuration->setText("00:00");
    }

    playlist.erase(playlist.begin() + index);
    drawPlaylist();
    if (index == playlist.size()) index--;
    playlistWidget->setCurrentRow(index);
}
bool MainWindow::openFolder () {
    QString folder = QFileDialog::getExistingDirectory(0, ("Select Folder with Songs"), QDir::homePath());

    if (folder.toStdString() == "")
        return false;

    QDir directory(folder);
    QStringList musicFiles = directory.entryList(QStringList() << "*.mp3" << "*.mo3" << "*.wav" << "*.mp2" << "*.mp1",QDir::Files|QDir::Readable);

    int duplicatesCount = 0;
    for (int i = 0; i < musicFiles.length(); i++)
    {
        wchar_t tmp[MAX_PATH];
        QString fullpath = folder + "/" + musicFiles[i];
        wcscpy(tmp, fullpath.toStdWString().c_str());

        Song temp(tmp);

        if (count(playlist.begin(), playlist.end(), temp) != 0)
        {
            duplicatesCount++;
            continue;
        }
        temp.setName(musicFiles[i].toStdWString());
        temp.setNameFromPath();
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

    msgBox.setStyleSheet("background-color: #141414; color: silver;");
    msgBox.exec();

    drawPlaylist();

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
                msgBox.setStyleSheet("background-color: #141414; color: silver;");
                msgBox.exec();
            } else {
                playlists[name] = vector <Song>();
                break;
            }
        } else {
            break;
        }
    } while (true);

    drawAllPlaylists();
}
void MainWindow::removePlaylist () {
    QString playlistName = playlistsWidget->currentItem()->text();

    if (playlistName == "Default") return;

    if (playlists[playlistName].size() > 0) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText("Are you sure to delete \'" + playlistName + "\'playlist?");
        msgBox.setStyleSheet("background-color: #141414; color: silver;");
        msgBox.setInformativeText("Playlist contains " + QString::fromStdString(to_string(playlists[playlistName].size())) + " songs.");
        int ret = msgBox.exec();

        if (ret == QMessageBox::No)
            return;
    }

    bool currentSongInPlaylist = false;

    for (auto iter = playlist.begin(); iter != playlist.end(); iter++)
        if (wcscmp(iter->path, current->path) == 0) currentSongInPlaylist = true;

    if (currentSongInPlaylist)
    {
        BASS_ChannelStop(channel);
        channel = NULL;
        songTitle->setText("");
        if (!paused) pause();
        clearPrerenderedFft();
        songPosition->setText("00:00");
        songDuration->setText("00:00");
    }

    // Deleting playlist and setting first playlist as current
    playlists.erase(playlistName);
    currentPlaylistName = "Default";
    playlist = playlists[currentPlaylistName];

    drawPlaylist();
    drawAllPlaylists();
}
void MainWindow::drawAllPlaylists ()
{
    playlistsWidget->clear();

    int i = 0;
    for (auto &playlist : playlists)
    {
        QListWidgetItem *  playlistItem = new QListWidgetItem(playlistsWidget);

        playlistItem->setText(playlist.first);
        playlistsWidget->addItem(playlistItem);

        if (playlist.first == currentPlaylistName)
            playlistsWidget->setCurrentRow(i);

        i++;
    }
}
void MainWindow::drawPlaylist() {
    playlistWidget->clear();

    for (int i = 0; i < playlist.size(); i++)
    {
        QListWidgetItem *  songItem = new QListWidgetItem(playlistWidget);
        QString path = QString::fromStdWString(wstring(playlist[i].path));
        QString name = QString::fromStdWString(to_wstring(i + 1) + L". " + playlist[i].getName());

        songItem->setData(Qt::UserRole, i);
        songItem->setData(1, path);
        songItem->setText(name);

        playlistWidget->addItem(songItem);
   }
}
void MainWindow::setTitle () {
    wstring name = current->getName();
    if (name.length() > 32)
        name = name.substr(0, 32) + L"...";

    songTitle->setText(QString::fromStdWString(name));
}
void MainWindow::changeCurrentPlaylist (QListWidgetItem * item) {
    playlists[currentPlaylistName] = playlist;

    cout << item->text().toStdString() << endl;

    playlist = playlists[item->text()];
    currentPlaylistName = item->text();

    drawPlaylist();
    drawAllPlaylists();
}
void MainWindow::setActive(QListWidgetItem * item) {
    paused = true;
    pauseBtn->setText("\uf04c"); // Set symbol to pause

    setActive (item->data(Qt::UserRole).toInt());
}
void MainWindow::setActive (int index) {
    current = playlist.begin() + index;

    if (channel != NULL)
        BASS_ChannelStop(channel);

    channel = BASS_StreamCreateFile(FALSE, current->path, 0, 0, 0);
    prerenderFft();

    BASS_ChannelPlay(channel, false);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

    QString pos = QString::fromStdString(seconds2string(getPosition()));
    songPosition->setText(pos);
    QString len = QString::fromStdString(seconds2string(getDuration()));
    songDuration->setText(len);

    playlistWidget->setCurrentRow(index);

    setTitle();
}
void MainWindow::forward () {
    if (channel == NULL || playlist.size() == 1)
        return;

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlist.size();
        } while (songID == distance(playlist.begin(), current));

        current = playlist.begin() + songID;
    }
    else current++;

    if (distance (playlist.end(), current) >= 0) current = playlist.begin();

    playlistWidget->setCurrentRow(current - playlist.begin());

    BASS_ChannelStop(channel);
    channel = BASS_StreamCreateFile(FALSE, current->path, 0, 0, 0);

    prerenderFft();

    BASS_ChannelPlay(channel, true);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

    QString pos = QString::fromStdString(seconds2string(getPosition()));
    songPosition->setText(pos);
    QString len = QString::fromStdString(seconds2string(getDuration()));
    songDuration->setText(len);

    setTitle();
}
void MainWindow::backward () {
    if (channel == NULL || playlist.size() == 1)
        return;

    if (shuffle) {
        int songID;
        do {
            songID = rand() % playlist.size();
        } while (songID == distance(playlist.begin(), current));

        current = playlist.begin() + songID;
    }
    else current--;

    if (distance (playlist.begin(), current) < 0) current = playlist.end() - 1;

    BASS_ChannelStop(channel);
    channel = BASS_StreamCreateFile(FALSE, current->path, 0, 0, 0);

    playlistWidget->setCurrentRow(current - playlist.begin());

    prerenderFft();

    BASS_ChannelPlay(channel, true);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

    QString pos = QString::fromStdString(seconds2string(getPosition()));
    songPosition->setText(pos);
    QString len = QString::fromStdString(seconds2string(getDuration()));
    songDuration->setText(len);

    setTitle();
}
void MainWindow::pause ()
{
    if (channel == NULL) return;

    paused = !paused;

    if (paused) {
        BASS_ChannelPlay(channel, false);
        pauseBtn->setText("\uf04c"); // Set symbol to pause
    }
    else {
        BASS_ChannelPause(channel);
        pauseBtn->setText("\uf04b"); // Set symbol to play
    }
}
void MainWindow::changeVolume (int vol)
{
    cout << vol << endl;
    volume = vol / 100.0f;
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
}
void MainWindow::changeRepeat () {
    if (shuffle == true)
        changeShuffle();

    repeat = !repeat;

    if (repeat) repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
    else repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
}
void MainWindow::changeShuffle () {
    if (repeat == true)
        changeRepeat();

    shuffle = !shuffle;

    if (shuffle) shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
    else shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: silver;");
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
    if (channel != NULL) {
        QString pos = QString::fromStdString(seconds2string(getPosition()));
        songPosition->setText(pos);

        if (getPosition() >= getDuration())
        {
            if (repeat) {
                BASS_ChannelStop(channel);
                channel = BASS_StreamCreateFile(FALSE, current->path, 0, 0, 0);
                BASS_ChannelPlay(channel, true);
                BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
            }
            else if (shuffle && playlist.size() > 1) {
                int songID;
                do {
                    songID = rand() % playlist.size();
                } while (songID == distance(playlist.begin(), current));

                current = playlist.begin() + songID;

                playlistWidget->setCurrentRow(current - playlist.begin());

                BASS_ChannelStop(channel);
                channel = BASS_StreamCreateFile(FALSE, current->path, 0, 0, 0);

                prerenderFft();

                BASS_ChannelPlay(channel, true);
                BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

                QString pos = QString::fromStdString(seconds2string(getPosition()));
                songPosition->setText(pos);
                QString len = QString::fromStdString(seconds2string(getDuration()));
                songDuration->setText(len);

                setTitle();
            } else {
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

    // Spectrum background
    QColor color = QColor(20, 20, 20);

    painter.setBrush(QBrush(color, Qt::SolidPattern));
    painter.setPen(QPen(Qt::transparent, 1, Qt::SolidLine, Qt::FlatCap));

    painter.drawRect (50, 350, 700, 40);

    if (!cover.isNull()) {
        QBrush brush(cover);
        QTransform transform;
        transform.scale(0.3, 0.3);
        transform.translate(85, 200);
        brush.setTransform(transform);
        painter.setBrush(brush);
        painter.drawRoundedRect(325, 60, 150, 150, 5, 5);
    }

    color = mainColor;

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
        cout << max << " ";

        BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, i), BASS_POS_BYTE);
    }

    BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, 0), BASS_POS_BYTE);
    changeVolume(vol * 100);
}

void MainWindow::mouseMoveEvent (QMouseEvent * event) {
    float mouseX = event->pos().x();
    float mouseY = event->pos().y();

    // cout << mouseX << " " << mouseY << endl;

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

    if (mouseX >= 15 && mouseX <= 575 && mouseY >= 400 && mouseY <= 560)
    {
        playlistWidget->raise();
    }
    else if (mouseX >= 600 && mouseX <= 785 && mouseY >= 400 && mouseY <= 560)
    {
        playlistsWidget->raise();
    } else {
        playlistWidget->lower();
        playlistsWidget->lower();
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
            BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, ((mouseX - 50) / (700.0f)) * getDuration()), BASS_POS_BYTE);

        }
        else if (event->button() == Qt::RightButton) {
            liveSpec = !liveSpec;
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
    settingsWin->mainColor = &mainColor;
    settingsWin->mainColorStr = &mainColorStr;

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
    closeBtn->setStyleSheet("font-size: 24px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
    volumeSlider->setStyleSheet("QSlider::groove:horizontal {" \
                                    "border: 1px solid #999999; " \
                                    "border-radius: 20px;" \
                                    "background: #181818;"\
                                    "margin: 7px 0;"\
                                "}" \
                               "QSlider::handle:horizontal {" \
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "\
                                    "border: 1px solid #5c5c5c; "\
                                    "width: 5px; " \
                                    "margin: -5px -2px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ " \
                                    "border-radius: 20px; "\
                                "}"
                                "QSlider::sub-page:horizontal {" \
                                    "border-radius: 20px;" \
                                    "margin: 7px 0;" \
                                    "background: " + tr(mainColorStr.c_str()) + "; " \
                                "}");
    playlistWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; font-size: 14px; /*border: 1px solid silver; */ border-radius: 5px; background-color: #181818; color: silver; }" \
                                  "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                  "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");

    playlistsWidget->setStyleSheet("QListWidget { outline: 0; padding: 5px; font-size: 14px; /*border: 1px solid silver; */ border-radius: 5px; background-color: #181818; color: silver; }" \
                                   "QListWidget::item { outline: none; color: silver; border: 0px solid black; background: rgba(0, 0, 0, 0); }" \
                                   "QListWidget::item:selected { outline: none; border: 0px solid black; color: " + tr(mainColorStr.c_str()) + "; }");


    QScrollBar *vbar = playlistWidget->verticalScrollBar();
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #141414; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    QScrollBar *hbar = playlistWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #141414; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    vbar = playlistsWidget->verticalScrollBar();
    vbar->setStyle( new QCommonStyle );
    vbar->setStyleSheet("QScrollBar:vertical { outline: 0; border-radius: 20px; border: 0px solid black; width: 5px; background: #141414; }" \
                        "QScrollBar::add-line:vertical { height: 0; }" \
                        "QScrollBar::sub-line:vertical { height: 0; }" \
                        "QScrollBar::handle:vertical { border-radius: 20px; width: 5px; background: gray; }" \
                        "QScrollBar::handle:vertical:hover { border-radius: 20px; width: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { height: 0px; }");

    hbar = playlistsWidget->horizontalScrollBar();
    hbar->setStyle( new QCommonStyle );
    hbar->setStyleSheet("QScrollBar:horizontal { outline: 0; border-radius: 20px; border: 0px solid black; height: 5px; background: #141414; }" \
                        "QScrollBar::add-line:horizontal { height: 0; }" \
                        "QScrollBar::sub-line:horizontal { height: 0; }" \
                        "QScrollBar::handle:horizontal { border-radius: 20px; height: 5px; background: gray; }" \
                        "QScrollBar::handle:horizontal:hover { border-radius: 20px; height: 5px; background: " + tr(mainColorStr.c_str()) + "; }" \
                        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:vertical { height: 0px; }");

    searchSong->setStyleSheet("QLineEdit { padding: 0px 5px; font-size: 12px; border: 0px solid silver; border-bottom: 1px solid silver; background-color: #181818; color: silver; }" \
                              "QLineEdit:focus { border-bottom: 1px solid " + QString::fromStdString(mainColorStr) + "; }");

    if (shuffle) shuffleBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
    if (repeat) repeatBtn->setStyleSheet("font-size: 14px; margin-top: 10px; border: 0px solid silver; background-color: #141414; color: " + tr(mainColorStr.c_str()) + ";");
}

void MainWindow::colorChange()
{
    if (!colorChanging) return;

    const int speed = 20;

    static int dr = 0;
    static int dg = 0;
    static int db = 0;

    if (mainColor == nextColor) {
        nextColor = settingsWin->colors[rand() % 7];

        dr = (mainColor.red() - nextColor.red()) / speed;
        dg = (mainColor.green() - nextColor.green()) / speed;
        db = (mainColor.blue() - nextColor.blue()) / speed;

    } else {
        if (dr < 0) {
            dr++;
            mainColor.setRed(mainColor.red() + speed);
        }
        if (dr > 0) {
            dr--;
            mainColor.setRed(mainColor.red() - speed);
        }

        if (dg < 0) {
            dg++;
            mainColor.setGreen(mainColor.green() + speed);
        }
        if (dg > 0) {
            dg--;
            mainColor.setGreen(mainColor.green() - speed);
        }

        if (db < 0) {
            db++;
            mainColor.setBlue(mainColor.blue() + speed);
        }
        if (db > 0) {
            db--;
            mainColor.setBlue(mainColor.blue() - speed);
        }

        if (dr == 0) mainColor.setRed(nextColor.red());
        if (dg == 0) mainColor.setGreen(nextColor.green());
        if (db == 0) mainColor.setBlue(nextColor.blue());

        mainColorStr = settingsWin->qcolorToStr(mainColor);
    }

    reloadStyles ();
}
