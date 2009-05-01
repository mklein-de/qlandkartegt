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

}
