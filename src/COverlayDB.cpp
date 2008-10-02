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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "COverlayDB.h"
#include "COverlayToolWidget.h"
#include "COverlayText.h"
#include "COverlayTextBox.h"
#include "COverlayDistance.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>
#include <projects.h>

COverlayDB * COverlayDB::m_self = 0;

COverlayDB::COverlayDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    toolview    = new COverlayToolWidget(tb);
}


COverlayDB::~COverlayDB()
{

}


void COverlayDB::draw(QPainter& p, const QRect& r)
{
    IOverlay * overlay;
    foreach(overlay, overlays) {
        overlay->draw(p);
    }
}


void COverlayDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& ovls = gpx.elementsByTagName("overlays");
    uint N = ovls.count();
    for(uint n = 0; n < N; ++n) {
        const QDomNode& ovl = ovls.item(n);

        QDomElement element = ovl.firstChildElement();
        while (!element.isNull()) {
            QString type = element.tagName();
            if(type == "text") {
                int top     = element.attribute("top","0").toInt();
                int left    = element.attribute("left","0").toInt();
                int width   = element.attribute("width","0").toInt();
                int height  = element.attribute("height","0").toInt();

                QRect rect(left, top, width, height);
                if(rect.isValid()) {
                    QString text = element.text();
                    addText(text,rect);
                }
            }
            else if(type == "textbox") {
                int top     = element.attribute("top","0").toInt();
                int left    = element.attribute("left","0").toInt();
                int width   = element.attribute("width","0").toInt();
                int height  = element.attribute("height","0").toInt();
                int anchorx = element.attribute("anchorx","0").toInt();
                int anchory = element.attribute("anchory","0").toInt();
                double lon  = element.attribute("lon","0").toDouble() * DEG_TO_RAD;
                double lat  = element.attribute("lat","0").toDouble() * DEG_TO_RAD;

                QRect rect(left, top, width, height);
                if(rect.isValid()) {
                    QString text = element.text();
                    addTextBox(text,lon, lat, QPoint(anchorx, anchory), rect);
                }
            }
            else if(type == "distance") {
                QString name;
                QString comment;
                QList<XY> points;
                QDomNodeList list;
                QDomNode node;

                list = element.elementsByTagName("name");
                if(list.count() == 1) {
                    node = list.item(0);
                    name = node.toElement().text();
                }
                list = element.elementsByTagName("comment");
                if(list.count() == 1) {
                    node = list.item(0);
                    comment = node.toElement().text();
                }

                list = element.elementsByTagName("point");
                for(int i = 0; i < list.size(); ++i) {
                    XY pt;
                    pt.u = list.item(i).toElement().attribute("lon", 0).toDouble() * DEG_TO_RAD;
                    pt.v = list.item(i).toElement().attribute("lat", 0).toDouble() * DEG_TO_RAD;
                    points << pt;
                }

                addDistance(name, comment, points);
            }

            element = element.nextSiblingElement();
        }
    }
}


void COverlayDB::saveGPX(CGpx& gpx)
{
    QString str;
    QDomElement root        = gpx.documentElement();

    QDomElement extensions  = gpx.createElement("extensions");
    root.appendChild(extensions);

    QDomElement _overlay_   = gpx.createElement("overlays");
    extensions.appendChild(_overlay_);

    IOverlay * overlay;
    foreach(overlay, overlays) {

        if(overlay->type == "Text") {
            COverlayText * overlaytext = qobject_cast<COverlayText*>(overlay);
            if(overlaytext == 0) continue;

            QDomElement text  = gpx.createElement("text");
            _overlay_.appendChild(text);

            text.setAttribute("top", overlaytext->rect.top());
            text.setAttribute("left", overlaytext->rect.left());
            text.setAttribute("width", overlaytext->rect.width());
            text.setAttribute("height", overlaytext->rect.height());

            QDomText _text_ = gpx.createTextNode(overlaytext->sometext);
            text.appendChild(_text_);
        }
        else if(overlay->type == "TextBox") {
            COverlayTextBox * ovl = qobject_cast<COverlayTextBox*>(overlay);
            if(ovl == 0) continue;

            QDomElement text  = gpx.createElement("textbox");
            _overlay_.appendChild(text);

            text.setAttribute("top", ovl->rect.top());
            text.setAttribute("left", ovl->rect.left());
            text.setAttribute("width", ovl->rect.width());
            text.setAttribute("height", ovl->rect.height());
            text.setAttribute("anchorx", ovl->pt.x());
            text.setAttribute("anchory", ovl->pt.y());
            str.sprintf("%1.8f",ovl->lon * RAD_TO_DEG);
            text.setAttribute("lon", str);
            str.sprintf("%1.8f",ovl->lat * RAD_TO_DEG);
            text.setAttribute("lat", str);

            QDomText _text_ = gpx.createTextNode(ovl->text);
            text.appendChild(_text_);
        }
        else if(overlay->type == "Distance") {
            COverlayDistance * ovl = qobject_cast<COverlayDistance*>(overlay);
            if(ovl == 0) continue;

            QDomElement elem  = gpx.createElement("distance");
            _overlay_.appendChild(elem);

            QDomElement name  = gpx.createElement("name");
            elem.appendChild(name);
            name.appendChild(gpx.createTextNode(ovl->name));

            QDomElement comment = gpx.createElement("comment");
            elem.appendChild(comment);
            comment.appendChild(gpx.createTextNode(ovl->comment));

            XY pt;
            foreach(pt, ovl->points) {
                QDomElement point = gpx.createElement("point");
                str.sprintf("%1.8f",pt.u * RAD_TO_DEG);
                point.setAttribute("lon",str);
                str.sprintf("%1.8f",pt.v * RAD_TO_DEG);
                point.setAttribute("lat", str);
                elem.appendChild(point);
            }
        }
    }
}


void COverlayDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.overlays(),QIODevice::ReadOnly);

    while(!stream.atEnd()) {
        stream >> *this;
    }

    emit sigChanged();
}


void COverlayDB::saveQLB(CQlb& qlb)
{
    IOverlay * overlay;
    foreach(overlay, overlays) {
        qlb << *overlay;
    }
}


void COverlayDB::clear()
{
    delOverlays(overlays.keys());
}


void COverlayDB::delOverlays(const QStringList& keys)
{
    QString key;
    foreach(key, keys) {
        IOverlay * overlay = overlays.take(key);
        overlay->deleteLater();
    }
    emit sigChanged();
    emit sigModified();
}


COverlayText * COverlayDB::addText(const QString& text, const QRect& rect)
{
    IOverlay * overlay = new COverlayText(text, rect, this);
    overlays[overlay->key] = overlay;

    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigModified()));

    emit sigChanged();
    emit sigModified();

    return qobject_cast<COverlayText*>(overlay);
}


COverlayTextBox * COverlayDB::addTextBox(const QString& text, double lon, double lat, const QPoint& anchor, const QRect& rect)
{

    IOverlay * overlay = new COverlayTextBox(text, lon, lat, anchor, rect, this);
    overlays[overlay->key] = overlay;

    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigModified()));

    emit sigChanged();
    emit sigModified();

    return qobject_cast<COverlayTextBox*>(overlay);
}


COverlayDistance * COverlayDB::addDistance(const QString& name, const QString& comment, const QList<XY>& pts)
{
    IOverlay * overlay = new COverlayDistance(name, comment, pts, this);
    overlays[overlay->key] = overlay;

    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigModified()));

    emit sigChanged();
    emit sigModified();

    return qobject_cast<COverlayDistance*>(overlay);
}


void COverlayDB::customMenu(const QString& key, QMenu& menu)
{
    if(!overlays.contains(key)) return;
    overlays[key]->customMenu(menu);
}
