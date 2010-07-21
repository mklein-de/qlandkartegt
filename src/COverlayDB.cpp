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

#include "COverlayDB.h"
#include "COverlayToolWidget.h"
#include "COverlayText.h"
#include "COverlayTextBox.h"
#include "COverlayDistance.h"
#include "COverlayDistanceEditWidget.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>
#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif

COverlayDB * COverlayDB::m_self = 0;

COverlayDB::COverlayDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
, addOverlaysAsDuplicate(false)
{
    m_self      = this;
    toolview    = new COverlayToolWidget(tb);
}


COverlayDB::~COverlayDB()
{

}


IOverlay * COverlayDB::getOverlayByKey(const QString& key)
{
    if(overlays.contains(key))
    {
        return overlays[key];
    }
    return 0;
}


void COverlayDB::looseFocus()
{
    IOverlay * overlay;
    foreach(overlay, overlays) overlay->looseFocus();
}


void COverlayDB::draw(QPainter& p, const QRect& r, bool& needsRedraw)
{
    p.setRenderHint(QPainter::Antialiasing,true);
    IOverlay * overlay;
    foreach(overlay, overlays)
    {
        overlay->draw(p);
    }
    p.setRenderHint(QPainter::Antialiasing,false);
}


void COverlayDB::loadGPX(CGpx& gpx)
{
    if (gpx.version() == CGpx::qlVer_foreign)
        return;

    // QLandkarteGT file format v1.0 had more than one extensions
    // tags, so we have to scan all of them.  We can stop once we
    // found an overlay tag below it.
    QDomElement extensions = gpx.firstChildElement("gpx").firstChildElement("extensions");
    while(!extensions.isNull())
    {
        QMap<QString,QDomElement> extensionsmap = CGpx::mapChildElements(extensions);
        const QDomElement ovl = extensionsmap.value(gpx.version() == CGpx::qlVer_1_0?
            "overlays":
        (CGpx::ql_ns + ":" + "overlays"));
        if(!ovl.isNull())
        {
            QDomNodeList ovllist = ovl.childNodes();
            uint i;
            for(i = 0; i < ovllist.length(); i++)
            {
                QDomNode child = ovllist.item(i);
                QString type;
                if (child.isNull() || !child.isElement())
                {
                    continue;
                }
                if (child.prefix().isEmpty())
                {
                    type = child.nodeName();
                }
                else
                {
                    type = child.namespaceURI()+":"+child.localName();
                }
                const QDomElement element = child.toElement();

                if(type == (gpx.version() == CGpx::qlVer_1_0?
                    "text":
                    (CGpx::ql_ns + ":" + "text")))
                {
                    int top     = element.attribute("top","0").toInt();
                    int left    = element.attribute("left","0").toInt();
                    int width   = element.attribute("width","0").toInt();
                    int height  = element.attribute("height","0").toInt();

                    QRect rect(left, top, width, height);
                    if(rect.isValid())
                    {
                        QString text = element.text();
                        addText(text,rect);
                    }
                }
                else if(type == (gpx.version() == CGpx::qlVer_1_0?
                    "textbox":
                    (CGpx::ql_ns + ":" + "textbox")))
                {
                    int top     = element.attribute("top","0").toInt();
                    int left    = element.attribute("left","0").toInt();
                    int width   = element.attribute("width","0").toInt();
                    int height  = element.attribute("height","0").toInt();
                    int anchorx = element.attribute("anchorx","0").toInt();
                    int anchory = element.attribute("anchory","0").toInt();
                    double lon  = element.attribute("lon","0").toDouble() * DEG_TO_RAD;
                    double lat  = element.attribute("lat","0").toDouble() * DEG_TO_RAD;

                    QRect rect(left, top, width, height);
                    if(rect.isValid())
                    {
                        QString text = element.text();
                        addTextBox(text,lon, lat, QPoint(anchorx, anchory), rect);
                    }
                }
                else if(type == (gpx.version() == CGpx::qlVer_1_0?
                    "distance":
                    (CGpx::ql_ns + ":" + "distance")))
                {
                    QString name;
                    QString comment;
                    double speed = 0.0;
                    QList<COverlayDistance::pt_t> points;
                    QDomNodeList list;
                    QDomNode node;

                    list = element.elementsByTagName("name");
                    if(list.count() == 1)
                    {
                        node = list.item(0);
                        name = node.toElement().text();
                    }
                    list = element.elementsByTagName("comment");
                    if(list.count() == 1)
                    {
                        node = list.item(0);
                        comment = node.toElement().text();
                    }

                    list = element.elementsByTagName("speed");
                    if(list.count() == 1)
                    {
                        node = list.item(0);
                        speed = node.toElement().text().toDouble();
                    }

                    list = element.elementsByTagName("point");
                    for(int i = 0; i < list.size(); ++i)
                    {
                        COverlayDistance::pt_t pt;
                        pt.u    = list.item(i).toElement().attribute("lon", 0).toDouble() * DEG_TO_RAD;
                        pt.v    = list.item(i).toElement().attribute("lat", 0).toDouble() * DEG_TO_RAD;
                        pt.idx  = i;
                        points << pt;
                    }

                    addDistance(name, comment, speed, points);
                }

            }
            break;
        }
        extensions = extensions.nextSiblingElement("extensions");
    }
}


void COverlayDB::saveGPX(CGpx& gpx, const QStringList& keys)
{
    if (gpx.getExportFlag())
    {
        return;
    }
    if (count() == 0)
    {
        return;
    }

    QString str;
    QDomElement root        = gpx.documentElement();
    QDomElement extensions  = gpx.getExtensions();
    QDomElement _overlay_   = gpx.createElement("ql:overlays");

    extensions.appendChild(_overlay_);

    IOverlay * overlay;
    foreach(overlay, overlays)
    {

        if(overlay->type == "Text")
        {
            COverlayText * overlaytext = qobject_cast<COverlayText*>(overlay);
            if(overlaytext == 0) continue;

            QDomElement text  = gpx.createElement("ql:text");
            _overlay_.appendChild(text);

            text.setAttribute("top", overlaytext->rect.top());
            text.setAttribute("left", overlaytext->rect.left());
            text.setAttribute("width", overlaytext->rect.width());
            text.setAttribute("height", overlaytext->rect.height());

            QDomText _text_ = gpx.createTextNode(overlaytext->sometext);
            text.appendChild(_text_);
        }
        else if(overlay->type == "TextBox")
        {
            COverlayTextBox * ovl = qobject_cast<COverlayTextBox*>(overlay);
            if(ovl == 0) continue;

            QDomElement text  = gpx.createElement("ql:textbox");
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
        else if(overlay->type == "Distance")
        {
            COverlayDistance * ovl = qobject_cast<COverlayDistance*>(overlay);
            if(ovl == 0) continue;

            QDomElement elem  = gpx.createElement("ql:distance");
            _overlay_.appendChild(elem);

            QDomElement name  = gpx.createElement("ql:name");
            elem.appendChild(name);
            name.appendChild(gpx.createTextNode(ovl->name));

            QDomElement comment = gpx.createElement("ql:comment");
            elem.appendChild(comment);
            comment.appendChild(gpx.createTextNode(ovl->comment));

            QDomElement speed = gpx.createElement("ql:speed");
            elem.appendChild(speed);
            speed.appendChild(gpx.createTextNode(QString("%1").arg(ovl->speed)));

            XY pt;
            foreach(pt, ovl->points)
            {
                QDomElement point = gpx.createElement("ql:point");
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
    stream.setVersion(QDataStream::Qt_4_5);

    while(!stream.atEnd())
    {
        stream >> *this;
    }

    emit sigChanged();
}



void COverlayDB::saveQLB(CQlb& qlb)
{
    IOverlay * overlay;
    foreach(overlay, overlays)
    {
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
    foreach(key, keys)
    {
        IOverlay * overlay = overlays.take(key);
        overlay->deleteLater();
    }
    emit sigChanged();
    emit sigModified();
}


COverlayText * COverlayDB::addText(const QString& text, const QRect& rect, const QString& key)
{
    IOverlay * overlay = new COverlayText(text, rect, this);
    if(!key.isEmpty() && !addOverlaysAsDuplicate)
    {
        overlay->_key_ = key;
    }
    overlays[overlay->key()] = overlay;

    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigModified()));

    emit sigChanged();
    emit sigModified();

    return qobject_cast<COverlayText*>(overlay);
}


COverlayTextBox * COverlayDB::addTextBox(const QString& text, double lon, double lat, const QPoint& anchor, const QRect& rect, const QString& key)
{
    IOverlay * overlay = new COverlayTextBox(text, lon, lat, anchor, rect, this);
    if(!key.isEmpty() && !addOverlaysAsDuplicate)
    {
        overlay->_key_ = key;
    }
    overlays[overlay->key()] = overlay;

    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    connect(overlay, SIGNAL(sigChanged()),SIGNAL(sigModified()));

    emit sigChanged();
    emit sigModified();

    return qobject_cast<COverlayTextBox*>(overlay);
}


COverlayDistance * COverlayDB::addDistance(const QString& name, const QString& comment, double speed, const QList<COverlayDistance::pt_t>& pts, const QString& key)
{
    IOverlay * overlay = new COverlayDistance(name, comment, speed, pts, this);
    if(!key.isEmpty() && !addOverlaysAsDuplicate)
    {
        overlay->_key_ = key;
    }
    overlays[overlay->key()] = overlay;

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


void COverlayDB::copyToClipboard(bool deleteSelection)
{
    IOverlay * ovl = highlightedOverlay();

    if(ovl == 0)
    {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    CQlb qlb(this);

    qlb << *ovl;

    QBuffer buffer;
    qlb.save(&buffer);
    QMimeData *md = new QMimeData;
    buffer.open(QIODevice::ReadOnly);
    md->setData("qlandkartegt/qlb",buffer.readAll());
    buffer.close();
    clipboard->clear();
    clipboard->setMimeData(md);

}

void COverlayDB::pasteFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    qDebug() << clipboard->mimeData()->formats();
    if (clipboard->mimeData()->hasFormat("qlandkartegt/qlb"))
    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        buffer.write(clipboard->mimeData()->data("qlandkartegt/qlb"));
        buffer.close();
        CQlb qlb(this);
        qlb.load(&buffer);

        addOverlaysAsDuplicate = true;
        loadQLB(qlb);
        addOverlaysAsDuplicate = false;

        emit sigModified();
    }
}

void COverlayDB::highlightOverlay(const QString& key)
{
    QMap<QString,IOverlay*>::iterator ovl = overlays.begin();
    while(ovl != overlays.end())
    {
        (*ovl)->setHighlight(false);
        ++ovl;
    }

    if(overlays.contains(key))
    {
        IOverlay * ovl = overlays[key];

        if((ovl->type == "Distance") && overlayDistanceEditWidget)
        {
            delete overlayDistanceEditWidget;
            overlayDistanceEditWidget = new COverlayDistanceEditWidget(theMainWindow->getCanvas(), qobject_cast<COverlayDistance*>(ovl));
            theMainWindow->setTempWidget(overlayDistanceEditWidget, tr("Overlay"));
        }
        else if(overlayDistanceEditWidget)
        {
            delete overlayDistanceEditWidget;
        }

        ovl->setHighlight(true);
    }

    emit sigChanged();

}

IOverlay* COverlayDB::highlightedOverlay()
{

    QMap<QString,IOverlay*>::iterator ovl = overlays.begin();
    while(ovl != overlays.end())
    {
        if((*ovl)->isHighlighted()) return *ovl;
        ++ovl;
    }
    return 0;

}

