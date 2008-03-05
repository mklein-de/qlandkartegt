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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CMapEditWidget.h"
#include "CCreateMapOSM.h"
#include "CCreateMapQMAP.h"
#include "CCreateMapGeoTiff.h"

CMapEditWidget::CMapEditWidget(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);
    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    comboSource->insertItem(eNone,tr(""));
    comboSource->insertItem(eOSM,QIcon(":/icons/iconOSM16x16.png"),tr("Open Street Map"));
    comboSource->insertItem(eQMAP,QIcon(":/icons/iconGlobe16x16.png"),tr("Create map collection from existing GeoTiff."));
    comboSource->insertItem(eGTIFF,QIcon(":/icons/iconGlobe16x16.png"),tr("Convert a TIFF into GeoTiff by geo referencing it."));

    connect(comboSource, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));

    widgetOSM       = new CCreateMapOSM(stackedWidget);
    widgetQMAP      = new CCreateMapQMAP(stackedWidget);
    widgetGeoTiff   = new CCreateMapGeoTiff(stackedWidget);

    stackedWidget->insertWidget(eOSM, widgetOSM);
    stackedWidget->insertWidget(eQMAP, widgetQMAP);
    stackedWidget->insertWidget(eGTIFF, widgetGeoTiff);
}


CMapEditWidget::~CMapEditWidget()
{

}
