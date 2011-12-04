/**********************************************************************************************
    Copyright (C) 2011 Oliver Eichler oliver.eichler@gmx.de

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
#include "CGridDB.h"
#include "CMapDB.h"
#include "IMap.h"
#include "CMainWindow.h"

#include <QtGui>

CGridDB * CGridDB::m_pSelf = 0;

CGridDB::CGridDB(QObject * parent)
: QObject(parent)
, pjWGS84(0)
, pjGrid(0)
, showGrid(false)
{
    m_pSelf = this;
    pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    //pjGrid  = pj_init_plus("+proj=utm +zone=32  +ellps=WGS84 +datum=WGS84 +no_defs");
    pjGrid  = pj_init_plus("+proj=merc  +ellps=WGS84 +datum=WGS84 +no_defs");
    //pjGrid  = pj_init_plus("+proj=longlat  +ellps=WGS84 +datum=WGS84 +no_defs");

    checkGrid = new QCheckBox(theMainWindow);
    checkGrid->setText(tr("Grid"));
    theMainWindow->statusBar()->addPermanentWidget(checkGrid);

    connect(checkGrid, SIGNAL(toggled(bool)), this, SLOT(slotShowGrid(bool)));
    connect(checkGrid, SIGNAL(clicked()), theMainWindow->getCanvas(), SLOT(update()));

    QSettings cfg;
    showGrid = cfg.value("map/grid", showGrid).toBool();
    checkGrid->setChecked(showGrid);

}

CGridDB::~CGridDB()
{
    if(pjWGS84) pj_free(pjWGS84);
    if(pjGrid) pj_free(pjGrid);

    QSettings cfg;
    cfg.setValue("map/grid", showGrid);
}


void CGridDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    if(pjWGS84 == 0 || pjGrid == 0 || !showGrid) return;

    IMap& map       = CMapDB::self().getMap();

    XY topLeft, topRight, btmLeft, btmRight;

    btmLeft.u   = topLeft.u     = rect.left();
    topRight.v  = topLeft.v     = rect.top();
    btmRight.u  = topRight.u    = rect.right();
    btmRight.v  = btmLeft.v     = rect.bottom();

    map.convertPt2Rad(topLeft.u,  topLeft.v);
    map.convertPt2Rad(topRight.u, topRight.v);
    map.convertPt2Rad(btmLeft.u,  btmLeft.v);
    map.convertPt2Rad(btmRight.u, btmRight.v);

    pj_transform(pjWGS84, pjGrid, 1, 0, &topLeft.u, &topLeft.v, 0);
    pj_transform(pjWGS84, pjGrid, 1, 0, &topRight.u, &topRight.v, 0);
    pj_transform(pjWGS84, pjGrid, 1, 0, &btmLeft.u, &btmLeft.v, 0);
    pj_transform(pjWGS84, pjGrid, 1, 0, &btmRight.u, &btmRight.v, 0);

//    qDebug() << "---";
//    qDebug() << "topLeft " << topLeft.u  << topLeft.v;
//    qDebug() << "topRight" << topRight.u << topRight.v;
//    qDebug() << "btmLeft " << btmLeft.u  << btmLeft.v;
//    qDebug() << "btmRight" << btmRight.u << btmRight.v;

//    qDebug() << topLeft.u - topRight.u;
//    qDebug() << btmLeft.u - btmRight.u;

//    qDebug() << topLeft.v  - btmLeft.v;
//    qDebug() << topRight.v - btmRight.v;


    double topMax   = topLeft.v  > topRight.v   ? topLeft.v  : topRight.v;
    double btmMin   = btmLeft.v  < btmRight.v   ? btmLeft.v  : btmRight.v;
    double leftMin  = topLeft.u  < btmLeft.u    ? topLeft.u  : btmLeft.u;
    double rightMax = topRight.u > btmRight.u   ? topRight.u : btmRight.u;


    double xGridSpace;
    double yGridSpace;
    double dX = fabs(leftMin - rightMax) / 10;
    if(dX < PI/180000)
    {
        xGridSpace = PI/1800000;
        yGridSpace = PI/1800000;
    }
    else if(dX < PI/18000)
    {
        xGridSpace = PI/180000;
        yGridSpace = PI/180000;
    }
    else if(dX < PI/1800)
    {
        xGridSpace = PI/18000;
        yGridSpace = PI/18000;
    }
    else if(dX < PI/180)
    {
        xGridSpace = PI/1800;
        yGridSpace = PI/1800;
    }

    else if(dX < 3000)
    {
        xGridSpace = 1000;
        yGridSpace = 1000;
    }
    else if(dX < 7000)
    {
        xGridSpace = 5000;
        yGridSpace = 5000;
    }
    else if(dX < 30000)
    {
        xGridSpace = 10000;
        yGridSpace = 10000;
    }
    else if(dX < 70000)
    {
        xGridSpace = 50000;
        yGridSpace = 50000;
    }
    else if(dX < 300000)
    {
        xGridSpace = 100000;
        yGridSpace = 100000;
    }
    else if(dX < 700000)
    {
        xGridSpace = 500000;
        yGridSpace = 500000;
    }
    else if(dX < 3000000)
    {
        xGridSpace = 1000000;
        yGridSpace = 1000000;
    }
    else if(dX < 7000000)
    {
        xGridSpace = 5000000;
        yGridSpace = 5000000;
    }
    else if(dX < 30000000)
    {
        xGridSpace = 10000000;
        yGridSpace = 10000000;
    }
    else if(dX < 70000000)
    {
        xGridSpace = 50000000;
        yGridSpace = 50000000;
    }


    double xStart = floor(leftMin / xGridSpace) * xGridSpace;
    double yStart = ceil(topMax / yGridSpace) * yGridSpace;

    double x = xStart - xGridSpace;
    double y = yStart + yGridSpace;

//    qDebug() << xStart  << yStart ;
//    qDebug() << xGridSpace  << yGridSpace ;

    p.save();
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::magenta);

    while(y > btmMin)
    {
        while(x < rightMax)
        {
            double x1 = x;
            double y1 = y;
            double x2 = x + xGridSpace;
            double y2 = y;
            double x3 = x + xGridSpace;
            double y3 = y - yGridSpace;
            double x4 = x;
            double y4 = y - yGridSpace;

            pj_transform(pjGrid, pjWGS84, 1, 0, &x1, &y1, 0);
            pj_transform(pjGrid, pjWGS84, 1, 0, &x2, &y2, 0);
            pj_transform(pjGrid, pjWGS84, 1, 0, &x3, &y3, 0);
            pj_transform(pjGrid, pjWGS84, 1, 0, &x4, &y4, 0);

            map.convertRad2Pt(x1, y1);
            map.convertRad2Pt(x2, y2);
            map.convertRad2Pt(x3, y3);
            map.convertRad2Pt(x4, y4);

            p.drawLine(x1, y1, x2, y2);
            p.drawLine(x2, y2, x3, y3);
            p.drawLine(x3, y3, x4, y4);
            p.drawLine(x4, y4, x1, y1);

            x += xGridSpace;
        }
        x  = xStart;
        y -= yGridSpace;
    }

    p.restore();
}
