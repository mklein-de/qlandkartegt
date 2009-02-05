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

#include "CMapEditWidget.h"
#include "CCreateMapOSM.h"
#include "CCreateMapQMAP.h"
#include "CCreateMapGeoTiff.h"
#include "CCreateMapWMS.h"

#include <QtGui>

CMapEditWidget::CMapEditWidget(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("CMapEditWidget");
    setAttribute(Qt::WA_DeleteOnClose,true);
    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    bool haveGDALWarp       = QProcess::execute("gdalwarp --version") == 0;
    bool haveGDALTranslate  = QProcess::execute("gdal_translate --version") == 0;
    bool haveGDAL = haveGDALWarp && haveGDALTranslate;

    comboSource->insertItem(eNone,tr(""));

    //     comboSource->insertItem(eOSM,QIcon(":/icons/iconOSM16x16.png"),tr("Open Street Map"));
    //     widgetOSM       = new CCreateMapOSM(stackedWidget);
    //     stackedWidget->insertWidget(eOSM, widgetOSM);

    comboSource->insertItem(eQMAP,QIcon(":/icons/iconGlobe16x16.png"),tr("Create map collection from existing GeoTiff."));
    widgetQMAP      = new CCreateMapQMAP(stackedWidget);
    stackedWidget->insertWidget(eQMAP, widgetQMAP);

    comboSource->insertItem(eGTIFF,QIcon(":/icons/iconGlobe16x16.png"),tr("Convert a TIFF into GeoTiff by geo referencing it."));
    if(haveGDAL) {
        widgetGeoTiff   = new CCreateMapGeoTiff(stackedWidget);
        stackedWidget->insertWidget(eGTIFF, widgetGeoTiff);
    }
    else {
        QLabel * label = new QLabel(stackedWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setText(tr("<b style='color: red;'>Can't find the GDAL tools in your path. Make sure you have Installed GDAL and all related command line applications.</b>"));
        stackedWidget->insertWidget(eGTIFF, label);
    }

    comboSource->insertItem(eWMS,QIcon(":/icons/iconWMS16x16.png"),tr("Create a GDAL WMS definition file."));
    widgetWMS       = new CCreateMapWMS(stackedWidget);
    stackedWidget->insertWidget(eWMS, widgetWMS);


    connect(comboSource, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));
}


CMapEditWidget::~CMapEditWidget()
{

}
