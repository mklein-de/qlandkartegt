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

#include "CMapFile.h"
#include "CMapLevel.h"
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <QtCore>
#include <QColor>

CMapFile::CMapFile(const QString& filename, QObject * parent, const QString& datum, const QString& gridfile)
: QObject(parent)
, filename(filename)
, dataset(0)
, xsize_px(0)
, ysize_px(0)
, pj(0)
, xscale(0.0)
, yscale(0.0)
, xref1(0.0)
, yref1(0.0)
, ok(false)
{
    dataset = (GDALDataset*)GDALOpen(filename.toUtf8(),GA_ReadOnly);
    if(dataset == 0) return;

    char str[1024];
    strncpy(str,dataset->GetProjectionRef(),sizeof(str));
    char * ptr = str;
    OGRSpatialReference oSRS;
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);
    strOrigProj = strProj = ptr;
    if(!datum.isEmpty() && !gridfile.isEmpty()) {
        strProj = strProj.replace(QString("+datum=%1").arg(datum), QString("+nadgrids=%1").arg(gridfile));
    }

    qDebug() << strProj;

    pj = pj_init_plus(strProj.toLatin1());
    if(pj == 0) return;

    xsize_px = dataset->GetRasterXSize();
    ysize_px = dataset->GetRasterYSize();

    double adfGeoTransform[6];
    dataset->GetGeoTransform( adfGeoTransform );

    xscale  = adfGeoTransform[1];
    yscale  = adfGeoTransform[5];

    xref1   = adfGeoTransform[0];
    yref1   = adfGeoTransform[3];

    xref2   = xref1 + xsize_px * xscale;
    yref2   = yref1 + ysize_px * yscale;

    qDebug() << xref1 << yref1 << xref2 << yref2;

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    if(pBand == 0) return;

    if(pBand->GetColorInterpretation() ==  GCI_PaletteIndex ) {
        GDALColorTable * pct = pBand->GetColorTable();
        for(int i=0; i < pct->GetColorEntryCount(); ++i) {
            const GDALColorEntry& e = *pct->GetColorEntry(i);
            colortable << qRgba(e.c1, e.c2, e.c3, e.c4);
        }
    }
    else if(pBand->GetColorInterpretation() ==  GCI_GrayIndex ) {
        for(int i=0; i < 256; ++i) {
            colortable << qRgba(i, i, i, 255);
        }
    }

    int success = 0;
    double idx = pBand->GetNoDataValue(&success);

    if(success) {
        QColor tmp(colortable[idx]);
        tmp.setAlpha(0);
        colortable[idx] = tmp.rgba();
    }

    pBand->GetBlockSize(&tileWidth,&tileHeight);

    PJ * pjWGS84 = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");

    lon1 = xref1;
    lat1 = yref1;
    pj_transform(pj,pjWGS84,1,0,&lon1,&lat1,0);

    lon2 = xref2;
    lat2 = yref2;
    pj_transform(pj,pjWGS84,1,0,&lon2,&lat2,0);

    pj_free(pjWGS84);

    ok = true;
}


CMapFile::~CMapFile()
{
    if(pj) pj_free(pj);
    if(dataset) delete dataset;

}
