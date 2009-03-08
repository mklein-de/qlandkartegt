/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
#include "CDlgConfig.h"
#include "CQlb.h"
#include "CGpx.h"
#include "tcxreader.h"
#include "CTabWidget.h"
#include "printpreview.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"

#include <QtGui>

CMainWindow * theMainWindow = 0;

CMainWindow::CMainWindow()
: modified(false)
{
    theMainWindow = this;

    setObjectName("MainWidget");
    setWindowTitle("QLandkarte GT");
    setWindowIcon(QIcon(":/icons/iconGlobe16x16.png"));

    resources = new CResources(this);

    setupMenuBar();

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

    megaMenu = new CMegaMenu(canvas);
    leftSplitter->addWidget(megaMenu);

    QWidget * wtmp      = new QWidget(this);
    QVBoxLayout * ltmp  = new QVBoxLayout(wtmp);
    wtmp->setLayout(ltmp);
    leftSplitter->addWidget(wtmp);

    summary = new QLabel(wtmp);
    summary->setWordWrap(true);
    summary->setAlignment(Qt::AlignJustify|Qt::AlignTop);
    ltmp->addWidget(summary);

    ltmp->addWidget(new QLabel(tr("<b>GPS Device:</b>"), wtmp));

    comboDevice = new QComboBox(wtmp);
    connect(resources, SIGNAL(sigDeviceChanged()), this, SLOT(slotDeviceChanged()));
    connect(comboDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDeviceChanged(int)));
    slotDeviceChanged();

    ltmp->addWidget(comboDevice);

    tabbar = new QTabWidget(canvas);
    leftSplitter->addWidget(tabbar);

    leftSplitter->setCollapsible(0, false);
    leftSplitter->setCollapsible(1, true);
    leftSplitter->setCollapsible(2, false);

    statusCoord = new QLabel(this);
    statusBar()->insertPermanentWidget(1,statusCoord);

    QSettings cfg;
    pathData = cfg.value("path/data","./").toString();

    mapdb       = new CMapDB(tabbar, this);
    wptdb       = new CWptDB(tabbar, this);
    trackdb     = new CTrackDB(tabbar, this);
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
    connect(mapdb, SIGNAL(sigChanged()), this, SLOT(update()));

    connect(mapdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(wptdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(trackdb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(diarydb, SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(overlaydb, SIGNAL(sigModified()), this, SLOT(slotModified()));

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

    if( cfg.contains("mainWidget/mainSplitter") ) {
        mainSplitter->restoreState(cfg.value("mainWidget/mainSplitter",mainSplitter->saveState()).toByteArray());
    }
    if( cfg.contains("mainWidget/leftSplitter") ) {
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

    slotDataChanged();

    connect(summary, SIGNAL(linkActivated(const QString&)),this,SLOT(slotOpenLink(const QString&)));

    canvas->setMouseMode(CCanvas::eMouseMoveArea);
    megaMenu->switchByKeyWord("Main");
}


CMainWindow::~CMainWindow()
{
    QSettings cfg;
    cfg.setValue("mainWidget/mainSplitter",mainSplitter->saveState());
    cfg.setValue("mainWidget/leftSplitter",leftSplitter->saveState());
    cfg.setValue("path/data",pathData);
    cfg.setValue("mainWidget/geometry", geometry());

    canvas = 0;
}


void CMainWindow::setTitleBar()
{
    if(wksFile.isEmpty()) {
        setWindowTitle(QString("QLandkarte GT") + (modified ? " *" : ""));
    }
    else {
        setWindowTitle(QString("QLandkarte GT - ") + QFileInfo(wksFile).fileName() + (modified ? " *" : ""));
    }
}


void CMainWindow::clearAll()
{
    QMessageBox::StandardButton res = QMessageBox::question(0, tr("Clear all..."), tr("This will erase all project data like waypoints and tracks."), QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok);

    if(res == QMessageBox::Ok) {
        CSearchDB::self().clear();
        CMapDB::self().clear();
        CWptDB::self().clear();
        CTrackDB::self().clear();
        CDiaryDB::self().clear();
        COverlayDB::self().clear();
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


void CMainWindow::setupMenuBar()
{
    QMenu * menu;

    menu = new QMenu(this);
    menu->setTitle(tr("&File"));
    menu->addAction(QIcon(":/icons/iconOpenMap16x16.png"),tr("Load Map"),this,SLOT(slotLoadMapSet()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconFileLoad16x16.png"),tr("Load Geo Data"),this,SLOT(slotLoadData()), Qt::CTRL + Qt::Key_L);
    menu->addAction(QIcon(":/icons/iconFileSave16x16.png"),tr("Save Geo Data"),this,SLOT(slotSaveData()), Qt::CTRL + Qt::Key_S);
    menu->addAction(QIcon(":/icons/iconFileAdd16x16.png"),tr("Add Geo Data"),this,SLOT(slotAddData()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconPrint16x16.png"),tr("Print Map ..."),this,SLOT(slotPrint()), Qt::CTRL + Qt::Key_P);
    menu->addAction(QIcon(":/icons/iconPrint16x16.png"),tr("Print Diary ..."),this,SLOT(slotPrintPreview()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconExit16x16.png"),tr("Exit"),this,SLOT(close()));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&Setup"));
    menu->addAction(QIcon(":/icons/iconConfig16x16.png"),tr("&General"),this,SLOT(slotConfig()));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&Help"));
    menu->addAction(QIcon(":/icons/iconGlobe16x16.png"),tr("About &QLandkarte GT"),this,SLOT(slotCopyright()));
    menuBar()->addMenu(menu);
}


void CMainWindow::keyPressEvent(QKeyEvent * e)
{

    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)) {
        return megaMenu->keyPressEvent(e);
    }
    else if(e->key() == Qt::Key_Escape) {
        return megaMenu->keyPressEvent(e);
    }
    else if((e->key() == Qt::Key_Plus) || (e->key() == Qt::Key_Minus)) {
        return megaMenu->keyPressEvent(e);
    }
    else if(e->modifiers() == Qt::AltModifier) {
        if((e->key() == Qt::Key_Up) || (e->key() == Qt::Key_Down)
        || (e->key() == Qt::Key_Left) || (e->key() == Qt::Key_Right)) {
            return megaMenu->keyPressEvent(e);
        }
    }
    return e->ignore();
}


void CMainWindow::closeEvent(QCloseEvent * e)
{
    if(!modified) {
        e->accept();
        return;
    }

    if (maybeSave()) {
        e->accept();
    }
    else {
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
        ,"Map Collection (*.qmap);;GeoTiff (*.tif);; Garmin (*.tdb);; WMS (*.xml)"
#else
        ,"Map Collection (*.qmap);;GeoTiff (*.tif);; Garmin (*.tdb)"
#endif
        , &filter
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
    if(haveGPSBabel) {
        formats = "QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx);;Geocaching.com/EasyGPS (*.loc);;Mapsource (*.gdb)";
    }
    else {
        formats = "QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx)";
    }

    QSettings cfg;
    QString filter   = cfg.value("geodata/filter","").toString();
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select input file")
        ,pathData
        ,formats
        ,&filter
        );
    if(filename.isEmpty()) return;

    if(modified) {
        if(!maybeSave()) {
            return;
        }
    }

    CMapDB::self().clear();
    CWptDB::self().clear();
    CTrackDB::self().clear();
    CDiaryDB::self().clear();
    COverlayDB::self().clear();

    loadData(filename, filter);

    modified = false;
    setTitleBar();

    cfg.setValue("geodata/filter",filter);
}


void CMainWindow::slotAddData()
{
    bool haveGPSBabel = QProcess::execute("gpsbabel -V") == 0;
    QString formats;
    if(haveGPSBabel) {
        formats = "QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx);;Geocaching.com/EasyGPS (*.loc);;Mapsource (*.gdb)";
    }
    else {
        formats = "QLandkarte (*.qlb);;GPS Exchange (*.gpx);;TCX TrainingsCenterExchange (*.tcx)";
    }

    QSettings cfg;
    QString filter   = cfg.value("geodata/filter","").toString();
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select input file")
        ,pathData
        ,formats
        ,&filter
        );

    if(filename.isEmpty()) return;

    QString tmp = wksFile;
    loadData(filename, filter);
    wksFile = tmp;

    modified = true;
    setTitleBar();

    cfg.setValue("geodata/filter",filter);
}


void CMainWindow::loadData(QString& filename, const QString& filter)
{
    QTemporaryFile tmpfile;
    bool loadGPXData = false;
    QString ext      = filename.right(4);

    if(filter == "QLandkarte (*.qlb)") {
        if(ext != ".qlb") filename += ".qlb";
        ext = "QLB";
    }
    else if(filter == "GPS Exchange (*.gpx)") {
        if(ext != ".gpx") filename += ".gpx";
        ext = "GPX";
    }
    else if(filter == "TCX TrainingsCenterExchange (*.tcx)") {
        if(ext != ".tcx") filename += ".tcx";
        ext = "TCX";
    }
    else if(filter == "Geocaching.com/EasyGPS (*.loc)") {
        if(ext != ".loc") filename += ".loc";
        ext = "LOC";
    }
    else if(filter == "Mapsource (*.gdb)") {
        if(ext != ".gdb") filename += ".gdb";
        ext = "GDB";
    }
    else {
        filename += ".qlb";
        ext = "QLB";
    }

    pathData = QFileInfo(filename).absolutePath();

    try
    {
        if(ext == "QLB") {
            CQlb qlb(this);
            qlb.load(filename);
            CMapDB::self().loadQLB(qlb);
            CWptDB::self().loadQLB(qlb);
            CTrackDB::self().loadQLB(qlb);
            CDiaryDB::self().loadQLB(qlb);
            COverlayDB::self().loadQLB(qlb);
        }
        else if(ext == "GPX") {
            CGpx gpx(this);
            gpx.load(filename);
            CMapDB::self().loadGPX(gpx);
            CWptDB::self().loadGPX(gpx);
            CTrackDB::self().loadGPX(gpx);
            CDiaryDB::self().loadGPX(gpx);
            COverlayDB::self().loadGPX(gpx);
        }
        else if(ext == "TCX") {
            TcxReader tcxReader(this);
            if (!tcxReader.read(filename)) {
                throw(tcxReader.errorString());
            }
            else {
                //emit CTrackDB::self().sigChanged(); //??
                QRectF r = CTrackDB::self().getBoundingRectF();
                if (!r.isNull ()) {
                    CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
                }
            }
        }
        else if(ext == "LOC") {
            tmpfile.open();
            loadGPXData = convertData("geo", filename, "gpx", tmpfile.fileName());
            if (!loadGPXData) {
                QMessageBox::critical(0,tr("Convert error"),"Error in data conversion?",QMessageBox::Ok,QMessageBox::NoButton);
            }
            else {
                CGpx gpx(this);
                gpx.load(tmpfile.fileName());
                CMapDB::self().loadGPX(gpx);
                CWptDB::self().loadGPX(gpx);
                CTrackDB::self().loadGPX(gpx);
                CDiaryDB::self().loadGPX(gpx);
                COverlayDB::self().loadGPX(gpx);
            }
        }
        else if(ext == "GDB") {
            tmpfile.open();
            loadGPXData = convertData("gdb", filename, "gpx", tmpfile.fileName());
            if (!loadGPXData) {
                QMessageBox::critical(0,tr("Convert error"),"Error in data conversion?",QMessageBox::Ok,QMessageBox::NoButton);
            }
            else {
                CGpx gpx(this);
                gpx.load(tmpfile.fileName());
                CMapDB::self().loadGPX(gpx);
                CWptDB::self().loadGPX(gpx);
                CTrackDB::self().loadGPX(gpx);
                CDiaryDB::self().loadGPX(gpx);
                COverlayDB::self().loadGPX(gpx);
            }
        }
        wksFile = filename;
    }
    catch(const QString& msg) {
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

    if (ret == QMessageBox::Save) {
        if(wksFile.isEmpty()) {
            slotSaveData();
        }
        else {
            QSettings cfg;
            QString filter = cfg.value("geodata/filter","").toString();
            saveData(wksFile, filter);
        }
        return true;
    }
    else if (ret == QMessageBox::Cancel) {
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
        );
    if(filename.isEmpty()) return;

    cfg.setValue("geodata/filter",filter);

    saveData(filename, filter);
}


void CMainWindow::saveData(const QString& fn, const QString& filter)
{
    QString filename = fn;
    QString ext = filename.right(4);

    if(filter == "QLandkarte (*.qlb)") {
        if(ext != ".qlb") filename += ".qlb";
        ext = "QLB";
    }
    else if(filter == "GPS Exchange (*.gpx)") {
        if(ext != ".gpx") filename += ".gpx";
        ext = "GPX";
    }
    else {
        filename += ".qlb";
        ext = "QLB";
    }

    pathData = QFileInfo(filename).absolutePath();

    try
    {
        if(ext == "QLB") {
            CQlb qlb(this);
            CMapDB::self().saveQLB(qlb);
            CWptDB::self().saveQLB(qlb);
            CTrackDB::self().saveQLB(qlb);
            CDiaryDB::self().saveQLB(qlb);
            COverlayDB::self().saveQLB(qlb);
            qlb.save(filename);
        }
        else if(ext == "GPX") {
            CGpx gpx(this);
            CMapDB::self().saveGPX(gpx);
            CWptDB::self().saveGPX(gpx);
            CTrackDB::self().saveGPX(gpx);
            CDiaryDB::self().saveGPX(gpx);
            COverlayDB::self().saveGPX(gpx);
            gpx.save(filename);
        }

        wksFile  = filename;
        modified = false;
    }
    catch(const QString& msg) {
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
    if(c > 0) {
        if(c == 1) {
            str += tr("Currently there is %1 <a href='Waypoints'>waypoint</a>, ").arg(c);
        }
        else {
            str += tr("Currently there are %1 <a href='Waypoints'>waypoints</a>, ").arg(c);
        }
    }
    else {
        str += tr("There are no waypoints, ");
    }

    c = CTrackDB::self().count();
    if(c > 0) {
        if(c == 1) {
            str += tr(" %1 <a href='Tracks'>track</a> and ").arg(c);
        }
        else {
            str += tr(" %1 <a href='Tracks'>tracks</a> and ").arg(c);
        }
    }
    else {
        str += tr("no tracks and ");
    }

    c = COverlayDB::self().count();
    if(c > 0) {
        if(c == 1) {
            str += tr(" %1 <a href='Overlay'>overlay</a>. ").arg(c);
        }
        else {
            str += tr(" %1 <a href='Overlay'>overlays</a>. ").arg(c);
        }
    }
    else {
        str += tr("no overlays. ");
    }

    c = CDiaryDB::self().count();
    if(c > 0) {
        str += tr("A <a href='Diary'>diary</a> is loaded.");
    }
    else {
        str += tr("The diary (<a href='Diary'>new</a>) is empty.");
    }

    str += "</p>";

    summary->setText(str);

}


void CMainWindow::slotOpenLink(const QString& link)
{
    if(link == "Diary") {
        CDiaryDB::self().openEditWidget();
    }
    else if(link == "Clear") {
        clearAll();
    }
    else {
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

    resources->m_devKey = devKey;
    comboDevice->setCurrentIndex(comboDevice->findData(resources->m_devKey));
}
