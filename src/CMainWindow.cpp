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

#include <QtGui>

CMainWindow * theMainWindow = 0;

CMainWindow::CMainWindow()
    : pathMaps("./")
    , mapFile("")
{
    theMainWindow = this;

    setObjectName("MainWidget");
    setWindowTitle("QLandkarte 2");
    setWindowIcon(QIcon(":/icons/iconGlobe16x16.png"));

    statusCoord = new QLabel(this);
    statusBar()->addPermanentWidget(statusCoord);

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
    leftSplitter->addWidget(new QWidget());

    showMaximized();

    // restore last session settings
    QSettings cfg;
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

    pathMaps = cfg.value("path/maps",pathMaps).toString();

    mapFile = cfg.value("map/mapFile",mapFile).toString();

    if(!mapFile.isEmpty()){
        canvas->loadMapSet(QDir(pathMaps).filePath(mapFile));
    }

}

CMainWindow::~CMainWindow()
{
    QSettings cfg;
    cfg.setValue("mainWidget/mainSplitter",mainSplitter->saveState());
    cfg.setValue("mainWidget/leftSplitter",leftSplitter->saveState());
    cfg.setValue("path/maps",pathMaps);
    cfg.setValue("map/mapFile",mapFile);
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
    menu->addAction(QIcon(":/icons/iconExit16x16.png"),tr("Exit"),this,SLOT(close()));
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
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select *.map file")
                                                    ,pathMaps
                                                    ,"Map Collection (*.map)"
                                                    );
    if(filename.isEmpty()) return;

    pathMaps = QFileInfo(filename).absolutePath();
    mapFile  = QFileInfo(filename).fileName();

    canvas->loadMapSet(filename);

}

void CMainWindow::slotCopyright()
{
    CCopyright dlg;
    dlg.exec();
}
