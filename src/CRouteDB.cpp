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
#include "CQlb.h"
#include "CGpx.h"
#include "CResources.h"
#include "IDevice.h"

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
    if(route->getName().isEmpty())
    {
        route->setName(tr("Route%1").arg(cnt++));
    }

    delRoute(route->key(), silent);
    routes[route->key()] = route;

    connect(route,SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }

}


void CRouteDB::delRoute(const QString& key, bool silent)
{
    if(!routes.contains(key)) return;
    delete routes.take(key);
    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}


void CRouteDB::delRoutes(const QStringList& keys)
{
    QString key;
    foreach(key,keys)
    {
        if(!routes.contains(key)) continue;
        delete routes.take(key);
    }
    emit sigChanged();
    emit sigModified();
}


CRoute * CRouteDB::getRoute(const QString& key)
{
    if(routes.contains(key))
    {
        return routes[key];
    }
    else
    {
        return 0;
    }
}


void CRouteDB::highlightRoute(const QString& key)
{
    QMap<QString,CRoute*>::iterator route = routes.begin();
    while(route != routes.end())
    {
        (*route)->setHighlight(false);
        ++route;
    }

    if(routes.contains(key))
    {
        routes[key]->setHighlight(true);
    }

    emit sigHighlightRoute(routes[key]);
    emit sigChanged();

}


CRoute* CRouteDB::highlightedRoute()
{

    QMap<QString,CRoute*>::iterator route = routes.begin();
    while(route != routes.end())
    {
        if((*route)->isHighlighted()) return *route;
        ++route;
    }
    return 0;

}


QRectF CRouteDB::getBoundingRectF(const QString key)
{
    if(!routes.contains(key))
    {
        return QRectF();
    }
    return routes.value(key)->getBoundingRectF();
}


/// load database data from gpx
void CRouteDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& rtes = gpx.elementsByTagName("rte");
    uint N = rtes.count();
    for(uint n = 0; n < N; ++n)
    {
        const QDomNode& rte = rtes.item(n);

        CRoute * r = 0;
        /* name is not a required element. */
        if(rte.namedItem("name").isElement())
        {
            r = new CRoute(this);
            r->setName(rte.namedItem("name").toElement().text());
        }
        else
        {
            /* Use desc if name is unavailable, else give it no name. */
            if (rte.namedItem("desc").isElement())
            {
                r = new CRoute(this);
                r->setName(rte.namedItem("desc").toElement().text());
            }
            else
            {
                r = new CRoute(this);
                r->setName(tr("Unnamed"));
            }
        }

        QDomElement rtept = rte.firstChildElement("rtept");

        while (!rtept.isNull())
        {
            XY pt;
            QDomNamedNodeMap attr = rtept.attributes();

            pt.u = attr.namedItem("lon").nodeValue().toDouble();
            pt.v = attr.namedItem("lat").nodeValue().toDouble();

            r->addPosition(pt.u,pt.v);

            if(rtept.namedItem("sym").isElement())
            {
                QString symname = rtept.namedItem("sym").toElement().text();
                r->setIcon(symname);
            }

            rtept = rtept.nextSiblingElement("rtept");
        }

        if(routes.contains(r->key()))
        {
            delete routes.take(r->key());
        }

        r->calcDistance();
        routes[r->key()] = r;

        connect(r,SIGNAL(sigChanged()),SIGNAL(sigChanged()));

    }

    emit sigChanged();
}


/// save database data to gpx
void CRouteDB::saveGPX(CGpx& gpx)
{
    QDomElement root = gpx.documentElement();
    QMap<QString,CRoute*>::iterator route = routes.begin();
    while(route != routes.end())
    {
        QDomElement gpxRoute = gpx.createElement("rte");
        root.appendChild(gpxRoute);

        QDomElement name = gpx.createElement("name");
        gpxRoute.appendChild(name);
        QDomText _name_ = gpx.createTextNode((*route)->getName());
        name.appendChild(_name_);

        unsigned cnt = 0;
        QList<XY>& rtepts = (*route)->getRoutePoints();
        QList<XY>::const_iterator rtept = rtepts.begin();
        while(rtept != rtepts.end())
        {
            QDomElement gpxRtept = gpx.createElement("rtept");
            gpxRoute.appendChild(gpxRtept);

            gpxRtept.setAttribute("lat",QString::number(rtept->v,'f',6));
            gpxRtept.setAttribute("lon",QString::number(rtept->u,'f',6));

            QString str = QString("%1").arg(++cnt,3,10,QChar('0'));

            QDomElement name = gpx.createElement("name");
            gpxRtept.appendChild(name);
            QDomText _name_ = gpx.createTextNode(str);
            name.appendChild(_name_);

            QDomElement cmt = gpx.createElement("cmt");
            gpxRtept.appendChild(cmt);
            QDomText _cmt_ = gpx.createTextNode(str);
            cmt.appendChild(_cmt_);

            QDomElement desc = gpx.createElement("desc");
            gpxRtept.appendChild(desc);
            QDomText _desc_ = gpx.createTextNode(str);
            desc.appendChild(_desc_);

            QDomElement sym = gpx.createElement("sym");
            gpxRtept.appendChild(sym);
            QDomText _sym_ = gpx.createTextNode((*route)->getIconName());
            sym.appendChild(_sym_);

            ++rtept;
        }

        ++route;
    }
}


/// load database data from QLandkarte binary
void CRouteDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.routes(),QIODevice::ReadOnly);

    while(!stream.atEnd())
    {
        CRoute * route = new CRoute(this);
        stream >> *route;
        route->calcDistance();
        addRoute(route, true);
    }

    emit sigChanged();
}


/// save database data to QLandkarte binary
void CRouteDB::saveQLB(CQlb& qlb)
{
    QMap<QString, CRoute*>::const_iterator route = routes.begin();
    while(route != routes.end())
    {
        qlb << *(*route);
        ++route;
    }
}


void CRouteDB::upload()
{
    if(routes.isEmpty()) return;

    IDevice * dev = CResources::self().device();
    if(dev)
    {
        QList<CRoute*> tmprtes = routes.values();
        dev->uploadRoutes(tmprtes);
    }
}


void CRouteDB::download()
{
    IDevice * dev = CResources::self().device();
    if(dev)
    {
        QList<CRoute*> tmprtes;
        dev->downloadRoutes(tmprtes);

        if(tmprtes.isEmpty()) return;

        CRoute * rte;
        foreach(rte,tmprtes)
        {
            addRoute(rte, true);
        }
    }

    emit sigChanged();
    emit sigModified();
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

    while(route != routes.end())
    {
        QPolygon& line = (*route)->getPolyline();

        bool firstTime = (*route)->firstTime;

        QList<XY>& rtepts         = (*route)->getRoutePoints();
        QList<XY>::iterator rtept = rtepts.begin();

        if ( needsRedraw || firstTime)
        {
            line.clear();
            while(rtept != rtepts.end())
            {
                double u = rtept->u * DEG_TO_RAD;
                double v = rtept->v * DEG_TO_RAD;

                map.convertRad2Pt(u,v);
                line << QPoint(u,v);;
                ++rtept;
            }
        }

        if(!rect.intersects(line.boundingRect()))
        {
            ++route; continue;
        }

        if((*route)->isHighlighted())
        {
            // store highlighted route to draw it later
            // it must be drawn above all other routes
            highlighted = route;
        }
        else
        {
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
            foreach(pt,line)
            {
                p.drawPixmap(pt.x() - 8 ,pt.y() - 8, bullet);
            }
        }

        (*route)->firstTime = false;
        ++route;
    }

    // if there is a highlighted route, draw it
    if(highlighted != routes.end())
    {
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
        foreach(pt,line)
        {
            p.drawPixmap(pt.x() - 8 ,pt.y() - 8, bullet);
        }
    }
}
