/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "CCreateMapFineTune.h"
#include "CMapDB.h"
#include "CMainWindow.h"
#include "CMapRaster.h"

#include <QtGui>

CCreateMapFineTune::CCreateMapFineTune(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);

    toolUp->setIcon(QIcon(":/icons/iconUpload16x16"));
    toolDown->setIcon(QIcon(":/icons/iconDownload16x16"));
    toolLeft->setIcon(QIcon(":/icons/iconLeft16x16"));
    toolRight->setIcon(QIcon(":/icons/iconRight16x16"));

    connect(pushOpenFile, SIGNAL(clicked()), this, SLOT(slotOpenFile()));
    connect(toolUp, SIGNAL(clicked()), this, SLOT(slotUp()));
    connect(toolDown, SIGNAL(clicked()), this, SLOT(slotDown()));
    connect(toolLeft, SIGNAL(clicked()), this, SLOT(slotLeft()));
    connect(toolRight, SIGNAL(clicked()), this, SLOT(slotRight()));

}

CCreateMapFineTune::~CCreateMapFineTune()
{

}

void CCreateMapFineTune::slotOpenFile()
{
    QSettings cfg;
    path = QDir(cfg.value("path/create",path.path()).toString());

    QString filename = QFileDialog::getOpenFileName(0, tr("Open map file..."),path.path(), tr("Referenced file (*.tif *.tiff *.png *.gif)"), 0, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) return;


    CMapDB::self().openMap(filename, false, *theMainWindow->getCanvas());

    IMap& map = CMapDB::self().getMap();

    QString proj = map.getProjection();
    if(proj.isEmpty())
    {
        toolUp->setEnabled(false);
        toolDown->setEnabled(false);
        toolLeft->setEnabled(false);
        toolRight->setEnabled(false);
    }
    else
    {
        toolUp->setEnabled(true);
        toolDown->setEnabled(true);
        toolLeft->setEnabled(true);
        toolRight->setEnabled(true);
    }

}

void CCreateMapFineTune::slotUp()
{
    IMap& map = CMapDB::self().getMap();
    map.incYOffset(1);
}

void CCreateMapFineTune::slotDown()
{
    IMap& map = CMapDB::self().getMap();
    map.decYOffset(1);
}

void CCreateMapFineTune::slotLeft()
{
    IMap& map = CMapDB::self().getMap();
    map.decXOffset(1);
}

void CCreateMapFineTune::slotRight()
{
    IMap& map = CMapDB::self().getMap();
    map.incXOffset(1);
}


void CCreateMapFineTune::slotSave()
{

}
