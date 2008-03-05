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

#include "CDlgCreateMap.h"
#include "CCreateMapOSM.h"
#include "CCreateMapQMAP.h"

CDlgCreateMap::CDlgCreateMap(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);
    comboSource->insertItem(eNone,tr(""));
    comboSource->insertItem(eOSM,QIcon(":/icons/iconOSM16x16.png"),tr("Open Street Map"));
    comboSource->insertItem(eQMAP,QIcon(":/icons/iconGlobe16x16.png"),tr("Create map collection from existing GeoTiff."));

    connect(comboSource, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));

    widgetOSM  = new CCreateMapOSM(stackedWidget);
    widgetQMAP = new CCreateMapQMAP(stackedWidget);

    stackedWidget->insertWidget(eOSM, widgetOSM);
    stackedWidget->insertWidget(eQMAP, widgetQMAP);
}


CDlgCreateMap::~CDlgCreateMap()
{

}


void CDlgCreateMap::editMap(const QString& filename)
{
    stackedWidget->setCurrentIndex(eQMAP);
    widgetQMAP->readqmap(filename);
    exec();
}
