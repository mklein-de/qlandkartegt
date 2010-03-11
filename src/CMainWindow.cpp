/**********************************************************************************************
    Copyright (C) 2008-2009 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMainWindow.h"
#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CCopyright.h"
#include "CResources.h"
#include "CMapDB.h"
#include "CWptDB.h"
#include "CTrackDB.h"
#include "CSearchDB.h"
#include "CDiaryDB.h"
#include "CRouteDB.h"
#include "CDlgConfig.h"
#include "CQlb.h"
#include "CGpx.h"
#include "tcxreader.h"
#include "CTabWidget.h"
#include "printpreview.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CActions.h"
#include "CMenus.h"
#include "IDevice.h"
#include "CDlgScreenshot.h"
#include "CUndoStack.h"
#include "WptIcons.h"
#include "CAppOpts.h"
#include "CMap3D.h"

#include <QtGui>
#ifdef WIN32
#include <io.h>
#endif

CMainWindow * theMainWindow = 0;

CMainWindow::CMainWindow()
: modified(false)
{
    theMainWindow = this;
    groupProvidedMenu = 0;
    setupMenu = 0;
    setObjectName("MainWidget");
    setWindowTitle("QLandkarte GT");
    setWindowIcon(QIcon(":/icons/iconGlobe16x16.png"));
    setAcceptDrops(true);

    initWptIcons();

    resources = new CResources(this);

    // setup splitter views
    mainSplitter = new QSplitter(Qt::Horizontal,this);
    setCentralWidget(mainSplitter);

    leftSplitter = new QSplitter(Qt::Vertical,this);
    mainSplitter->addWidget(leftSplitter);

    rightSplitter = new QSplitter(Qt::Vertical,this);
    mainSplitter->addWidget(rightSplitter);

    setCentralWidget(mainSplitter);

    canvasTab = new CTabWidget(this);
    rightSplitter->addWidget(canvasTab);

    canvas = new CCanvas(this);
    canvasTab->addTab(canvas,tr("Map"));

    actionGroupProvider = new CMenus(this);

    megaMenu = new CMegaMenu(canvas);
    leftSplitter->addWidget(megaMenu);

    connect(leftSplitter, SIGNAL(splitterMoved(int, int)), megaMenu, SLOT(slotSplitterMoved(int, int)));

    CActions *actions = actionGroupProvider->getActions();
    canvas->addAction(actions->getAction("aZoomIn"));
    canvas->addAction(actions->getAction("aZoomOut"));
    canvas->addAction(actions->getAction("aMoveLeft"));
    canvas->addAction(actions->getAction("aMoveRight"));
    canvas->addAction(actions->getAction("aMoveUp"));
    canvas->addAction(actions->getAction("aMoveDown"));
    addAction(actions->getAction("aRedo"));
    addAction(actions->getAction("aUndo"));

    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToMap");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToWpt");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToTrack");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToRoute");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToLiveLog");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToOverlay");
    actionGroupProvider->addAction(CMenus::MainMenu, "aSwitchToMainMore");
    actionGroupProvider->addAction(CMenus::MainMenu, "aClearAll");
    actionGroupProvider->addAction(CMenus::MainMenu, "aUploadAll");
    actionGroupProvider->addAction(CMenus::MainMenu, "aDownloadAll");

    actionGroupProvider->addAction(CMenus::MapMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::MapMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::MapMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::MapMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::MapMenu, "aSelectArea");
    actionGroupProvider->addAction(CMenus::MapMenu, "aEditMap");
    actionGroupProvider->addAction(CMenus::MapMenu, "aSearchMap");
#ifdef PLOT_3D
    actionGroupProvider->addAction(CMenus::MapMenu, "aSwitchToMap3D");
#endif
    actionGroupProvider->addAction(CMenus::MapMenu, "aUploadMap");


#ifdef PLOT_3D
    actionGroupProvider->addAction(CMenus::Map3DMenu, "aCloseMap3D");
    actionGroupProvider->addAction(CMenus::Map3DMenu, "aMap3DMode");
    actionGroupProvider->addAction(CMenus::Map3DMenu, "aMap3DFPVMode");
    actionGroupProvider->addAction(CMenus::Map3DMenu, "aMap3DLighting");
    actionGroupProvider->addAction(CMenus::Map3DMenu, "aMap3DTrackMode");
#endif

    actionGroupProvider->addAction(CMenus::WptMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::WptMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::WptMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::WptMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::WptMenu, "aNewWpt");
    actionGroupProvider->addAction(CMenus::WptMenu, "aEditWpt");
    actionGroupProvider->addAction(CMenus::WptMenu, "aMoveWpt");
#ifdef HAS_EXIF
    actionGroupProvider->addAction(CMenus::WptMenu, "aImageWpt");
#endif
    actionGroupProvider->addAction(CMenus::WptMenu, "aUploadWpt");
    actionGroupProvider->addAction(CMenus::WptMenu, "aDownloadWpt");

    actionGroupProvider->addAction(CMenus::TrackMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aCombineTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aEditTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aCutTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aSelTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aUploadTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aDownloadTrack");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aCopyToClipboard");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aPasteFromClipboard");
    actionGroupProvider->addAction(CMenus::TrackMenu, "aDeleteTrackSelection");

    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aLiveLog");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aLockMap");
    actionGroupProvider->addAction(CMenus::LiveLogMenu, "aAddWpt");

    actionGroupProvider->addAction(CMenus::OverlayMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aText");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aTextBox");
    actionGroupProvider->addAction(CMenus::OverlayMenu, "aDistance");

    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aDiary");
//    actionGroupProvider->addAction(CMenus::MainMoreMenu, "aWorldBasemap");

    actionGroupProvider->addAction(CMenus::RouteMenu, "aSwitchToMain");
    actionGroupProvider->addAction(CMenus::RouteMenu, "aMoveArea");
    actionGroupProvider->addAction(CMenus::RouteMenu, "aZoomArea");
    actionGroupProvider->addAction(CMenus::RouteMenu, "aCenterMap");
    actionGroupProvider->addAction(CMenus::RouteMenu, "aUploadRoute");
    actionGroupProvider->addAction(CMenus::RouteMenu, "aDownloadRoute");

    connect(actionGroupProvider, SIGNAL(stateChanged()),megaMenu , SLOT(switchState()));

    switchState();

    QWidget * wtmp      = new QWidget(this);
    QVBoxLayout * ltmp  = new QVBoxLayout(wtmp);
    wtmp->setMinimumHeight(1);
    wtmp->setLayout(ltmp);
    leftSplitter->addWidget(wtmp);

    summary = new QLabel(wtmp);
    summary->setWordWrap(true);
    summary->setAlignment(Qt::AlignJustify|Qt::AlignTop);
    //
    ltmp->addWidget(summary);

    ltmp->addWidget(new QLabel(tr("<b>GPS Device:</b>"), wtmp));

    comboDevice = new QComboBox(wtmp);
    connect(resources, SIGNAL(sigDeviceChanged()), this, SLOT(slotDeviceChanged()));
    connect(comboDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDeviceChanged(int)));
    slotDeviceChanged();

    ltmp->addWidget(comboDevice);

    tabbar = new QTabWidget(canvas);
    leftSplitter->addWidget(tabbar);

    leftSplitter->setCollapsible(0, true);
    leftSplitter->setCollapsible(1, true);
    leftSplitter->setCollapsible(2, false);

    statusCoord = new QLabel(this);
    statusBar()->insertPermanentWidget(1,statusCoord);

    QSettings cfg;
    pathData = cfg.value("path/data","./").toString();

    mapdb       = new CMapDB(tabbar, this);
    wptdb       = new CWptDB(tabbar, this);
    trackdb     = new CTrackDB(tabbar, this);
    routedb     = new CRouteDB(tabbar, this);
    diarydb     = new CDiaryDB(canvasTab, this);
    searchdb    = new CSearchDB(tabbar, this);
    livelogdb   = new CLiveLogDB(tabbar, this);
    overlaydb   = new COverlayDB(tabbar, this);

    connect(searchdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(wptdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(trackdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(livelogdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(overlaydb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(slotToolBoxChanged(int)));
    connect(routedb, SIGNAL(sigChanged()), this, SLOT(update()));
    connect(mapdb, SIGNAL(sigChanged()), this, SLOT(update()));

    connect(mapdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(wptdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(trackdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(diarydb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(overlaydb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(routedb, SIGNAL(sigModified()), this, SLOT(slotModified()));

    searchdb->gainFocus();

    // restore last session position and size of mainWidget
    {
        if ( cfg.contains("mainWidget/geometry"))
        {
            QRect r = cfg.value("mainWidget/geometry").toRect();
            //qDebug() << r << QDesktopWidget().screenGeometry();
            if (r.isValid() && QDesktopWidget().screenGeometry().intersects(r))
                setGeometry(r);
            else
                showMaximized();
        }
    }

    // restore last session settings
    QList<int> sizes = mainSplitter->sizes();
    sizes[0] = (int)(mainSplitter->width() * 0.1);
    sizes[1] = (int)(mainSplitter->width() * 0.9);
    mainSplitter->setSizes(sizes);
    sizes = leftSplitter->sizes();
    sizes[0] = (int)(mainSplitter->height() * 0.3);
    sizes[1] = (int)(mainSplitter->height() * 0.3);
    sizes[2] = (int)(mainSplitter->height() * 0.4);
    leftSplitter->setSizes(sizes);

    if( cfg.contains("mainWidget/mainSplitter") )
    {
        mainSplitter->restoreState(cfg.value("mainWidget/mainSplitter",mainSplitter->saveState()).toByteArray());
    }
    if( cfg.contains("mainWidget/leftSplitter") )
    {
        leftSplitter->restoreState(cfg.value("mainWidget/leftSplitter",leftSplitter->saveState()).toByteArray());
    }

    sizes.clear();
    sizes << 200 << 50 << 50;
    rightSplitter->setSizes(sizes);

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));
    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));
    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));
    connect(&CDiaryDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));
    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDataChanged()));

    slotDataChanged();

    connect(summary, SIGNAL(linkActivated(const QString&)),this,SLOT(slotOpenLink(const QString&)));

    canvas->setMouseMode(CCanvas::eMouseMoveArea);
    megaMenu->switchByKeyWord("Main");

    if (qlOpts->monitor != -1)
    {
        snRead = new QSocketNotifier(qlOpts->monitor, QSocketNotifier::Read, this);
        connect(snRead, SIGNAL(activated(int)), this, SLOT(slotReloadArgs()));
    }

    mostRecent = cfg.value("geodata/mostRecent",QStringList()).toStringList();


    foreach(QString arg, qlOpts->arguments)
    {
        loadData(arg, QString());
    }

    modified = false;
    setTitleBar();

    setupMenuBar();
    connect(actionGroupProvider, SIGNAL(stateChanged()), this, SLOT(switchState()));

    megaMenu->slotSplitterMoved(leftSplitter->sizes()[0], 1);

}


void CMainWindow::slotReloadArgs()
{
    char c;
    int i;
#ifdef WIN32
                                 // read char
    i = _read(qlOpts->monitor, &c, 1);
#else
                                 // read char
    i = read(qlOpts->monitor, &c, 1);
#endif

    if(i != 1)
    {
        delete snRead;
        return;
    }

    CWptDB::self().clear();
    CTrackDB::self().clear();

    foreach(QString arg, qlOpts->arguments)
    {
        loadData(arg, QString());
    }
}


CMainWindow::~CMainWindow()
{
    QSettings cfg;
    cfg.setValue("mainWidget/mainSplitter",mainSplitter->saveState());
    cfg.setValue("mainWidget/leftSplitter",leftSplitter->saveState());
    cfg.setValue("path/data",pathData);
    cfg.setValue("mainWidget/geometry", geometry());
    cfg.setValue("geodata/mostRecent", mostRecent);
    canvas = 0;
}


void CMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();
        QFileInfo fi(urls[0].path());

        if(fi.suffix() == "gpx")
        {
            event->acceptProposedAction();
        }

    }
}


void CMainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QUrl url;

    foreach(url, urls)
    {
        QString filename = url.path();
        QString filter;
        loadData(filename, filter);
    }

    event->acceptProposedAction();
}


void CMainWindow::setTitleBar()
{
    if(wksFile.isEmpty())
    {
        setWindowTitle(QString("QLandkarte GT") + (modified ? " *" : ""));
    }
    else
    {
        setWindowTitle(QString("QLandkarte GT - ") + QFileInfo(wksFile).fileName() + (modified ? " *" : ""));
    }
}


void CMainWindow::clearAll()
{
    QMessageBox::StandardButton res = QMessageBox::question(0, tr("Clear all..."), tr("This will erase all project data like waypoints and tracks."), QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok);

    if(res == QMessageBox::Ok)
    {
        CSearchDB::self().clear();
        CMapDB::self().clear();
        CWptDB::self().clear();
        CTrackDB::self().clear();
        CDiaryDB::self().clear();
        COverlayDB::self().clear();
        CRouteDB::self().clear();
        clear();
    }
}


void CMainWindow::clear()
{
    modified = false;
    wksFile.clear();
    setTitleBar();
}


void CMainWindow::setTempWidget(QWidget * w)
{
    rightSplitter->addWidget(w);
}


void CMainWindow::setPositionInfo(const QString& info)
{
    statusCoord->setText(info);
}


void CMainWindow::switchState()
{
    if (groupProvidedMenu)
    {
        groupProvidedMenu->clear();
        actionGroupProvider->addActionsToMenu(groupProvidedMenu);
    }
}


#if defined(Q_WS_MAC)
// do not translate on the Mac, so the item is shifted
#  define tr_nomac(x) (x)
#else
#  define tr_nomac(x) tr(x)
#endif

void CMainWindow::setupMenuBar()
{
    QMenu * menu;

    menuMostRecent = new QMenu(tr("Load most recent..."),this);
    menuMostRecent->setIcon(QIcon(":/icons/iconFileLoad16x16.png"));

    QString recent;
    foreach(recent, mostRecent)
    {
        menuMostRecent->addAction(recent, this, SLOT(slotLoadRecent()));
    }

    menu = new QMenu(this);
    menu->setTitle(tr("&File"));
    menu->addAction(QIcon(":/icons/iconOpenMap16x16.png"),tr("Load Map"),this,SLOT(slotLoadMapSet()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconFileLoad16x16.png"),tr("Load Geo Data"),this,SLOT(slotLoadData()), Qt::CTRL + Qt::Key_L);
    menu->addAction(QIcon(":/icons/iconFileSave16x16.png"),tr("Save Geo Data"),this,SLOT(slotSaveData()), Qt::CTRL + Qt::Key_S);
    menu->addAction(QIcon(":/icons/iconFileExport16x16.png"),tr("Export Geo Data"),this,SLOT(slotExportData()), Qt::CTRL + Qt::Key_X);
    menu->addAction(QIcon(":/icons/iconFileAdd16x16.png"),tr("Add Geo Data"),this,SLOT(slotAddData()));
    menu->addMenu(menuMostRecent);
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconScreenshot16x16.png"),tr("Device Screenshot ..."),this,SLOT(slotScreenshot()));
    menu->addAction(QIcon(":/icons/iconRaster16x16.png"),tr("Save as image ..."),this,SLOT(slotSaveImage()));
    menu->addAction(QIcon(":/icons/iconPrint16x16.png"),tr("Print Map ..."),this,SLOT(slotPrint()), Qt::CTRL + Qt::Key_P);
    menu->addAction(QIcon(":/icons/iconPrint16x16.png"),tr("Print Diary ..."),this,SLOT(slotPrintPreview()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconExit16x16.png"),tr_nomac("Exit"),this,SLOT(close()));
    menuBar()->addMenu(menu);


    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::MapMenu);
    menu->setTitle(tr("&Map"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::WptMenu);
    menu->setTitle(tr("&Waypoint"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::TrackMenu);
    menu->setTitle(tr("&Track"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::RouteMenu);
    menu->setTitle(tr("&Route"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::LiveLogMenu);
    menu->setTitle(tr("&Live Log"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::OverlayMenu);
    menu->setTitle(tr("&Overlay"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    actionGroupProvider->addActionsToMenu(menu,CMenus::MenuBarMenu,CMenus::MainMoreMenu);
    menu->setTitle(tr("Mor&e"));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&Setup"));
#if defined(Q_WS_MAC)
    menu->addAction(QIcon(":/icons/iconConfig16x16.png"),"&Preferences",this,SLOT(slotConfig()));
#else
    menu->addAction(QIcon(":/icons/iconConfig16x16.png"),tr("&General"),this,SLOT(slotConfig()));
#endif
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr_nomac("&Help"));
    menu->addAction(QIcon(":/icons/iconGlobe16x16.png"),tr_nomac("About &QLandkarte GT"),this,SLOT(slotCopyright()));
    menuBar()->addMenu(menu);
}


void CMainWindow::closeEvent(QCloseEvent * e)
{
    if(!modified)
    {
        e->accept();
        return;
    }

    if (maybeSave())
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}


void CMainWindow::slotLoadMapSet()
{
    QSettings cfg;

    QString filter   = cfg.value("maps/filter","").toString();
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select map...")
        ,CResources::self().pathMaps
    #ifdef WMS_CLIENT
        ,"All (*.*);;Map Collection (*.qmap);;Garmin (*.tdb);;WMS (*.xml)"
    #else
        ,"All (*.*);;Map Collection (*.qmap);;Garmin (*.tdb)"
    #endif
        , &filter
        , QFileDialog::DontUseNativeDialog
        );
    if(filename.isEmpty()) return;

    CResources::self().pathMaps = QFileInfo(filename).absolutePath();
    CMapDB::self().openMap(filename, false, *canvas);

    cfg.setValue("maps/filter",filter);
}


void CMainWindow::slotCopyright()
{
    CCopyright dlg;
    dlg.exec();
}


void CMainWindow::slotToolBoxChanged(int idx)
{
    QString key = tabbar->widget(idx)->objectName();
    megaMenu->switchByKeyWord(key);
}


void CMainWindow::slotConfig()
{
    CDlgConfig dlg(this);
    dlg.exec();
}


void CMainWindow::slotLoadData()
{

    bool haveGPSBabel = QProcess::execute("gpsbabel -V") == 0;
    QString formats;
    if(haveGPSBabel)
    {
        formats = "All supported files (*.qlb *.gpx *.tcx *.loc *.gdb *.kml);;QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx);;Geocaching.com - EasyGPS (*.loc);;Mapsource (*.gdb);;Google Earth (*.kml)";
    }
    else
    {
        formats = "All supported files (*.qlb *.gpx *.tcx);;QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx)";
    }

    QSettings cfg;
    QString filter   = cfg.value("geodata/filter","").toString();
    QStringList filenames = QFileDialog::getOpenFileNames( 0, tr("Select input files")
        ,pathData
        ,formats
        ,&filter
        , QFileDialog::DontUseNativeDialog
        );

    if(filenames.isEmpty()) return;

    if(modified)
    {
        if(!maybeSave())
        {
            return;
        }
    }

    CMapDB::self().clear();
    CWptDB::self().clear();
    CTrackDB::self().clear();
    CRouteDB::self().clear();
    CDiaryDB::self().clear();
    COverlayDB::self().clear();

    QString filename;
    foreach(filename, filenames)
    {
        loadData(filename, filter);
        addRecent(filename);
    }

    wksFile = filename;

    modified = false;
    setTitleBar();

    cfg.setValue("geodata/filter",filter);
}


void CMainWindow::slotAddData()
{
    bool haveGPSBabel = QProcess::execute("gpsbabel -V") == 0;
    QString formats;
    if(haveGPSBabel)
    {
        formats = "All supported files (*.qlb *.gpx *.tcx *.loc *.gdb *.kml);;QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx);;Geocaching.com - EasyGPS (*.loc);;Mapsource (*.gdb);;Google Earth (*.kml)";
    }
    else
    {
        formats = "All supported files (*.qlb *.gpx *.tcx);;QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx)";
    }

    QSettings cfg;
    int i;
    QString filename;
    QString filter   = cfg.value("geodata/filter","").toString();
    QStringList filenames = QFileDialog::getOpenFileNames( 0, tr("Select input files")
        ,pathData
        ,formats
        ,&filter
        , QFileDialog::DontUseNativeDialog
        );

    for (i = 0; i < filenames.size(); ++i)
    {
        filename = filenames.at(i);
        if(filename.isEmpty()) return;

        QString tmp = wksFile;
        loadData(filename, filter);
        addRecent(filename);

        wksFile = tmp;

        modified = true;
        setTitleBar();

        cfg.setValue("geodata/filter",filter);
    }

}


void CMainWindow::loadData(QString& filename, const QString& filter)
{
    QTemporaryFile tmpfile;
    bool loadGPXData = false;
    QFileInfo fileInfo(filename);
    QString ext = fileInfo.suffix().toUpper();

    pathData = fileInfo.absolutePath();

    try
    {
        if(ext == "QLB")
        {
            CQlb qlb(this);
            qlb.load(filename);
            CMapDB::self().loadQLB(qlb);
            CWptDB::self().loadQLB(qlb);
            CTrackDB::self().loadQLB(qlb);
            CRouteDB::self().loadQLB(qlb);
            CDiaryDB::self().loadQLB(qlb);
            COverlayDB::self().loadQLB(qlb);
        }
        else if(ext == "GPX")
        {
            CGpx gpx(this);
            gpx.load(filename);
            CMapDB::self().loadGPX(gpx);
            CWptDB::self().loadGPX(gpx);
            CTrackDB::self().loadGPX(gpx);
            CRouteDB::self().loadGPX(gpx);
            CDiaryDB::self().loadGPX(gpx);
            COverlayDB::self().loadGPX(gpx);
        }
        else if(ext == "TCX")
        {
            TcxReader tcxReader(this);
            if (!tcxReader.read(filename))
            {
                throw(tcxReader.errorString());
            }
            else
            {
                //emit CTrackDB::self().sigChanged(); //??
                QRectF r = CTrackDB::self().getBoundingRectF();
                if (!r.isNull ())
                {
                    CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
                }
            }
        }
        else if(ext == "LOC")
        {
            tmpfile.open();
            loadGPXData = convertData("geo", filename, "gpx", tmpfile.fileName());
            if (!loadGPXData)
            {
                QMessageBox::critical(0,tr("Convert error"),"Error in data conversion?",QMessageBox::Ok,QMessageBox::NoButton);
            }
            else
            {
                CGpx gpx(this);
                gpx.load(tmpfile.fileName());
                CMapDB::self().loadGPX(gpx);
                CWptDB::self().loadGPX(gpx);
                CTrackDB::self().loadGPX(gpx);
                CRouteDB::self().loadGPX(gpx);
                CDiaryDB::self().loadGPX(gpx);
                COverlayDB::self().loadGPX(gpx);
            }
        }
        else if(ext == "GDB")
        {
            tmpfile.open();
            loadGPXData = convertData("gdb", filename, "gpx", tmpfile.fileName());
            if (!loadGPXData)
            {
                QMessageBox::critical(0,tr("Convert error"),"Error in data conversion?",QMessageBox::Ok,QMessageBox::NoButton);
            }
            else
            {
                CGpx gpx(this);
                gpx.load(tmpfile.fileName());
                CMapDB::self().loadGPX(gpx);
                CWptDB::self().loadGPX(gpx);
                CTrackDB::self().loadGPX(gpx);
                CRouteDB::self().loadGPX(gpx);
                CDiaryDB::self().loadGPX(gpx);
                COverlayDB::self().loadGPX(gpx);
            }
        }
        else if(ext == "KML")
        {
            tmpfile.open();
            loadGPXData = convertData("kml", filename, "gpx", tmpfile.fileName());
            if (!loadGPXData)
            {
                QMessageBox::critical(0,tr("Convert error"),"Error in data conversion?",QMessageBox::Ok,QMessageBox::NoButton);
            }
            else
            {
                CGpx gpx(this);
                gpx.load(tmpfile.fileName());
                CMapDB::self().loadGPX(gpx);
                CWptDB::self().loadGPX(gpx);
                CTrackDB::self().loadGPX(gpx);
                CRouteDB::self().loadGPX(gpx);
                CDiaryDB::self().loadGPX(gpx);
                COverlayDB::self().loadGPX(gpx);
            }
        }
        wksFile = filename;
    }
    catch(const QString& msg)
    {
        wksFile.clear();
        QMessageBox:: critical(this,tr("Error"), msg, QMessageBox::Cancel, QMessageBox::Cancel);
    }
}


bool CMainWindow::convertData(const QString& inFormat, const QString& inFile, const QString& outFormat, const QString& outFile)
{
    QString program = "gpsbabel";
    QStringList arguments;
    arguments << "-i" << inFormat << "-f" << inFile << "-o" << outFormat << "-F" << outFile;

    QProcess *babelProcess = new QProcess(this);
    babelProcess->start(program, arguments);
    if (!babelProcess->waitForStarted())
        return false;

    if (!babelProcess->waitForFinished())
        return false;

    return babelProcess->exitCode() ==0;
}


bool CMainWindow::maybeSave()
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Save geo data?"),
        tr("The loaded data has been modified.\n"
        "Do you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard| QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
    {
        if(wksFile.isEmpty())
        {
            slotSaveData();
        }
        else
        {
            QSettings cfg;
            QString filter = cfg.value("geodata/filter","").toString();
            saveData(wksFile, filter);
        }
        return true;
    }
    else if (ret == QMessageBox::Cancel)
    {
        return false;
    }
    return true;
}


void CMainWindow::slotSaveData()
{
    QSettings cfg;

    QString filter =cfg.value("geodata/filter","").toString();

    QString filename = QFileDialog::getSaveFileName( 0, tr("Select output file")
        ,pathData
        ,"QLandkarte (*.qlb);;GPS Exchange (*.gpx)"
        ,&filter
        , QFileDialog::DontUseNativeDialog
        );

    if(filename.isEmpty()) return;

    cfg.setValue("geodata/filter",filter);
    saveData(filename, filter);
    addRecent(filename);
}


void CMainWindow::slotExportData()
{
    QString filename = QFileDialog::getSaveFileName( 0, tr("Select output file")
        ,pathData
        ,"GPS Exchange (*.gpx)"
        ,0
        , QFileDialog::DontUseNativeDialog
        );

    if(filename.isEmpty()) return;

    saveData(filename, "GPS Exchange (*.gpx)", true);
    addRecent(filename);
}


void CMainWindow::saveData(const QString& fn, const QString& filter, bool exportFlag)
{
    QString filename = fn;
    QString ext = filename.right(4);

    if(exportFlag || filter == "GPS Exchange (*.gpx)")
    {
        if(ext != ".gpx") filename += ".gpx";
        ext = "GPX";
    }
    else if(filter == "QLandkarte (*.qlb)")
    {
        if(ext != ".qlb") filename += ".qlb";
        ext = "QLB";
    }
    else
    {
        if (ext == ".gpx")
        {
            ext = "GPX";
        }
        else if (ext == ".qlb")
        {
            ext = "QLB";
        }
        else
        {
            filename += ".qlb";
            ext = "QLB";
        }
    }

    pathData = QFileInfo(filename).absolutePath();

    try
    {
        if(ext == "QLB")
        {
            CQlb qlb(this);
            CMapDB::self().saveQLB(qlb);
            CWptDB::self().saveQLB(qlb);
            CTrackDB::self().saveQLB(qlb);
            CRouteDB::self().saveQLB(qlb);
            CDiaryDB::self().saveQLB(qlb);
            COverlayDB::self().saveQLB(qlb);
            qlb.save(filename);
        }
        else if(ext == "GPX")
        {
            CGpx gpx(this, exportFlag);
            CMapDB::self().saveGPX(gpx);
            CWptDB::self().saveGPX(gpx);
            CTrackDB::self().saveGPX(gpx);
            CRouteDB::self().saveGPX(gpx);
            gpx.makeExtensions();
            CDiaryDB::self().saveGPX(gpx);
            COverlayDB::self().saveGPX(gpx);
            gpx.save(filename);
        }

        wksFile  = filename;
        modified = false;
    }
    catch(const QString& msg)
    {
        wksFile.clear();
        QMessageBox:: critical(this,tr("Error"), msg, QMessageBox::Cancel, QMessageBox::Cancel);
    }

    setTitleBar();
}


void CMainWindow::slotPrint()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Map"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    canvas->print(printer);
}


void CMainWindow::slotSaveImage()
{

    QString filter;
    QString filename = QFileDialog::getSaveFileName( 0, tr("Select output file")
        ,pathData
        ,"Bitmap (*.png *.jpg);;"
        ,&filter
        , QFileDialog::DontUseNativeDialog
        );

    if(filename.isEmpty()) return;

    CMap3D * map3d = qobject_cast<CMap3D*>(canvasTab->currentWidget());

    if(map3d)
    {
        map3d->slotSaveImage(filename);
    }
    else
    {
        QImage img(canvas->size(), QImage::Format_ARGB32);
        canvas->print(img);
        img.save(filename);
    }
}


void CMainWindow::slotModified()
{
    modified = true;
    setTitleBar();
}


void CMainWindow::slotPrintPreview()
{
    QTextEdit textEdit(this);
    textEdit.insertHtml(CDiaryDB::self().getDiary());
    PrintPreview *preview = new PrintPreview(textEdit.document(), this);
    preview->setWindowModality(Qt::WindowModal);
    preview->setAttribute(Qt::WA_DeleteOnClose);
    preview->show();
}


void CMainWindow::slotDataChanged()
{

    int c;
    QString str = tr("<div style='float: left;'><b>Project Summary (<a href='Clear'>clear</a> project):</b></div>");

    str += "<p>";
    c = CWptDB::self().count();
    if(c > 0)
    {
        if(c == 1)
        {
            str += tr("Currently there is %1 <a href='Waypoints'>waypoint</a>, ").arg(c);
        }
        else
        {
            str += tr("Currently there are %1 <a href='Waypoints'>waypoints</a>, ").arg(c);
        }
    }
    else
    {
        str += tr("There are no waypoints, ");
    }

    c = CTrackDB::self().count();
    if(c > 0)
    {
        if(c == 1)
        {
            str += tr(" %1 <a href='Tracks'>track</a>, ").arg(c);
        }
        else
        {
            str += tr(" %1 <a href='Tracks'>tracks</a>, ").arg(c);
        }
    }
    else
    {
        str += tr("no tracks, ");
    }

    c = CRouteDB::self().count();
    if(c > 0)
    {
        if(c == 1)
        {
            str += tr(" %1 <a href='Routes'>route</a> and ").arg(c);
        }
        else
        {
            str += tr(" %1 <a href='Routes'>routes</a> and ").arg(c);
        }
    }
    else
    {
        str += tr("no routes and ");
    }

    c = COverlayDB::self().count();
    if(c > 0)
    {
        if(c == 1)
        {
            str += tr(" %1 <a href='Overlay'>overlay</a>. ").arg(c);
        }
        else
        {
            str += tr(" %1 <a href='Overlay'>overlays</a>. ").arg(c);
        }
    }
    else
    {
        str += tr("no overlays. ");
    }

    c = CDiaryDB::self().count();
    if(c > 0)
    {
        str += tr("A <a href='Diary'>diary</a> is loaded.");
    }
    else
    {
        str += tr("The diary (<a href='Diary'>new</a>) is empty.");
    }

    str += "</p>";

    summary->setText(str);

}


void CMainWindow::slotOpenLink(const QString& link)
{
    if(link == "Diary")
    {
        CDiaryDB::self().openEditWidget();
    }
    else if(link == "Clear")
    {
        clearAll();
    }
    else
    {
        CMegaMenu::self().switchByKeyWord(link);
    }
}


void CMainWindow::slotCurrentDeviceChanged(int i)
{
    resources->m_devKey = comboDevice->itemData(i).toString();
}


void CMainWindow::slotDeviceChanged()
{
    QString devKey = resources->m_devKey;

    comboDevice->clear();
    comboDevice->addItem(tr(""),"");
    comboDevice->addItem(tr("QLandkarte M"), "QLandkarteM");
    comboDevice->addItem(resources->m_devType, "Garmin");
    comboDevice->addItem(tr("NMEA"), "NMEA");
    comboDevice->addItem(tr("Mikrokopter"), "Mikrokopter");

    resources->m_devKey = devKey;
    comboDevice->setCurrentIndex(comboDevice->findData(resources->m_devKey));
}


void CMainWindow::slotScreenshot()
{
    CDlgScreenshot dlg(this);
    dlg.exec();

}


void CMainWindow::slotLoadRecent()
{
    QAction * act = qobject_cast<QAction*>(sender());
    if(act)
    {
        QString filename = act->text();

        if(modified)
        {
            if(!maybeSave())
            {
                return;
            }
        }

        CMapDB::self().clear();
        CWptDB::self().clear();
        CTrackDB::self().clear();
        CRouteDB::self().clear();
        CDiaryDB::self().clear();
        COverlayDB::self().clear();

        loadData(filename,"");

        wksFile = filename;

        modified = false;
        setTitleBar();
    }
}


void CMainWindow::addRecent(const QString& filename)
{
    QString recent;
    foreach(recent, mostRecent)
    {
        if(recent == filename) return;
    }

    if(mostRecent.count() >= 10)
    {
        mostRecent.removeLast();
    }
    mostRecent.prepend(filename);

    menuMostRecent->clear();
    foreach(recent, mostRecent)
    {
        menuMostRecent->addAction(recent, this, SLOT(slotLoadRecent()));
    }

}
