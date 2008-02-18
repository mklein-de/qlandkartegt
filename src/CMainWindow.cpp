/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/


#include "CMainWindow.h"
#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CCopyright.h"
#include "CResources.h"
#include "CMapDB.h"
#include "CWptDB.h"
#include "CSearchDB.h"
#include "CDlgConfig.h"
#include "CDlgCreateMap.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>

CMainWindow * theMainWindow = 0;

CMainWindow::CMainWindow()
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

    canvas = new CCanvas(this);
    rightSplitter->addWidget(canvas);

    megaMenu = new CMegaMenu(canvas);
    leftSplitter->addWidget(megaMenu);

    toolbox = new QToolBox(canvas);
    leftSplitter->addWidget(toolbox);

    statusCoord = new QLabel(this);
    statusBar()->insertPermanentWidget(1,statusCoord);

    QSettings cfg;
    pathData = cfg.value("path/data","./").toString();

    searchdb    = new CSearchDB(toolbox, this);
    mapdb       = new CMapDB(toolbox, this);
    wptdb       = new CWptDB(toolbox, this);

    connect(searchdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(wptdb, SIGNAL(sigChanged()), canvas, SLOT(update()));
    connect(toolbox, SIGNAL(currentChanged(int)), this, SLOT(slotToolBoxChanged(int)));

    showMaximized();

    // restore last session settings
    QList<int> sizes = mainSplitter->sizes();
    sizes[0] = (int)(mainSplitter->width() * 0.1);
    sizes[1] = (int)(mainSplitter->width() * 0.9);
    mainSplitter->setSizes(sizes);
    sizes = leftSplitter->sizes();
    sizes[0] = (int)(mainSplitter->height() * 0.5);
    sizes[1] = (int)(mainSplitter->height() * 0.5);
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

}

CMainWindow::~CMainWindow()
{
    QSettings cfg;
    cfg.setValue("mainWidget/mainSplitter",mainSplitter->saveState());
    cfg.setValue("mainWidget/leftSplitter",leftSplitter->saveState());
    cfg.setValue("path/data",pathData);
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
    menu->addAction(QIcon(":/icons/iconOpenMap16x16.png"),tr("Load Map Set"),this,SLOT(slotLoadMapSet()));
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconFileLoad16x16.png"),tr("Load Geo Data"),this,SLOT(slotLoadData()), Qt::CTRL + Qt::Key_L);
    menu->addAction(QIcon(":/icons/iconFileSave16x16.png"),tr("Save Geo Data"),this,SLOT(slotSaveData()), Qt::CTRL + Qt::Key_S);
    menu->addSeparator();
    menu->addAction(QIcon(":/icons/iconExit16x16.png"),tr("Exit"),this,SLOT(close()));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&Maps"));
    menu->addAction(QIcon(":/icons/iconOpenMap16x16.png"),tr("Load Map Set"),this,SLOT(slotLoadMapSet()));
    menu->addAction(QIcon(":/icons/iconMapWizard16x16.png"),tr("Create / Edit ..."),this,SLOT(slotCreateMap()));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&Setup"));
    menu->addAction(QIcon(":/icons/iconConfig16x16.png"),tr("Config"),this,SLOT(slotConfig()));
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(tr("&About"));
    menu->addAction(QIcon(":/icons/iconGlobe16x16.png"),tr("Copyright"),this,SLOT(slotCopyright()));
    menuBar()->addMenu(menu);
}

void CMainWindow::keyPressEvent(QKeyEvent * e)
{

    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)){
        return megaMenu->keyPressEvent(e);
    }
    else if(e->key() == Qt::Key_Escape) {
        return megaMenu->keyPressEvent(e);
    }
    else if((e->key() == Qt::Key_Plus) || (e->key() == Qt::Key_Minus)) {
        return megaMenu->keyPressEvent(e);
    }
    else if(e->modifiers() == Qt::AltModifier){
        if((e->key() == Qt::Key_Up) || (e->key() == Qt::Key_Down)
            || (e->key() == Qt::Key_Left) || (e->key() == Qt::Key_Right)) {
            return megaMenu->keyPressEvent(e);
        }
    }
    return e->ignore();
}



void CMainWindow::slotLoadMapSet()
{
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select *.qmap file")
                                                    ,CResources::self().pathMaps
                                                    ,"Map Collection (*.qmap)"
                                                    );
    if(filename.isEmpty()) return;

    CResources::self().pathMaps = QFileInfo(filename).absolutePath();
    CMapDB::self().openMap(filename,*canvas);

}

void CMainWindow::slotCopyright()
{
    CCopyright dlg;
    dlg.exec();
}

void CMainWindow::slotToolBoxChanged(int idx)
{
    QString key = toolbox->widget(idx)->objectName();
    megaMenu->switchByKeyWord(key);
}

void CMainWindow::slotConfig()
{
    CDlgConfig dlg(this);
    dlg.exec();
}

void CMainWindow::slotLoadData()
{
    QString filter;
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select input file")
                                                    ,pathData
                                                    ,"QLandkarte (*.qlb);;GPS Exchange (*.gpx)"
                                                    ,&filter
                                                );
    if(filename.isEmpty()) return;

    QString ext = filename.right(4);

    if(filter == "QLandkarte (*.qlb)"){
        if(ext != ".qlb") filename += ".qlb";
        ext = "QLB";
    }
    else if(filter == "GPS Exchange (*.gpx)"){
        if(ext != ".gpx") filename += ".gpx";
        ext = "GPX";
    }
    else{
        filename += ".qlb";
        ext = "QLB";
    }

    pathData = QFileInfo(filename).absolutePath();

    if(ext == "QLB"){
        CQlb qlb(this);
        qlb.load(filename);
        CWptDB::self().loadQLB(qlb);
    }
    else if(ext == "GPX"){
        CGpx gpx(this);
        gpx.load(filename);
        CWptDB::self().loadGPX(gpx);
    }
}

void CMainWindow::slotSaveData()
{
    QString filter;
    QString filename = QFileDialog::getSaveFileName( 0, tr("Select output file")
                                                    ,pathData
                                                    ,"QLandkarte (*.qlb);;GPS Exchange (*.gpx)"
                                                    ,&filter
                                                );
    if(filename.isEmpty()) return;

    QString ext = filename.right(4);

    if(filter == "QLandkarte (*.qlb)"){
        if(ext != ".qlb") filename += ".qlb";
        ext = "QLB";
    }
    else if(filter == "GPS Exchange (*.gpx)"){
        if(ext != ".gpx") filename += ".gpx";
        ext = "GPX";
    }
    else{
        filename += ".qlb";
        ext = "QLB";
    }

    pathData = QFileInfo(filename).absolutePath();

    if(ext == "QLB"){
        CQlb qlb(this);
        CWptDB::self().saveQLB(qlb);
        qlb.save(filename);
    }
    else if(ext == "GPX"){
        CGpx gpx(this);
        CWptDB::self().saveGPX(gpx);
        gpx.save(filename);
    }

}

void CMainWindow::slotCreateMap()
{
    CDlgCreateMap dlg(this);
    dlg.exec();
}
