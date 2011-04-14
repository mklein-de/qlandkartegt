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

#include "CMapOSM.h"
#include "GeoMath.h"
#include "CCanvas.h"

#include <QtGui>
#include "CMainWindow.h"
#include "COsmTilesHash.h"
#include "CResources.h"
#include "CDlgMapOSMConfig.h"
#include "CMapOSMType.h"
#include <QDebug>

CMapOSM::CMapOSM(CCanvas * parent)
: IMap(eTile, "OSMTileServer", parent)
, parent(parent)
, zoomFactor(1.0)
, x(0)
, y(0)
, xscale( 1.19432854652)
, yscale(-1.19432854652)
, needsRedrawOvl(true)

{
    currentTileListIndex = -1;
    osmTiles = 0;

    cb = new QComboBox(theMainWindow->getCanvas());
    connect(cb,SIGNAL(activated( int )),this,SLOT(setNewTileUrl(int)));

    theMainWindow->statusBar()->insertPermanentWidget(0,cb);

    this->rebuildServerList();

    QSettings cfg;

    int tileListIndex = cfg.value("osm/tileListIndex",0).toInt();
    if (tileListIndex >= tileList.size())
        tileListIndex = 0;

    setNewTileUrl(tileListIndex);
    pjsrc = pj_init_plus("+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +a=6378137 +b=6378137 +units=m +no_defs ");

    char * ptr = pj_get_def(pjsrc,0);
    qDebug() << "OSM:" << ptr;
    //     if(ptr) free(ptr);

    QString pos     = cfg.value("osm/topleft","N82 58.759 W151 08.934").toString();
    zoomidx         = cfg.value("osm/zoomidx",15).toInt();

    lon1 = xref1   = -40075016/2;
    lat1 = yref1   =  40075016/2;
    lon2 = xref2   =  40075016/2;
    lat2 = yref2   = -40075016/2;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

    if(pos.isEmpty())
    {
        x = 0;
        y = 0;
    }
    else
    {
        float u = 0;
        float v = 0;
        GPS_Math_Str_To_Deg(pos, u, v);
        x = u * DEG_TO_RAD;
        y = v * DEG_TO_RAD;
        pj_transform(pjtar,pjsrc,1,0,&x,&y,0);
    }

    zoom(zoomidx);

    resize(parent->size());

}


CMapOSM::~CMapOSM()
{
    QString pos;
    QSettings cfg;

    double u = x;
    double v = y;
    pj_transform(pjsrc,pjtar,1,0,&u,&v,0);

    GPS_Math_Deg_To_Str(u * RAD_TO_DEG, v * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");

    cfg.setValue("osm/topleft",pos);
    cfg.setValue("osm/zoomidx",zoomidx);
    cfg.setValue("osm/tileListIndex",currentTileListIndex);
    midU = rect.center().x();
    midV = rect.center().y();
    convertPt2Rad(midU, midV);

    if(pjsrc) pj_free(pjsrc);
    if (osmTiles) delete osmTiles;
    if (cb) delete cb;
}


void CMapOSM::rebuildServerList()
{
    QString cbOldText=cb->currentText();

    tileList.clear();

    QSettings cfg;

    // %1 = osm_zoom; %2 = osm_x; %3 = osm_y

    CMapOSMType mapOsm(QString("OpenStreetMap"),QString("tile.openstreetmap.org/%1/%2/%3.png"));
    mapOsm.setBuiltin(QString("osm"));
    mapOsm.setEnabled(cfg.value("osm/builtinMaps/osm", true).toBool());
    tileList << mapOsm;

    CMapOSMType mapOcm(QString("OpenCycleMap"),QString("andy.sandbox.cloudmade.com/tiles/cycle/%1/%2/%3.png"));
    mapOcm.setBuiltin(QString("ocm"));
    mapOcm.setEnabled(cfg.value("osm/builtinMaps/ocm", true).toBool());
    tileList << mapOcm;

    //    CMapOSMType mapGeo(QString("topo.geofabrik.de CC-NC-SA"),QString("topo.geofabrik.de/trails/%1/%2/%3.png"));
    //    mapGeo.setBuiltin(QString("geofabrik"));
    //    mapGeo.setEnabled(cfg.value("osm/builtinMaps/geo", true).toBool());
    //    tileList << mapGeo;

    CMapOSMType mapOpm(QString("OpenPisteMap"),QString("openpistemap.org/tiles/contours/%1/%2/%3.png"));
    mapOpm.setBuiltin(QString("opm"));
    mapOpm.setEnabled(cfg.value("osm/builtinMaps/opm", true).toBool());
    tileList << mapOpm;

    CMapOSMType mapWam(QString("WanderatlasMap"),QString("maps.ich-geh-wandern.de/contours/%1/%2/%3.png"));
    mapWam.setBuiltin(QString("wam"));
    mapWam.setEnabled(cfg.value("osm/builtinMaps/wam", true).toBool());
    tileList << mapWam;

#if PRIVATE
    CMapOSMType mapOade(QString("Outdooractive DE"),QString("t0.outdooractive.com/portal/map/%1/%2/%3.png"));
    mapOade.setBuiltin(QString("oade"));
    mapOade.setEnabled(cfg.value("osm/builtinMaps/oade", true).toBool());
    tileList << mapOade;

    CMapOSMType mapOaat(QString("Outdooractive AT"),QString("t0.outdooractive.com/austria/map/%1/%2/%3.png"));
    mapOaat.setBuiltin(QString("oaat"));
    mapOaat.setEnabled(cfg.value("osm/builtinMaps/oaat", true).toBool());
    tileList << mapOaat;

    CMapOSMType mapOait(QString("Outdooractive IT"),QString("t0.outdooractive.com/suedtirol/map/%1/%2/%3.png"));
    mapOait.setBuiltin(QString("oait"));
    mapOait.setEnabled(cfg.value("osm/builtinMaps/oait", true).toBool());
    tileList << mapOait;
#endif

    int size = cfg.beginReadArray("osm/customMaps");
    for (int i = 0; i < size; ++i)
    {
        cfg.setArrayIndex(i);
        CMapOSMType map(QString(cfg.value("mapName").toString()),QString(cfg.value("mapString").toString()));
        tileList << map;
    }
    cfg.endArray();


    cb->clear();
    for(int i = 0; i < tileList.size(); i++)
    {
        CMapOSMType p = tileList.at(i);
        if (p.isEnabled())
        {
            cb->addItem(p.title);
        }
    }

    cb->setCurrentIndex(cb->findText(cbOldText));
}


void CMapOSM::setNewTileUrl(int index)
{

    if(index >= tileList.count())
    {
        index = tileList.count() - 1;
    }
    else if(index < 0)
    {
        index = 0;
    }

    if (currentTileListIndex!= index)
    {
        currentTileListIndex = index;
        if (osmTiles)
        {
            delete osmTiles;
            osmTiles = 0;
        }
        if (cb->currentIndex() != currentTileListIndex)
        {
            cb->setCurrentIndex(currentTileListIndex);
        }

        osmTiles = new COsmTilesHash(tileList.at(index).path);
        connect(osmTiles,SIGNAL(newImageReady(QImage,bool)),this,SLOT(newImageReady(QImage,bool)));

        needsRedraw = true;
        emit sigChanged();
    }

}


void CMapOSM::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}


void CMapOSM::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}


void CMapOSM::move(const QPoint& old, const QPoint& next)
{
    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    needsRedraw     = true;
    needsRedrawOvl  = true;
    setFastDrawTimer();
    emit sigChanged();

    setAngleNorth();
}


void CMapOSM::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? -1 : 1;

    // sigChanged will be sent at the end of this function
    blockSignals(true);
    zoom(zoomidx);

    // convert geo. coordinates back to point
    convertRad2Pt(p1.u, p1.v);

    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference point befor and after zoom
    xx += p1.u - p0.x();
    yy += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2M(xx, yy);
    x = xx;
    y = yy;

    needsRedraw     = true;
    needsRedrawOvl   = true;
    blockSignals(false);
    emit sigChanged();
}


void CMapOSM::zoom(qint32& level)
{
    if(level > 18) level = 18;
    // no level less than 1
    if(level < 1)
    {
        level       = 1;
        zoomFactor  = 1.0;
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = (1<<(level-1));
    needsRedraw     = true;
    needsRedrawOvl   = true;
    setFastDrawTimer();
    emit sigChanged();

}


void CMapOSM::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;
    int i;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = fabs(u[1] - u[0]);
    dV = fabs(v[0] - v[2]);

    int z1 = dU / size.width();
    int z2 = dV / size.height();

    for(i=0; i < 18; ++i)
    {
        zoomFactor  = (1<<i);
        zoomidx     = i + 1;
        if(zoomFactor > z1 && zoomFactor > z2) break;
    }

    qDebug() << zoomFactor << z1 << zoomFactor << z2;

    double u_ = lon1 + (lon2 - lon1)/2;
    double v_ = lat1 + (lat2 - lat1)/2;
    convertRad2Pt(u_,v_);
    move(QPoint(u_,v_), rect.center());

    needsRedraw     = true;
    needsRedrawOvl   = true;
    emit sigChanged();

    qDebug() << "zoom:" << zoomFactor;
}


void CMapOSM::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    // render map if necessary
    if(needsRedraw)
    {
        draw();
    }

    p.drawImage(0,0,buffer);

    // render overlay
    if(!ovlMap.isNull() && lastTileLoaded && !doFastDraw)
    {
        ovlMap->draw(size, needsRedrawOvl, p);
        needsRedrawOvl = false;
    }

    needsRedraw = false;

    if(CResources::self().showZoomLevel())
    {

        QString str;
        if(zoomFactor < 1.0)
        {
            str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
        }
        else
        {
            str = tr("Zoom level x%1").arg(zoomFactor);
        }

        p.setPen(Qt::white);
        p.setFont(QFont("Sans Serif",14,QFont::Black));

        p.drawText(9  ,23, str);
        p.drawText(10 ,23, str);
        p.drawText(11 ,23, str);
        p.drawText(9  ,24, str);
        p.drawText(11 ,24, str);
        p.drawText(9  ,25, str);
        p.drawText(10 ,25, str);
        p.drawText(11 ,25, str);

        p.setPen(Qt::darkBlue);
        p.drawText(10,24,str);

        p.setFont(QFont("Sans Serif",8,QFont::Black));

        if(currentTileListIndex < 3)
        {
            CCanvas::drawText(tr("Map has been created by %1 under Creative Commons Attribution-ShareAlike 2.0 license").arg(tileList.at(currentTileListIndex).title), p, rect.bottomLeft() + QPoint(rect.width() / 2, -5) , QColor(Qt::darkBlue));
            CCanvas::drawText(tr("and has been downloaded from: %1").arg(QString(tileList.at(currentTileListIndex).path).arg('z').arg('x').arg('y')), p, rect.bottomLeft() + QPoint(rect.width() / 2, +7) , QColor(Qt::darkBlue));
        }

    }
}


void CMapOSM::draw()
{
    if(pjsrc == 0) return IMap::draw();

    int osm_zoom = 18 - zoomidx;

    double lon = x;
    double lat = y;
    convertM2Rad(lon,lat);
    lastTileLoaded = false;
    if (osmTiles)
        osmTiles->startNewDrawing( lon * RAD_TO_DEG, lat * RAD_TO_DEG,  osm_zoom, rect);

}


void CMapOSM::newImageReady(QImage image, bool done)
{
    buffer            = image;
    lastTileLoaded    = done;
    needsRedraw       = false;
    emit sigChanged();
}


void CMapOSM::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    QRectF r(x, y, size.width() * xscale * zoomFactor,  size.height() * yscale * zoomFactor);
    p1.u        = r.left();
    p1.v        = r.top();
    p2.u        = r.right();
    p2.v        = r.bottom();

    if(!pj_is_latlong(pjsrc))
    {
        pj_transform(pjsrc,pjtar,1,0,&p1.u,&p1.v,0);
        pj_transform(pjsrc,pjtar,1,0,&p2.u,&p2.v,0);
    }

    my_xscale   = xscale * zoomFactor;
    my_yscale   = yscale * zoomFactor;
}


void CMapOSM::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1;
    lat1 = this->lat1;
    lon2 = this->lon2;
    lat2 = this->lat2;
}


void CMapOSM::config()
{
    CDlgMapOSMConfig * dlg = new CDlgMapOSMConfig(this);
    dlg->show();

}
