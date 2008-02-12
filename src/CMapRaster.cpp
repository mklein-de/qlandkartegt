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

#include "CMapRaster.h"
#include "CMapLevel.h"
#include "CMapFile.h"
#include "CMapDEM.h"
#include "GeoMath.h"

#include <QtGui>

#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <projects.h>


static  const char * cimage_args[] = {
     "BLOCKXSIZE=256"
    ,"BLOCKYSIZE=256"
    ,"TILED=YES"
    ,"COMPRESS=DEFLATE"
    ,NULL
};

CExportMapThread::CExportMapThread(CMapRaster * map)
    : QThread(map)
    , theMap(map)
    , canceled(false)
{

    connect(this,SIGNAL(sigSetRange(int,int)),&map->progressExport,SLOT(setRange(int,int)));
    connect(this,SIGNAL(sigSetValue(int)),&map->progressExport,SLOT(setValue(int)));
    connect(this,SIGNAL(sigSetMessage(const QString &)),&map->progressExport,SLOT(setLabelText(const QString &)));
    connect(this,SIGNAL(sigDone(int)),&map->progressExport,SLOT(done(int)));

    connect(map->butCancelExport,SIGNAL(pressed()), this, SLOT(slotCancel()));

};

void CExportMapThread::setup(const XY& p1, const XY& p2, const QString& filename, const QString& c)
{
    QMutexLocker lock(&mutex);
    topLeft     = p1;
    bottomRight = p2;

    QFileInfo fi(filename);

    filebasename    = fi.baseName();
    exportPath      = fi.path();
    comment         = c;
    if(comment.isEmpty()){
        comment = filebasename;
        comment = comment.replace("_"," ");
    }

}

void CExportMapThread::slotCancel()
{
    QMutexLocker lock(&mutex);
    canceled = true;
}

void CExportMapThread::run()
{
    QMutexLocker lock(&mutex);
    qDebug() << ">>>> CExportMapThread::run()";

    canceled = false;
    int cntLevel = 0, cntFile = 0;

    // build export name by <BaseMapName>_<top [degree]>_<left [degree]>_<width [m]>x<height [m]>
    /*
        topLeft         p4


          p3        bottomRight
    */

    XY p3,p4;
    p3.u = topLeft.u;
    p3.v = bottomRight.v;
    p4.u = bottomRight.u;
    p4.v = topLeft.v;

    float a1 = 0, a2 = 0;
    int realWidth  = ::distance(topLeft, p4, a1, a2);
    int realHeight = ::distance(topLeft, p3, a1, a2);

    QSettings mapdef(exportPath.filePath(filebasename + ".qmap"),QSettings::IniFormat);
    mapdef.setValue("home/zoom",1);

    mapdef.beginGroup("description");
    mapdef.setValue("comment",comment);

    QString str;
    GPS_Math_Deg_To_Str(topLeft.u * RAD_TO_DEG, topLeft.v * RAD_TO_DEG, str);
    str = str.replace("\260","");
    mapdef.setValue("topleft",str);

    GPS_Math_Deg_To_Str(bottomRight.u * RAD_TO_DEG, bottomRight.v * RAD_TO_DEG, str);
    str = str.replace("\260","");
    mapdef.setValue("bottomright",str);

    mapdef.setValue("width",QString("%1 m").arg(realWidth));
    mapdef.setValue("height",QString("%1 m").arg(realHeight));

    mapdef.endGroup();

    QVector<CMapLevel*> maplevels = theMap->maplevels;
    QVector<CMapLevel*>::const_iterator maplevel = maplevels.begin();

    // iterate over map levels and files
    while(maplevel != maplevels.end()){
        ++cntLevel;

        // calculate export area for that map level [m]. The top left and
        // bottom right point has to be transformed from WGS84 into the map
        // level's projection system
        XY p1 = topLeft;
        XY p2 = bottomRight;

        pj_transform(theMap->pjtar,(*(*maplevel)->begin())->pj,1,0,&p1.u,&p1.v,0);
        pj_transform(theMap->pjtar,(*(*maplevel)->begin())->pj,1,0,&p2.u,&p2.v,0);

        QRectF exportarea(QPointF(p1.u,p1.v),QPointF(p2.u,p2.v));

        // calculate center point
        XY pt;
        QPointF px  = exportarea.center();
        pt.u = px.x();
        pt.v = px.y();
        pj_transform((*(*maplevel)->begin())->pj,theMap->pjtar,1,0,&pt.u,&pt.v,0);

        QString center;
        GPS_Math_Deg_To_Str(pt.u * RAD_TO_DEG, pt.v * RAD_TO_DEG, center);
        center = center.replace("\260","");
        mapdef.setValue("home/center",center);

        // copy zoom levels
        mapdef.beginGroup(QString("level%1").arg(cntLevel));
        mapdef.setValue("zoomLevelMin",(*maplevel)->min);
        mapdef.setValue("zoomLevelMax",(*maplevel)->max);

        // iterate over all files and test them for intersection with the export area.
        // if a valid intersection is found the area is adjusted to block boundaries
        // and exported.
        QStringList files;
        QVector<CMapFile*>::const_iterator mapfile = (*maplevel)->begin();
        while(mapfile != (*maplevel)->end()){
            ++cntFile;

            QRectF maparea  = QRectF(QPointF((*mapfile)->xref1, (*mapfile)->yref1), QPointF((*mapfile)->xref2, (*mapfile)->yref2));
            QRect intersect = exportarea.intersected(maparea).toRect();

            if(intersect.isValid()){
                QString strFilename = QString("%1_%2_%3.tif").arg(filebasename).arg(cntLevel).arg(cntFile);

                files << QFileInfo(strFilename).fileName();

                emit sigSetMessage(tr("Export level %1 file %2 to %3").arg(cntLevel).arg(cntFile).arg(strFilename));
                quint32 x1,y1,x2,y2;
                x1 = intersect.left();
                y1 = intersect.bottom();
                x2 = intersect.right();
                y2 = intersect.top();

                // the original boundaries in [m]
//                 qDebug() << intersect << x1 << y1 << x2 << y2;

                x1 = (((x1 - (*mapfile)->xref1) / (*mapfile)->xscale) / (*mapfile)->tileWidth);
                y1 = (((y1 - (*mapfile)->yref1) / (*mapfile)->yscale) / (*mapfile)->tileHeight);

                x2 = (((x2 - (*mapfile)->xref1) / (*mapfile)->xscale + (*mapfile)->tileWidth)) / (*mapfile)->tileWidth;
                y2 = (((y2 - (*mapfile)->yref1) / (*mapfile)->yscale + (*mapfile)->tileHeight)) / (*mapfile)->tileHeight;

                // the boundaries in number of blocks
//                 qDebug() << intersect << x1 << y1 << x2 << y2;

                emit sigSetRange(0,(x2 - x1) * (y2 - y1));

                // setup dataset
                char strBlockXSize[64],strBlockYSize[64];
                snprintf(strBlockXSize,sizeof(strBlockXSize),"BLOCKXSIZE=%i",(*mapfile)->tileWidth);
                snprintf(strBlockYSize,sizeof(strBlockYSize),"BLOCKYSIZE=%i",(*mapfile)->tileHeight);
                cimage_args[0] = strBlockXSize;
                cimage_args[1] = strBlockYSize;


                GDALDriverManager * drvman  = GetGDALDriverManager();
                GDALDriver  * driver        = drvman->GetDriverByName("GTiff");
                GDALDataset * dataset       = driver->Create(exportPath.filePath(strFilename).toLatin1(),
                                                             (x2 - x1) * (*mapfile)->tileWidth,
                                                             (y2 - y1) * (*mapfile)->tileHeight,
                                                             1,GDT_Byte,(char **)cimage_args);

                // setup projection
                char * ptr = 0;
                OGRSpatialReference oSRS;
                oSRS.importFromProj4((*mapfile)->strProj.toLatin1());
                oSRS.exportToWkt(&ptr);
                dataset->SetProjection(ptr);

                CPLFree(ptr);

                double adfGeoTransform[6];
                memset(adfGeoTransform,0,sizeof(adfGeoTransform));

                double  Ep = (*mapfile)->xref1 + x1 * (*mapfile)->tileWidth * (*mapfile)->xscale;
                double  Np = (*mapfile)->yref1 + y1 * (*mapfile)->tileHeight * (*mapfile)->yscale;

                adfGeoTransform[0] = Ep;                                /* top left x */
                adfGeoTransform[1] = (*mapfile)->xscale;                /* w-e pixel resolution */
                adfGeoTransform[2] = 0;                                 /* rotation, 0 if image is "north up" */
                adfGeoTransform[3] = Np;                                /* top left y */
                adfGeoTransform[4] = 0;                                 /* rotation, 0 if image is "north up" */
                adfGeoTransform[5] = (*mapfile)->yscale;                /* n-s pixel resolution */
                dataset->SetGeoTransform(adfGeoTransform);

                // get access to source and target raster band
                GDALRasterBand * pBandSrc   = (*mapfile)->dataset->GetRasterBand(1);;
                GDALRasterBand * pBandTar   = dataset->GetRasterBand(1);

                // copy colortable
                pBandTar->SetColorTable(pBandSrc->GetColorTable());

                // start to copy block by block
                quint8 * blockdata = new quint8[(*mapfile)->tileWidth * (*mapfile)->tileHeight];
                quint32 n,m,progress = 0;

                for(m = y1; m < y2; ++m){
                    for(n = x1; n < x2; ++n){

                        emit sigSetValue(++progress);

                        pBandSrc->ReadBlock(n,m,blockdata);
                        pBandTar->WriteBlock(n-x1,m-y1,blockdata);

                        // allow main thread to cancel
                        mutex.unlock();
                        mutex.lock();
                        if(canceled){
                            emit sigDone(0);
                            qDebug() << "<<<< CExportMapThread::run() - canceled";
                            return;
                        }
                    }
                }

                dataset->FlushCache();
                delete [] blockdata;
                delete dataset;
            }
            ++mapfile;
        }
        mapdef.setValue("files",files.join("|"));
        mapdef.endGroup(); // level...
        ++maplevel;
    }

    mapdef.setValue("main/levels",cntLevel);
    mapdef.sync();
    emit sigDone(0);
    qDebug() << "<<<< CExportMapThread::run()";
}

CMapRaster::CMapRaster(const QString& filename, QObject * parent)
    : IMap(parent)
    , filename(filename)
    , pMaplevel(0)
    , zoomFactor(1)
    , pDEM(0)

{
    // setup export progress dialog
    butCancelExport = new QPushButton(tr("Cancel"),&progressExport);
    progressExport.setCancelButton(butCancelExport);

    // create map export thread handler
    thExportMap     = new CExportMapThread(this);


    QDir path = QFileInfo(filename).absolutePath();
    // load map definition
    QSettings mapdef(filename,QSettings::IniFormat);
    int n = mapdef.value("main/levels",0).toInt();

    // create map level list
    CMapLevel * maplevel = 0;
    while(n){
        mapdef.beginGroup(QString("level%1").arg(n));
        quint32 min = mapdef.value("zoomLevelMin",-1).toUInt();
        quint32 max = mapdef.value("zoomLevelMax",-1).toUInt();
        maplevel = new CMapLevel(min,max,this);

        // add GeoTiff files to map level
        QStringList files = mapdef.value("files","").toString().split("|", QString::SkipEmptyParts);
        if(files.count()){
            QString file;
            foreach(file,files){
                maplevel->addMapFile(path.filePath(file));
            }
            maplevels << maplevel;
        }

        mapdef.endGroup();
        --n;
    }

    // If no configuration is stored read values from the map definition's "home" section
    // zoom() has to be called in either case to setup / initialize all other internal parameters
    mapdef.beginGroup(QString("home"));
    zoomidx = mapdef.value("zoom",1).toUInt();
    zoom(zoomidx);

    QString pos = mapdef.value("center","").toString();
    float u = topLeft.u;
    float v = topLeft.v;
    GPS_Math_Str_To_Deg(pos, u, v);

    topLeft.u = u * DEG_TO_RAD;
    topLeft.v = v * DEG_TO_RAD;
    mapdef.endGroup();


    QString fileDEM = mapdef.value("DEM/file","").toString();
    if(!fileDEM.isEmpty()){
        pDEM = new CMapDEM(path.filePath(fileDEM), this);
    }

    QSettings cfg;
    exportPath  = cfg.value("path/export",cfg.value("path/maps","./")).toString();

    qDebug() << "done";
}

CMapRaster::~CMapRaster()
{
    QSettings mapdef(filename,QSettings::IniFormat);
    mapdef.beginGroup(QString("home"));

    mapdef.setValue("zoom",zoomidx);
    QString pos;

    GPS_Math_Deg_To_Str(topLeft.u * RAD_TO_DEG, topLeft.v * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");
    mapdef.setValue("center",pos);
    mapdef.endGroup();

    QSettings cfg;
    cfg.setValue("path/export",exportPath);
}


void CMapRaster::draw(QPainter& p)
{
    if(pMaplevel.isNull()){
        IMap::draw(p);
        return;
    }

    bool foundMap = false;

    const CMapFile * map = *pMaplevel->begin();

    // top left
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    bottomRight.u = pt.u + size.width() * map->xscale * zoomFactor;
    bottomRight.v = pt.v + size.height() * map->yscale * zoomFactor;
    pj_transform(pjsrc,pjtar,1,0,&bottomRight.u,&bottomRight.v,0);

    // the viewport rectangel in [m]
    QRectF viewport(pt.u, pt.v, size.width() * map->xscale * zoomFactor,  size.height() * map->yscale * zoomFactor);

    // Iterate over all mapfiles within a maplevel. If a map's rectangel intersects with the
    // viewport rectangle, the part of the map within the intersecting rectangle has to be drawn.
    QVector<CMapFile*>::const_iterator mapfile = pMaplevel->begin();
    while(mapfile != pMaplevel->end()){

        map = *mapfile;

        QRectF maparea   = QRectF(QPointF(map->xref1, map->yref1), QPointF(map->xref2, map->yref2));
        QRectF intersect = viewport.intersected(maparea);

        if(intersect.isValid()){

            // x/y offset [pixel] into file matrix
            qint32 xoff = (intersect.left() - map->xref1) / map->xscale;
            qint32 yoff = (intersect.bottom()  - map->yref1) / map->yscale;

            // number of x/y pixel to read
            qint32 pxx  =   (qint32)(intersect.width() / map->xscale);
            qint32 pxy  =  -(qint32)(intersect.height() / map->yscale);

            // the final image width and height in pixel
            qint32 w    =   (qint32)(pxx / zoomFactor) & 0xFFFFFFFC;
            qint32 h    =   (qint32)(pxy / zoomFactor);

            GDALRasterBand * pBand;
            pBand = map->dataset->GetRasterBand(1);

            QImage img(QSize(w,h),QImage::Format_Indexed8);
            img.setColorTable(map->colortable);

            CPLErr err = pBand->RasterIO(GF_Read
                    ,(int)xoff,(int)yoff
                    ,pxx,pxy
                    ,img.bits()
                    ,w,h
                    ,GDT_Byte,0,0);

            if(!err){
                double xx = intersect.left(), yy = intersect.bottom();
                convertM2Pt(xx,yy);
                p.drawPixmap(xx,yy,QPixmap::fromImage(img));
                foundMap = true;
            }

        }
        ++mapfile;
    }

    if(!foundMap){
        IMap::draw(p);
    }

    if(pDEM && (overlay != eNone)){
        const CMapFile * map = *pMaplevel->begin();
        pDEM->draw(p, topLeft, bottomRight, map->xscale*zoomFactor, map->yscale*zoomFactor, overlay);
    }

}


void CMapRaster::convertPt2M(double& u, double& v)
{
    if(pMaplevel.isNull()) return;

    const CMapFile * map = *pMaplevel->begin();

    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = pt.u + u * map->xscale * zoomFactor;
    v = pt.v + v * map->yscale * zoomFactor;
}

void CMapRaster::convertM2Pt(double& u, double& v)
{
    if(pMaplevel.isNull()) return;

    const CMapFile * map = *pMaplevel->begin();

    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = (u - pt.u) / (map->xscale * zoomFactor);
    v = (v - pt.v) / (map->yscale * zoomFactor);
}

void CMapRaster::move(const QPoint& old, const QPoint& next)
{

    XY p2 = topLeft;
    convertRad2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

}

void CMapRaster::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? -1 : 1;
    zoom(zoomidx);

    // convert geo. coordinates back to point
    convertRad2Pt(p1.u, p1.v);

    XY p2 = topLeft;
    convertRad2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;
}

void CMapRaster::zoom(quint32& level)
{
    if(maplevels.isEmpty()){
        pMaplevel   = 0;
        pjsrc       = 0;
        return;
    }

    // no level less than 1
    if(level < 1) level = 1;

    QVector<CMapLevel*>::const_iterator maplevel = maplevels.begin();

    while(maplevel != maplevels.end()){
        if((*maplevel)->min <= level && level <= (*maplevel)->max){
            break;
        }

        ++maplevel;
    }

    // no maplevel means level is larger than maximum level
    // thus the last (maximum) level is used.
    if(maplevel == maplevels.end()){
        --maplevel;
        level = (*maplevel)->max;
    }

    pMaplevel   = *maplevel;
    pjsrc       = (*pMaplevel->begin())->pj;
    zoomFactor  = level - (*maplevel)->min + 1;
    qDebug() << "zoom:" << zoomFactor;
}

void CMapRaster::select(const QRect& rect)
{
    if(pMaplevel.isNull()) return;

    XY p1,p2;
    p1.u = rect.left();
    p1.v = rect.top();
    p2.u = rect.right();
    p2.v = rect.bottom();

    convertPt2Rad(p1.u, p1.v);
    convertPt2Rad(p2.u, p2.v);

    QString filebase    = QString("%1_%2_%3").arg(QFileInfo(filename).baseName()).arg(p1.v * RAD_TO_DEG).arg(p1.u * RAD_TO_DEG);
    filebase            = filebase.replace(".","");
    QString fn = QFileDialog::getSaveFileName(0,tr("Select map collection filename...")
                                               ,QDir(exportPath).filePath(filebase + ".qmap")
                                               ,"*.qmap"
                                             );
    if(fn.isEmpty()){
        return;
    }

    exportPath = QFileInfo(fn).path();


    QString internalMapName = QInputDialog::getText(0,tr("Enter short description..."),QFileInfo(filename).baseName());

    progressExport.setLabelText(tr("Export map ..."));
    progressExport.show();

    thExportMap->setup(p1,p2,fn,internalMapName);
    thExportMap->start();
}

void CMapRaster::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    if(pMaplevel.isNull()){
        lon1 = lat1 = lon2 = lat2 = 0;
        return;
    }

    pMaplevel->dimensions(lon1, lat1, lon2, lat2);
}

float CMapRaster::getElevation(float lon, float lat)
{
    if(pDEM){
        return pDEM->getElevation(lon,lat);
    }
    return IMap::getElevation(lon,lat);

}
