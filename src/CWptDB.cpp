/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de
    Copyright (C) 2010 Joerg Wunsch <j@uriah.heep.sax.de>

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

#include "CWptDB.h"
#include "CWptToolWidget.h"
#include "CDlgEditWpt.h"
#include "CWpt.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CQlb.h"
#include "CGpx.h"
#include "CResources.h"
#include "IDevice.h"
#include "CMapDB.h"
#include "IMap.h"
#include "WptIcons.h"
#include "GeoMath.h"
#include "config.h"
#include "CSettings.h"

#include <QtGui>

CWptDB * CWptDB::m_self = 0;

#ifdef HAS_EXIF
#include <libexif/exif-data.h>

typedef void (*exif_content_foreach_entry_t)(ExifContent *, ExifContentForeachEntryFunc , void *);
typedef void (*exif_data_unref_t)(ExifData *);
typedef ExifData* (*exif_data_new_from_file_t)(const char *);
typedef void (*exif_data_foreach_content_t)(ExifData *, ExifDataForeachContentFunc , void *);
typedef ExifIfd (*exif_content_get_ifd_t)(ExifContent *);
typedef ExifRational (*exif_get_rational_t)(const unsigned char *, ExifByteOrder);
typedef ExifByteOrder (*exif_data_get_byte_order_t)(ExifData *);

static exif_content_foreach_entry_t f_exif_content_foreach_entry;
static exif_data_unref_t f_exif_data_unref;
static exif_data_new_from_file_t f_exif_data_new_from_file;
static exif_data_foreach_content_t f_exif_data_foreach_content;
static exif_content_get_ifd_t f_exif_content_get_ifd;
static exif_get_rational_t f_exif_get_rational;
static exif_data_get_byte_order_t f_exif_data_get_byte_order;
#endif

CWptDB::CWptDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
, showNames(true)
{
    SETTINGS;
    showNames   = cfg.value("waypoint/showNames", showNames).toBool();

    m_self      = this;
    toolview    = new CWptToolWidget(tb);

    CQlb qlb(this);
    qlb.load(QDir::home().filePath(CONFIGDIR "sticky.qlb"));
    loadQLB(qlb, false);

#ifdef HAS_EXIF
#ifdef WIN32
    f_exif_content_foreach_entry    = (exif_content_foreach_entry_t)QLibrary::resolve("libexif-12", "exif_content_foreach_entry");
    f_exif_data_unref               = (exif_data_unref_t)QLibrary::resolve("libexif-12", "exif_data_unref");
    f_exif_data_new_from_file       = (exif_data_new_from_file_t)QLibrary::resolve("libexif-12", "exif_data_new_from_file");
    f_exif_data_foreach_content     = (exif_data_foreach_content_t)QLibrary::resolve("libexif-12", "exif_data_foreach_content");
    f_exif_content_get_ifd          = (exif_content_get_ifd_t)QLibrary::resolve("libexif-12", "exif_content_get_ifd");
    f_exif_get_rational             = (exif_get_rational_t)QLibrary::resolve("libexif-12", "exif_get_rational");
    f_exif_data_get_byte_order      = (exif_data_get_byte_order_t)QLibrary::resolve("libexif-12", "exif_data_get_byte_order");
#else
    f_exif_content_foreach_entry    = exif_content_foreach_entry;
    f_exif_data_unref               = exif_data_unref;
    f_exif_data_new_from_file       = exif_data_new_from_file;
    f_exif_data_foreach_content     = exif_data_foreach_content;
    f_exif_content_get_ifd          = exif_content_get_ifd;
    f_exif_get_rational             = exif_get_rational;
    f_exif_data_get_byte_order      = exif_data_get_byte_order;
#endif
#endif

    connect(toolview, SIGNAL(sigChanged()), SIGNAL(sigChanged()));
}


CWptDB::~CWptDB()
{
    SETTINGS;
    cfg.setValue("waypoint/showNames", showNames);

    CQlb qlb(this);

    QMap<QString, CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        if((*wpt)->sticky)
        {
            qlb << *(*wpt);
        }
        ++wpt;
    }

    qlb.save(QDir::home().filePath(CONFIGDIR "sticky.qlb"));
}

QString CWptDB::getNewWptName()
{
    const int s = lastWptName.size();
    int idx;

    if(s == 0) return lastWptName;

    for(idx = s; idx > 0; idx--)
    {
        if(!lastWptName[idx - 1].isDigit())
        {
            break;
        }
    }

    if(idx == s)
    {
        return ""; //lastWptName + "1";
    }
    else if(idx == 0)
    {
        return QString::number(lastWptName.toInt() + 1);
    }
    else
    {
        return lastWptName.left(idx) + QString::number(lastWptName.mid(idx).toInt() + 1);
    }
}

bool CWptDB::keyLessThanAlpha(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.name.toLower() < s2.name.toLower();
}

bool CWptDB::keyLessThanComment(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.comment.toLower() < s2.comment.toLower();
}

bool CWptDB::keyLessThanIcon(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    if(s1.icon.toLower() == s2.icon.toLower())
    {
        return keyLessThanAlpha(s1,s2);
    }
    return s1.icon.toLower() < s2.icon.toLower();
}

bool CWptDB::keyLessThanDistance(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.d < s2.d;
}

bool CWptDB::keyLessThanTime(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.time < s2.time;
}


void CWptDB::clear()
{
    delWpt(wpts.keys());
    CWpt::resetKeyCnt();
    emit sigChanged();
}


QList<CWptDB::keys_t> CWptDB::keys()
{
    QList<keys_t> k;

    QString k1;
    QStringList ks = wpts.keys();

    foreach(k1, ks)
    {
        keys_t k2;

        k2.key      = k1;
        k2.name     = wpts[k1]->name;
        k2.time     = wpts[k1]->timestamp;

        // pick comment/descripton, what ever fits
        if(wpts[k1]->comment.isEmpty())
        {
            k2.comment  = wpts[k1]->description;
        }
        else
        {
            k2.comment  = wpts[k1]->comment;
        }

        k2.comment.remove(QRegExp("<head.*[^>]*><\\/head>"));
        k2.comment.remove(QRegExp("<[^>]*>"));
        k2.comment = k2.comment.simplified();

        // truncate comment if necessary
        if(k2.comment.size() > 33)
        {
            k2.comment = k2.comment.left(30) + "...";
        }

        if(wpts[k1]->geocache.hasData)
        {
            k2.icon     = wpts[k1]->geocache.type;
        }
        else
        {
            k2.icon     = wpts[k1]->iconString;
        }
        k2.lon      = wpts[k1]->lon;
        k2.lat      = wpts[k1]->lat;
        k2.d        = 0;
        k2.isCache  = wpts[k1]->geocache.hasData;

        k << k2;
    }

    QString pos;
    CWptToolWidget::sortmode_e sortmode = CWptToolWidget::getSortMode(pos);

    switch(sortmode)
    {
        case CWptToolWidget::eSortByName:
            qSort(k.begin(), k.end(), CWptDB::keyLessThanAlpha);
            break;
        case CWptToolWidget::eSortByTime:
            qSort(k.begin(), k.end(), CWptDB::keyLessThanTime);
            break;
        case CWptToolWidget::eSortByComment:
            qSort(k.begin(), k.end(), CWptDB::keyLessThanComment);
            break;
        case CWptToolWidget::eSortByIcon:
            qSort(k.begin(), k.end(), CWptDB::keyLessThanIcon);
            break;
        case CWptToolWidget::eSortByDistance:
            {
                projXY p1, p2;
                float lon, lat;
                GPS_Math_Str_To_Deg(pos, lon, lat, true);
                p1.u = lon * DEG_TO_RAD;
                p1.v = lat * DEG_TO_RAD;

                QList<CWptDB::keys_t>::iterator _k = k.begin();
                while(_k != k.end())
                {
                    double a1 = 0, a2 = 0;

                    p2.u = _k->lon * DEG_TO_RAD;
                    p2.v = _k->lat * DEG_TO_RAD;

                    _k->d = distance(p1, p2, a1, a2);
                    ++_k;
                }
                qSort(k.begin(), k.end(), CWptDB::keyLessThanDistance);
            }
            break;
    }

    return k;
}


CWpt * CWptDB::newWpt(float lon, float lat, float ele, const QString& name)
{
    CWpt * wpt  = new CWpt(this);
    wpt->lon    = lon * RAD_TO_DEG;
    wpt->lat    = lat * RAD_TO_DEG;
    wpt->ele    = ele;
    wpt->name   = name;

    SETTINGS;
    wpt->setIcon(cfg.value("waypoint/lastSymbol","").toString());

    if(name.isEmpty())
    {
        CDlgEditWpt dlg(*wpt,theMainWindow->getCanvas());
        if(dlg.exec() == QDialog::Rejected)
        {
            delete wpt;
            return 0;
        }
    }
    wpts[wpt->getKey()] = wpt;

    cfg.setValue("waypoint/lastSymbol",wpt->iconString);

    emit sigChanged();
    emit sigModified();

    lastWptName = wpt->getName();

    return wpt;
}


CWpt * CWptDB::getWptByKey(const QString& key)
{
    if(!wpts.contains(key)) return 0;

    return wpts[key];

}


void CWptDB::delWpt(const QString& key, bool silent, bool saveSticky)
{
    if(!wpts.contains(key)) return;

    if(wpts[key]->sticky)
    {

        if(saveSticky) return;

        QString msg = tr("Do you really want to delete the sticky waypoint '%1'").arg(wpts[key]->name);
        if(QMessageBox::question(0,tr("Delete sticky waypoint ..."),msg, QMessageBox::Ok|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }
    CWpt * wpt =  wpts.take(key);
    wpt->deleteLater();
    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}


void CWptDB::delWpt(const QStringList& keys, bool saveSticky)
{
    QString key;
    foreach(key, keys)
    {
        delWpt(key,true, saveSticky);
    }

    if(!keys.isEmpty())
    {
        emit sigChanged();
        emit sigModified();
    }
}


void CWptDB::addWpt(CWpt * wpt, bool silent)
{
    if(wpts.contains(wpt->getKey()))
    {
        if(wpts[wpt->getKey()]->sticky)
        {
            delete wpt;
            return;
        }
        else
        {
            delWpt(wpt->getKey(), true);
        }
    }
    wpts[wpt->getKey()] = wpt;

    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}


void CWptDB::setProxyDistance(const QStringList& keys, double dist)
{
    QString key;
    foreach(key,keys)
    {
        wpts[key]->prx = dist;
        emit sigModified(key);
    }
    emit sigChanged();
    emit sigModified();

}

void CWptDB::setIcon(const QStringList& keys, const QString& iconName)
{
    QString key;
    foreach(key,keys)
    {
        CWpt * wpt = wpts[key];

        // no icon change for geocaches
        if(!wpt->geocache.hasData)
        {
            wpt->setIcon(iconName);
            emit sigModified(key);
        }
    }
    emit sigChanged();
    emit sigModified();

}

void CWptDB::loadGPX(CGpx& gpx)
{
    bool hasItems = false;
    const QDomNodeList& waypoints = gpx.elementsByTagName("wpt");
    uint N = waypoints.count();

    for(uint n = 0; n < N; ++n)
    {
        hasItems = true;
        const QDomNode& waypoint = waypoints.item(n);

        CWpt * wpt = new CWpt(this);

        const QDomNamedNodeMap& attr = waypoint.attributes();
        wpt->lon = attr.namedItem("lon").nodeValue().toDouble();
        wpt->lat = attr.namedItem("lat").nodeValue().toDouble();
        if(waypoint.namedItem("name").isElement())
        {
            wpt->name = waypoint.namedItem("name").toElement().text();
        }
        if(waypoint.namedItem("cmt").isElement())
        {
            wpt->comment = waypoint.namedItem("cmt").toElement().text();
        }
        if(waypoint.namedItem("desc").isElement())
        {
            wpt->description = waypoint.namedItem("desc").toElement().text();
        }
        if(waypoint.namedItem("link").isElement())
        {
            const QDomNode& link = waypoint.namedItem("link");
            const QDomNamedNodeMap& attr = link.toElement().attributes();
            wpt->link = attr.namedItem("href").nodeValue();
        }
        if(waypoint.namedItem("url").isElement())
        {
            wpt->link = waypoint.namedItem("url").toElement().text();
        }
        if(waypoint.namedItem("urlname").isElement())
        {
            wpt->urlname = waypoint.namedItem("urlname").toElement().text();
        }
        if(waypoint.namedItem("sym").isElement())
        {
            wpt->setIcon(waypoint.namedItem("sym").toElement().text());
        }
        if(waypoint.namedItem("ele").isElement())
        {
            wpt->ele = waypoint.namedItem("ele").toElement().text().toDouble();
        }
        if(waypoint.namedItem("time").isElement())
        {
          QString timetext = waypoint.namedItem("time").toElement().text();
          (void)parseTimestamp(timetext, wpt->timestamp);
        }
        if(waypoint.namedItem("type").isElement())
        {
            wpt->type = waypoint.namedItem("type").toElement().text();
        }

        if(waypoint.namedItem("parent").isElement())
        {
            wpt->setParentWpt(waypoint.namedItem("parent").toElement().text());
        }

        if(waypoint.namedItem("extensions").isElement())
        {
            QDomElement tmpelem;
            const QDomNode& ext = waypoint.namedItem("extensions");
            QMap<QString,QDomElement> extensionsmap = CGpx::mapChildElements(ext);


            // old garmin proximity format namespace
            tmpelem = extensionsmap.value(CGpx::gpxx_ns + ":" + "WaypointExtension");
            if(!tmpelem.isNull())
            {
                QMap<QString,QDomElement> waypointextensionmap = CGpx::mapChildElements(tmpelem);

                tmpelem = waypointextensionmap.value(CGpx::gpxx_ns + ":" + "Proximity");
                if(!tmpelem.isNull())
                {
                    wpt->prx = tmpelem.text().toDouble();
                }
            }

            // new garmin proximity format namespace
            tmpelem = extensionsmap.value(CGpx::wptx1_ns + ":" + "WaypointExtension");
            if(!tmpelem.isNull())
            {
                QMap<QString,QDomElement> waypointextensionmap = CGpx::mapChildElements(tmpelem);

                tmpelem = waypointextensionmap.value(CGpx::wptx1_ns + ":" + "Proximity");
                if(!tmpelem.isNull())
                {
                    wpt->prx = tmpelem.text().toDouble();
                }
            }

            // parent (cache) waypoint
            tmpelem = extensionsmap.value(CGpx::ql_ns + ":" + "parent");
            if(!tmpelem.isNull())
            {
                wpt->setParentWpt(tmpelem.text());
            }

            // the qlgt internal item key
            tmpelem = extensionsmap.value(CGpx::ql_ns + ":" + "key");
            if(!tmpelem.isNull())
            {
                wpt->setKey(tmpelem.text());
            }

            // QLandkarteGT backward compatibility
            if (gpx.version() == CGpx::qlVer_1_0)
            {
                if(ext.namedItem("dist").isElement())
                {
                    wpt->prx = ext.namedItem("dist").toElement().text().toDouble();
                }
            }
        }

        if(wpt->lat == 1000 || wpt->lon == 1000 || (wpt->name.isEmpty() && wpt->comment.isEmpty()))
        {
            delete wpt;
            continue;
        }

        wpt->loadGpxExt(waypoint);

        addWpt(wpt, true);
    }

    CWpt::resetKeyCnt();

    if(hasItems)
    {
        emit sigChanged();
    }
}


void CWptDB::saveGPX(CGpx& gpx, const QStringList& keys)
{
    QString str;
    QDomElement root = gpx.documentElement();

    QList<keys_t> _keys = this->keys();
    QList<keys_t>::const_iterator _key = _keys.begin();

    while(_key != _keys.end())
    {
        CWpt * wpt = wpts[_key->key];

        // sticky waypoints are not saved
        if(wpt->sticky)
        {
            ++_key;
            continue;
        }

        // waypoint filtering
        // if keys list is not empty the waypoints key has to be in the list
        if(!keys.isEmpty() && !keys.contains(wpt->getKey()))
        {
            ++_key;
            continue;
        }

        QDomElement waypoint = gpx.createElement("wpt");
        root.appendChild(waypoint);
        str.sprintf("%1.8f", wpt->lat);
        waypoint.setAttribute("lat",str);
        str.sprintf("%1.8f", wpt->lon);
        waypoint.setAttribute("lon",str);

        if(wpt->ele != 1e25f)
        {
            QDomElement ele = gpx.createElement("ele");
            waypoint.appendChild(ele);
            QDomText _ele_ = gpx.createTextNode(QString::number(wpt->ele));
            ele.appendChild(_ele_);
        }

        QDateTime t = QDateTime::fromTime_t(wpt->timestamp);
        QDomElement time = gpx.createElement("time");
        waypoint.appendChild(time);
        QDomText _time_ = gpx.createTextNode(t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'"));
        time.appendChild(_time_);

        QDomElement name = gpx.createElement("name");
        waypoint.appendChild(name);
        QDomText _name_ = gpx.createTextNode(wpt->name);
        name.appendChild(_name_);

        if(!wpt->comment.isEmpty())
        {
            QString comment = wpt->comment;
            comment.remove(QRegExp("<head.*[^>]*><\\/head>"));
            comment.remove(QRegExp("<[^>]*>"));
            comment = comment.simplified();


            QDomElement cmt = gpx.createElement("cmt");
            waypoint.appendChild(cmt);
            QDomText _cmt_ = gpx.createTextNode(comment);
            cmt.appendChild(_cmt_);
        }

        if(!wpt->description.isEmpty())
        {
            QDomElement desc = gpx.createElement("desc");
            waypoint.appendChild(desc);
            QDomText _desc_ = gpx.createTextNode(wpt->description);
            desc.appendChild(_desc_);
        }

        if(!wpt->link.isEmpty() && wpt->urlname.isEmpty())
        {
            QDomElement link = gpx.createElement("link");
            waypoint.appendChild(link);
            link.setAttribute("href",wpt->link);
            QDomElement text = gpx.createElement("text");
            link.appendChild(text);
            QDomText _text_ = gpx.createTextNode(wpt->name);
            text.appendChild(_text_);
        }

        if(!wpt->link.isEmpty() && !wpt->urlname.isEmpty())
        {
            QDomElement url = gpx.createElement("url");
            waypoint.appendChild(url);
            QDomText _url_ = gpx.createTextNode(wpt->link);
            url.appendChild(_url_);

            QDomElement urlname = gpx.createElement("urlname");
            waypoint.appendChild(urlname);
            QDomText _urlname_ = gpx.createTextNode(wpt->urlname);
            urlname.appendChild(_urlname_);
        }

        QDomElement sym = gpx.createElement("sym");
        waypoint.appendChild(sym);
        QDomText _sym_ = gpx.createTextNode(wpt->iconString);
        sym.appendChild(_sym_);

        if(!wpt->type.isEmpty())
        {
            QDomElement type = gpx.createElement("type");
            waypoint.appendChild(type);
            QDomText _type_ = gpx.createTextNode(wpt->type);
            type.appendChild(_type_);
        }

        if(gpx.getExportMode() == CGpx::eOcmExport)
        {
            if(!wpt->getParentWpt().isEmpty())
            {
                QDomElement parent = gpx.createElement("parent");
                parent.setAttribute("xmlns", "http://opencachemanage.sourceforge.net/schema1");
                waypoint.appendChild(parent);
                QDomText _parent_ = gpx.createTextNode(wpt->getParentWpt());
                parent.appendChild(_parent_);
            }
        }


        QDomElement extensions = gpx.createElement("extensions");
        waypoint.appendChild(extensions);
        if(!wpt->getParentWpt().isEmpty())
        {
            QDomElement gpxx_ext = gpx.createElement("gpxx:WaypointExtension");
            extensions.appendChild(gpxx_ext);


            if(!wpt->getParentWpt().isEmpty() && (gpx.getExportMode() == CGpx::eQlgtExport))
            {
                QDomElement parent = gpx.createElement("ql:parent");
                extensions.appendChild(parent);
                QDomText _parent_ = gpx.createTextNode(wpt->getParentWpt());
                parent.appendChild(_parent_);
            }
        }

        if(wpt->prx != 1e25f)
        {
            QDomElement gpxx_ext = gpx.createElement("wptx1:WaypointExtension");
            extensions.appendChild(gpxx_ext);

            QDomElement proximity = gpx.createElement("wptx1:Proximity");
            gpxx_ext.appendChild(proximity);
            QDomText _proximity_ = gpx.createTextNode(QString::number(wpt->prx));
            proximity.appendChild(_proximity_);

        }

        QDomElement qlkey = gpx.createElement("ql:key");
        extensions.appendChild(qlkey);
        QDomText _qlkey_ = gpx.createTextNode(wpt->getKey());
        qlkey.appendChild(_qlkey_);


        wpt->saveGpxExt(waypoint, gpx.getExportMode());

        // export buddy waypoints
        if(wpt->geocache.exportBuddies)
        {
            wpt->showBuddies(true);
            const QList<CWpt::buddy_t>& buddies = wpt->buddies;
            foreach(const CWpt::buddy_t& buddy, buddies)
            {
                QDomElement waypoint = gpx.createElement("wpt");
                root.appendChild(waypoint);
                str.sprintf("%1.8f", buddy.lat*RAD_TO_DEG);
                waypoint.setAttribute("lat",str);
                str.sprintf("%1.8f", buddy.lon*RAD_TO_DEG);
                waypoint.setAttribute("lon",str);

                QDomElement name = gpx.createElement("name");
                waypoint.appendChild(name);
                QDomText _name_ = gpx.createTextNode(buddy.name);
                name.appendChild(_name_);

                QDomElement sym = gpx.createElement("sym");
                waypoint.appendChild(sym);
                QDomText _sym_ = gpx.createTextNode("Civil");
                sym.appendChild(_sym_);

                if(gpx.getExportMode() == CGpx::eOcmExport)
                {
                    QDomElement parent = gpx.createElement("parent");
                    parent.setAttribute("xmlns", "http://opencachemanage.sourceforge.net/schema1");
                    waypoint.appendChild(parent);
                    QDomText _parent_ = gpx.createTextNode(wpt->getName());
                    parent.appendChild(_parent_);
                }
                else if(gpx.getExportMode() == CGpx::eQlgtExport)
                {
                    QDomElement extensions = gpx.createElement("extensions");
                    waypoint.appendChild(extensions);

                    QDomElement parent = gpx.createElement("ql:parent");
                    extensions.appendChild(parent);
                    QDomText _parent_ = gpx.createTextNode(wpt->getName());
                    parent.appendChild(_parent_);
                }
            }

            wpt->showBuddies(false);
        }


        ++_key;
    }
}


void CWptDB::loadQLB(CQlb& qlb, bool newKey)
{
    QDataStream stream(&qlb.waypoints(),QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    while(!stream.atEnd())
    {
        CWpt * wpt = new CWpt(this);
        stream >> *wpt;

        if(newKey)
        {
            wpt->setKey(wpt->getKey() + QString("%1").arg(QDateTime::currentDateTime().toTime_t()));
        }

        addWpt(wpt,true);
    }

    if(qlb.waypoints().size())
    {
        emit sigChanged();
    }

}


void CWptDB::saveQLB(CQlb& qlb)
{
    QMap<QString, CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        qlb << *(*wpt);
        ++wpt;
    }
}


void CWptDB::upload(const QStringList& keys)
{
    if(wpts.isEmpty()) return;

    IDevice * dev = CResources::self().device();
    if(dev)
    {
        QList<CWpt*> tmpwpts;

        if(keys.isEmpty())
        {
            tmpwpts = wpts.values();
        }
        else
        {
            QString key;
            foreach(key, keys)
            {
                tmpwpts << wpts[key];
            }
        }

        dev->uploadWpts(tmpwpts);
    }

}


void CWptDB::download()
{
    IDevice * dev = CResources::self().device();
    if(dev)
    {
        QList<CWpt*> tmpwpts;
        dev->downloadWpts(tmpwpts);

        if(tmpwpts.isEmpty()) return;

        CWpt * wpt;
        foreach(wpt,tmpwpts)
        {
            addWpt(wpt,true);
        }
    }

    emit sigChanged();
    emit sigModified();
}


void CWptDB::selWptByKey(const QString& key, bool selectMode)
{
    CWptToolWidget * t = qobject_cast<CWptToolWidget*>(toolview);
    if(t)
    {
        if(selectMode && wpts.contains(key))
        {
            wpts[key]->selected =! wpts[key]->selected;
        }

        t->selWptByKey(key);
        gainFocus();
    }
}

void CWptDB::selWptInRange(const QPointF& center, double radius)
{
    projXY p0;
    p0.u = center.x();
    p0.v = center.y();
    CWptToolWidget * t = qobject_cast<CWptToolWidget*>(toolview);

    foreach(CWpt * wpt, wpts)
    {
        projXY p1;
        p1.u = wpt->lon * DEG_TO_RAD;
        p1.v = wpt->lat * DEG_TO_RAD;

        double d,a1,a2;
        d = distance(p0, p1, a1, a2);

        if(d < radius)
        {
            wpt->selected = true;
            if(t)
            {
                t->selWptByKey(wpt->getKey());
            }
        }
    }
}


void CWptDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    IMap& map = CMapDB::self().getMap();
    QColor color = CResources::self().wptTextColor();

    QPixmap pixOk(":/icons/iconOk10x10.png");

    // added by AD
    QList<QRect> blockAreas;
    QFontMetrics fm(CResources::self().getMapFont());
    // end added by AD

    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect.contains(QPoint(u,v)))
        {
            QPixmap icon = (*wpt)->getIcon();
            QPixmap back = QPixmap(icon.size());
            back.fill(Qt::white);
            back.setMask(icon.alphaChannel().createMaskFromColor(Qt::black));

            int o =  icon.width() >>1;

            // draw waypoint icon

            p.drawPixmap(u - (o + 1) , v - (o + 1), back);
            p.drawPixmap(u - (o + 1) , v - (o + 0), back);
            p.drawPixmap(u - (o + 1) , v - (o - 1), back);

            p.drawPixmap(u - (o + 0) , v - (o + 1), back);
            p.drawPixmap(u - (o + 0) , v - (o - 1), back);

            p.drawPixmap(u - (o - 1) , v - (o + 1), back);
            p.drawPixmap(u - (o - 1) , v - (o + 0), back);
            p.drawPixmap(u - (o - 1) , v - (o - 1), back);

            p.drawPixmap(u - o , v - o, icon);

            if((*wpt)->selected)
            {
                p.setPen(Qt::NoPen);
                p.setBrush(Qt::white);
                p.drawEllipse(u - 2, v - 2, 14, 14);
                p.drawPixmap(u, v, pixOk);
            }

            // added by AD
            blockAreas << QRect(u - o , v - o, icon.width(), icon.height());
        }

        if(!(*wpt)->buddies.isEmpty())
        {
            QPixmap icon(":/icons/iconWaypoint16x16.png");
            CWpt::buddy_t buddy;
            QList<CWpt::buddy_t>& buddies = (*wpt)->buddies;
            foreach(buddy, buddies)
            {
                double x = buddy.lon;
                double y = buddy.lat;
                map.convertRad2Pt(x,y);

                p.setBrush(CCanvas::brushBackWhite);
                p.setPen(CCanvas::penBorderBlue);
                p.drawLine(u,v,x,y);
                p.drawEllipse(QPoint(x,y),12,12);
                p.drawPixmap(x - 8, y - 8, icon);
            }
        }

        ++wpt;
    }

    // added by AD
    // draw the labels
    wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect.contains(QPoint(u,v)))
        {
            QPixmap icon = (*wpt)->getIcon();

            if(showNames)
            {
                QString name;

                if((*wpt)->isGeoCache())
                {
                    name = (*wpt)->getGeocacheData().name + " (" + (*wpt)->getName() + ")";
                }
                else
                {
                    name = (*wpt)->getName();
                }

                QRect textArea = fm.boundingRect(name);
                bool intersects;

                // try above
                textArea.moveCenter(QPoint(u, v - (icon.height() >> 1) - textArea.height()));
                intersects = false;
                for (int k = 0; k < blockAreas.size() && !intersects; ++k)
                {
                    intersects = textArea.intersects(blockAreas.at(k));
                }

                if (intersects)
                {
                    // try below
                    textArea.moveCenter(QPoint(u, v + ((icon.height() + textArea.height()) >> 1)));
                    intersects = false;
                    for (int k = 0; k < blockAreas.size() && !intersects; ++k)
                    {
                        intersects = textArea.intersects(blockAreas.at(k));
                    }

                    if (intersects)
                    {
                        // try right
                        textArea.moveCenter(QPoint(u + ((icon.height() + textArea.height() + textArea.width()) >> 1),
                                                   v - (textArea.height() >> 2)));
                        intersects = false;
                        for (int k = 0; k < blockAreas.size() && !intersects; ++k)
                        {
                            intersects = textArea.intersects(blockAreas.at(k));
                        }

                        if (intersects)
                        {
                            // try left
                            textArea.moveCenter(QPoint(u - ((icon.height() + (textArea.height() >> 1) + textArea.width()) >> 1),
                                                       v - (textArea.height() >> 2)));
                            intersects = false;
                            for (int k = 0; k < blockAreas.size() && !intersects; ++k)
                            {
                                intersects = textArea.intersects(blockAreas.at(k));
                            }
                        }
                    }
                }

                if (!intersects)
                {
                    blockAreas << textArea;
                    // compensate for drawText magic and plot
                    textArea.translate(0, textArea.height());
                    CCanvas::drawText(name, p, textArea.center(), color);
                }
            }
        }
        ++wpt;
    }
    // end added by AD

    wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        if((*wpt)->prx != WPT_NOFLOAT)
        {
            projXY pt1, pt2;

            double u = (*wpt)->lon * DEG_TO_RAD;
            double v = (*wpt)->lat * DEG_TO_RAD;
            map.convertRad2Pt(u,v);

            pt1.u = (*wpt)->lon * DEG_TO_RAD;
            pt1.v = (*wpt)->lat * DEG_TO_RAD;
            pt2 = GPS_Math_Wpt_Projection(pt1, (*wpt)->prx, 90 * DEG_TO_RAD);
            map.convertRad2Pt(pt2.u,pt2.v);
            double r = pt2.u - u;

            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::white,3));
            p.drawEllipse(QRect(u - r - 1, v - r - 1, 2*r + 1, 2*r + 1));
            p.setPen(QPen(Qt::red,1));
            p.drawEllipse(QRect(u - r - 1, v - r - 1, 2*r + 1, 2*r + 1));
        }
        ++wpt;
    }
}


#ifdef HAS_EXIF
static void exifContentForeachEntryFuncGPS(ExifEntry * exifEntry, void *user_data)
{
    CWptDB::exifGPS_t& exifGPS = *(CWptDB::exifGPS_t*)user_data;

    switch(exifEntry->tag)
    {
        case EXIF_TAG_GPS_LATITUDE_REF:
        {
            if(exifEntry->data[0] != 'N')
            {
                exifGPS.lat_sign = -1;
            }
            break;
        }
        case EXIF_TAG_GPS_LATITUDE:
        {
            if(exifEntry->components == 3)
            {
                ExifRational * p = (ExifRational*)exifEntry->data;
                ExifRational deg = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                ExifRational min = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                ExifRational sec = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                exifGPS.lat = (double)deg.numerator/deg.denominator + (double)min.numerator/(min.denominator * 60) + (double)sec.numerator/((double)sec.denominator * 3600.0);
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE_REF:
        {
            if(exifEntry->data[0] != 'E')
            {
                exifGPS.lon_sign = -1;
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE:
        {
            if(exifEntry->components == 3)
            {
                ExifRational * p = (ExifRational*)exifEntry->data;
                ExifRational deg = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                ExifRational min = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                ExifRational sec = f_exif_get_rational((const unsigned char*)p++, exifGPS.byte_order);
                exifGPS.lon = (double)deg.numerator/deg.denominator + (double)min.numerator/(min.denominator * 60) + (double)sec.numerator/((double)sec.denominator * 3600.0);
            }
            break;
        }
        default:;
    }
}


static void exifContentForeachEntryFunc0(ExifEntry * exifEntry, void *user_data)
{
    CWptDB::exifGPS_t& exifGPS = *(CWptDB::exifGPS_t*)user_data;

    switch(exifEntry->tag)
    {
        case EXIF_TAG_DATE_TIME:
        {
            //             qDebug() << exifEntry->format << exifEntry->components << exifEntry->size;
            //             qDebug() << (char*)exifEntry->data;
            //             2009:05:23 14:12:10
            QDateTime timestamp = QDateTime::fromString((char*)exifEntry->data, "yyyy:MM:dd hh:mm:ss");
            exifGPS.timestamp   = timestamp.toTime_t();
            break;
        }
        default:;
    }

}


static void exifDataForeachContentFunc(ExifContent * exifContent, void * user_data)
{
    switch(f_exif_content_get_ifd(exifContent))
    {

        case EXIF_IFD_0:
            f_exif_content_foreach_entry(exifContent, exifContentForeachEntryFunc0, user_data);
            break;

        case EXIF_IFD_GPS:
            f_exif_content_foreach_entry(exifContent, exifContentForeachEntryFuncGPS, user_data);
            break;

        default:;

    }
    //     qDebug() << "***" << exif_content_get_ifd(exifContent) << "***";
    //     exif_content_dump(exifContent,0);
}


void CWptDB::createWaypointsFromImages()
{

    if(f_exif_data_new_from_file == 0)
    {
#ifdef WIN32
        QMessageBox::warning(0,tr("Missing libexif"), tr("Unable to find libexif-12.dll."), QMessageBox::Abort, QMessageBox::Abort);
#else
        QMessageBox::warning(0,tr("Missing libexif"), tr("Unable to find libexif.so."), QMessageBox::Abort, QMessageBox::Abort);
#endif
        return;
    }

    SETTINGS;
    QString path = cfg.value("path/images", "./").toString();
    path = QFileDialog::getExistingDirectory(0, tr("Select path..."), path, FILE_DIALOG_FLAGS);

    if(path.isEmpty()) return;

    cfg.setValue("path/images", path);

    QDir dir(path);
    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png";
    QStringList files = dir.entryList(filter, QDir::Files);
    QString file;

    quint32 progCnt = 0;
    QProgressDialog progress(tr("Read EXIF tags from pictures."), tr("Abort"), 0, files.size());


    foreach(file, files)
    {
        //         qDebug() << "---------------" << file << "---------------";

        progress.setValue(progCnt++);
        if (progress.wasCanceled()) break;
        qApp->processEvents();


        ExifData * exifData = f_exif_data_new_from_file(dir.filePath(file).toLocal8Bit());

        ExifByteOrder exifByteOrder = f_exif_data_get_byte_order(exifData);

        exifGPS_t exifGPS(exifByteOrder);

        f_exif_data_foreach_content(exifData, exifDataForeachContentFunc, &exifGPS);

        CWpt * wpt      = new CWpt(this);
        wpt->lon        = exifGPS.lon * exifGPS.lon_sign;
        wpt->lat        = exifGPS.lat * exifGPS.lat_sign;
        wpt->timestamp  = exifGPS.timestamp;
        wpt->setIcon("Flag, Red");
        wpt->name       = file;

        CWpt::image_t image;

        QPixmap pixtmp(dir.filePath(file).toLocal8Bit());
        int w = pixtmp.width();
        int h = pixtmp.height();

        if(w < h)
        {
            h *= 240.0 / w;
            w  = 240;
        }
        else
        {
            h *= 320.0 / w;
            w  = 320;
        }

        image.filePath  = dir.filePath(file).toLocal8Bit();
        image.pixmap    = pixtmp.scaled(w,h,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        image.info      = file;
        wpt->images << image;
        addWpt(wpt, true);

        f_exif_data_unref(exifData);
    }
    emit sigChanged();
    emit sigModified();
}
#endif

void CWptDB::makeVisible(const QStringList& keys)
{


    if(keys.isEmpty())
    {
        return;
    }

    QRectF r;
    QString key;
    foreach(key, keys)
    {

        CWpt * wpt =  wpts[key];

        if(r.isNull())
        {
            r = QRectF(wpt->lon, wpt->lat, 0.0001, 0.0001);
        }
        else
        {
            r |= QRectF(wpt->lon, wpt->lat, 0.0001, 0.0001);
        }

    }

    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }


}

void CWptDB::getListOfGeoCaches(QStringList& caches)
{
    keys_t _key_;
    QList<CWptDB::keys_t> _keys_ = keys();

    caches.clear();

    foreach(_key_, _keys_)
    {
        if(_key_.isCache)
        {
            caches << _key_.name;
        }
    }

}
