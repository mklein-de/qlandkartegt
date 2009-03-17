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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "IMap.h"
#include "CWpt.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "CMapDEM.h"
#include "IMapSelection.h"

#include <QtGui>
#include <math.h>

IMap::IMap(maptype_e type, const QString& key, CCanvas * parent)
: QObject(parent)
, maptype(type)
, zoomidx(1)
, pjsrc(0)
, pjtar(0)
, needsRedraw(true)
, key(key)
, doFastDraw(false)
{
    pjtar   = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    QSettings cfg;
    zoomidx = cfg.value("map/zoom",zoomidx).toUInt();

    if(parent) {
        resize(parent->size());
        connect(parent, SIGNAL(sigResize(const QSize&)), this , SLOT(resize(const QSize&)));
        parent->update();
    }

    timerFastDraw = new QTimer(this);
    timerFastDraw->setSingleShot(true);
    connect(timerFastDraw, SIGNAL(timeout()), this, SLOT(slotResetFastDraw()));

}


IMap::~IMap()
{
    qDebug() << "IMap::~IMap()";
    if(pjtar) pj_free(pjtar);

    QSettings cfg;
    cfg.setValue("map/zoom",zoomidx);
}


void IMap::resize(const QSize& s)
{
    size = s;
    rect.setSize(s);
    buffer = QImage(size, QImage::Format_ARGB32);

    needsRedraw = true;
    emit sigResize(s);
}


void IMap::draw(QPainter& p)
{
    p.fillRect(rect,QColor("#ffffcc"));
    p.drawText(rect,Qt::AlignCenter,"no map");
    needsRedraw = false;
}


void IMap::draw()
{
    QPainter p(&buffer);
    p.fillRect(rect,QColor("#ffffcc"));
    p.drawText(rect,Qt::AlignCenter,"no map");
    needsRedraw = false;
}


void IMap::convertPt2Rad(double& u, double& v)
{
    if(pjsrc == 0) {
        //         u = v = 0;
        return;
    }
    convertPt2M(u,v);

    XY pt;
    pt.u = u;
    pt.v = v;

    pj_transform(pjsrc,pjtar,1,0,&pt.u,&pt.v,0);

    u = pt.u;
    v = pt.v;
}


void IMap::convertRad2Pt(double& u, double& v)
{
    if(pjsrc == 0) {
        return;
    }

    XY pt;
    pt.u = u;
    pt.v = v;

    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = pt.u;
    v = pt.v;

    convertM2Pt(u,v);
}


void IMap::convertRad2M(double& u, double& v)
{
    if(pjsrc == 0) {
        return;
    }

    pj_transform(pjtar,pjsrc,1,0,&u,&v,0);
}


void IMap::convertM2Rad(double& u, double& v)
{
    if(pjsrc == 0) {
        return;
    }

    pj_transform(pjsrc,pjtar,1,0,&u,&v,0);
}


float IMap::getElevation(double lon, double lat)
{
    return WPT_NOFLOAT;
}


void IMap::getArea_n_Scaling_fromBase(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    CMapDB::self().getMap().getArea_n_Scaling(p1,p2,my_xscale,my_yscale);
}


static char nullstr[1] = "";
char * IMap::getProjection()
{
    if(pjsrc == 0) {
        return nullstr;
    }
    return pj_get_def(pjsrc,0);
}


void IMap::registerDEM(CMapDEM& dem)
{
    if(pjsrc == 0) {
        dem.deleteLater();
        throw tr("No basemap projection. That shouldn't happen.");
    }

    char * ptr1 = 0, *ptr2 = 0;
    QString proj1 = ptr1 = pj_get_def(pjsrc,0);
    QString proj2 = ptr2 = dem.getProjection();
    if(ptr1) free(ptr1);
    if(ptr2) free(ptr2);

    if(proj1 != proj2) {
        dem.deleteLater();
        throw tr("DEM projection does not match the projection of the basemap.");
    }
}


void IMap::addOverlayMap(const QString& k)
{
    // prevent registering twice
    if(key == k) {
        return;
    }

    needsRedraw = true;

    // pass request to next overlay map
    if(!ovlMap.isNull()) {
        ovlMap->addOverlayMap(k);
        emit sigChanged();
        return;
    }

    // add overlay to last overlay map
    ovlMap = CMapDB::self().createMap(k);
    connect(ovlMap, SIGNAL(sigChanged()), this, SLOT(slotOvlChanged()));
    emit sigChanged();
}


void IMap::slotOvlChanged()
{
    needsRedraw = true;
    emit sigChanged();
}


void IMap::delOverlayMap(const QString& k)
{
    if(ovlMap.isNull()) return;

    if(ovlMap->getKey() != k) {
        ovlMap->delOverlayMap(k);
        emit sigChanged();
        return;
    }

    ovlMap->deleteLater();
    ovlMap = ovlMap->ovlMap;
    emit sigChanged();
}


bool IMap::hasOverlayMap(const QString& k)
{

    if(ovlMap.isNull()) return (k == key);

    if(key != k) {
        return ovlMap->hasOverlayMap(k);
    }

    return true;
}


void IMap::setFastDraw()
{
    timerFastDraw->start(500);
    doFastDraw = true;
}


void IMap::slotResetFastDraw()
{
    needsRedraw = true;
    doFastDraw  = false;
    emit sigChanged();
}


bool IMap::isLonLat()
{
    if(pjsrc) {
        return pj_is_latlong(pjsrc);
    }
    return true;
}

void IMap::select(IMapSelection& ms, const QRect& rect)
{
    throw tr("This map does not support this feature.");
}

