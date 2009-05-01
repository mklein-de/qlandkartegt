/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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

#include "CRoute.h"
#include "CRouteDB.h"
#include "CRouteToolWidget.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>

CRouteDB * CRouteDB::m_self = 0;

CRouteDB::CRouteDB(QTabWidget * tb, QObject * parent)
: IDB(tb, parent)
, cnt(0)
{
    m_self      = this;
    toolview    = new CRouteToolWidget(tb);
}

CRouteDB::~CRouteDB()
{

}

void CRouteDB::addRoute(CRoute * route, bool silent)
{
    if(route->getName().isEmpty()) {
        route->setName(tr("Route%1").arg(cnt++));
    }

    delRoute(route->key(), silent);
    routes[route->key()] = route;

    connect(route,SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    if(!silent) {
        emit sigChanged();
        emit sigModified();
    }

}

void CRouteDB::delRoute(const QString& key, bool silent)
{
    if(!routes.contains(key)) return;
    delete routes.take(key);
    if(!silent) {
        emit sigChanged();
        emit sigModified();
    }
}

void CRouteDB::delRoutes(const QStringList& keys)
{
    QString key;
    foreach(key,keys) {
        if(!routes.contains(key)) continue;
        delete routes.take(key);
    }
    emit sigChanged();
    emit sigModified();
}

void CRouteDB::highlightRoute(const QString& key)
{
    QMap<QString,CRoute*>::iterator route = routes.begin();
    while(route != routes.end()) {
        (*route)->setHighlight(false);
        ++route;
    }

    routes[key]->setHighlight(true);

    emit sigHighlightRoute(routes[key]);
    emit sigChanged();

}


CRoute* CRouteDB::highlightedRoute()
{

    QMap<QString,CRoute*>::iterator route = routes.begin();
    while(route != routes.end()) {
        if((*route)->isHighlighted()) return *route;
        ++route;
    }
    return 0;

}

QRectF CRouteDB::getBoundingRectF(const QString key)
{
    if(!routes.contains(key)) {
        return QRectF();
    }
    return routes.value(key)->getBoundingRectF();
}


/// load database data from gpx
void CRouteDB::loadGPX(CGpx& gpx)
{

}

/// save database data to gpx
void CRouteDB::saveGPX(CGpx& gpx)
{

}

/// load database data from QLandkarte binary
void CRouteDB::loadQLB(CQlb& qlb)
{

}

/// save database data to QLandkarte binary
void CRouteDB::saveQLB(CQlb& qlb)
{

}

void CRouteDB::upload()
{

}

void CRouteDB::download()
{

}

void CRouteDB::clear()
{
    cnt = 0;
    delRoutes(routes.keys());
    emit sigChanged();

}

void CRouteDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    IMap& map = CMapDB::self().getMap();

    p.setRenderHint(QPainter::Antialiasing,false);

    QMap<QString,CRoute*>::iterator route       = routes.begin();
    QMap<QString,CRoute*>::iterator highlighted = routes.end();

    while(route != routes.end()) {
        QPolygon& line = (*route)->getPolyline();

        bool firstTime = (*route)->firstTime;

        QList<XY>& rtepts         = (*route)->getRoutePoints();
        QList<XY>::iterator rtept = rtepts.begin();

        if ( needsRedraw || firstTime) {
            line.clear();
            while(rtept != rtepts.end()) {
                double u = rtept->u * DEG_TO_RAD;
                double v = rtept->v * DEG_TO_RAD;

                map.convertRad2Pt(u,v);
                line << QPoint(u,v);;
                ++rtept;
            }
        }

        if(!rect.intersects(line.boundingRect())) {
            ++route; continue;
        }

        if((*route)->isHighlighted()) {
            // store highlighted route to draw it later
            // it must be drawn above all other routes
            highlighted = route;
        }
        else {
            // draw normal route
            QPen pen(Qt::darkMagenta,7);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);
            p.drawPolyline(line);
            p.setPen(Qt::white);
            p.drawPolyline(line);

            // draw bubbles
            QPoint pt;
            QPixmap bullet = (*route)->getIcon();
            foreach(pt,line) {
                p.drawPixmap(pt.x() - 8 ,pt.y() - 8, bullet);
            }
        }

        (*route)->firstTime = false;
        ++route;
    }

    // if there is a highlighted route, draw it
    if(highlighted != routes.end()) {
        route = highlighted;

        QPolygon& line = (*route)->getPolyline();

        // draw skunk line
        QPen pen(Qt::magenta,7);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        p.drawPolyline(line);
        p.setPen(Qt::white);
        p.drawPolyline(line);

        // draw bubbles
        QPoint pt;
        QPixmap bullet = (*route)->getIcon();
        foreach(pt,line) {
            p.drawPixmap(pt.x() - 8 ,pt.y() - 8, bullet);
        }
    }
}

