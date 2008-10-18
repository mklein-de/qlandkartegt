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

#include "IMap.h"
#include "CWpt.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>
#include <math.h>

IMap::IMap(const QString& key, CCanvas * parent)
: QObject(parent)
, zoomidx(1)
, pjsrc(0)
, pjtar(0)
, needsRedraw(true)
, key(key)
{
    pjtar   = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    QSettings cfg;
    zoomidx = cfg.value("map/zoom",zoomidx).toUInt();

    if(parent){
        resize(parent->size());
        connect(parent, SIGNAL(sigResize(const QSize&)), this , SLOT(resize(const QSize&)));
        parent->update();
    }
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
}


void IMap::draw(QPainter& p)
{
    p.fillRect(rect,QColor("#ffffcc"));
    p.drawText(rect,Qt::AlignCenter,"no map");
}

void IMap::draw()
{
    QPainter p(&buffer);
    p.fillRect(rect,QColor("#ffffcc"));
    p.drawText(rect,Qt::AlignCenter,"no map");
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


float IMap::getElevation(float lon, float lat)
{
    return WPT_NOFLOAT;
}


void IMap::getArea_n_Scaling_fromBase(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    CMapDB::self().getMap().getArea_n_Scaling(p1,p2,my_xscale,my_yscale);
}

const char * IMap::getProjection()
{
    if(pjsrc == 0){
        return "";
    }
    return pj_get_def(pjsrc,0);
}

