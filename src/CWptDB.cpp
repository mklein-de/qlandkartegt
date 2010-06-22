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

#include <QtGui>

CWptDB * CWptDB::m_self = 0;

#ifdef HAS_EXIF
#include <libexif/exif-data.h>

typedef void (*exif_content_foreach_entry_t)(ExifContent *, ExifContentForeachEntryFunc , void *);
typedef void (*exif_data_unref_t)(ExifData *);
typedef ExifData* (*exif_data_new_from_file_t)(const char *);
typedef void (*exif_data_foreach_content_t)(ExifData *, ExifDataForeachContentFunc , void *);
typedef ExifIfd (*exif_content_get_ifd_t)(ExifContent *);

static exif_content_foreach_entry_t f_exif_content_foreach_entry;
static exif_data_unref_t f_exif_data_unref;
static exif_data_new_from_file_t f_exif_data_new_from_file;
static exif_data_foreach_content_t f_exif_data_foreach_content;
static exif_content_get_ifd_t f_exif_content_get_ifd;
#endif

CWptDB::CWptDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CWptToolWidget(tb);

    CQlb qlb(this);
    qlb.load(QDir::home().filePath(CONFIGDIR "sticky.qlb"));
    loadQLB(qlb);

#ifdef HAS_EXIF
#ifdef WIN32
    f_exif_content_foreach_entry    = (exif_content_foreach_entry_t)QLibrary::resolve("libexif-12", "exif_content_foreach_entry");
    f_exif_data_unref               = (exif_data_unref_t)QLibrary::resolve("libexif-12", "exif_data_unref");
    f_exif_data_new_from_file       = (exif_data_new_from_file_t)QLibrary::resolve("libexif-12", "exif_data_new_from_file");
    f_exif_data_foreach_content     = (exif_data_foreach_content_t)QLibrary::resolve("libexif-12", "exif_data_foreach_content");
    f_exif_content_get_ifd          = (exif_content_get_ifd_t)QLibrary::resolve("libexif-12", "exif_content_get_ifd");
#else
    f_exif_content_foreach_entry    = (exif_content_foreach_entry_t)QLibrary::resolve("libexif", "exif_content_foreach_entry");
    f_exif_data_unref               = (exif_data_unref_t)QLibrary::resolve("libexif", "exif_data_unref");
    f_exif_data_new_from_file       = (exif_data_new_from_file_t)QLibrary::resolve("libexif", "exif_data_new_from_file");
    f_exif_data_foreach_content     = (exif_data_foreach_content_t)QLibrary::resolve("libexif", "exif_data_foreach_content");
    f_exif_content_get_ifd          = (exif_content_get_ifd_t)QLibrary::resolve("libexif", "exif_content_get_ifd");
#endif
#endif

}


CWptDB::~CWptDB()
{
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
    return s1.icon.toLower() < s2.icon.toLower();
}

bool CWptDB::keyLessThanDistance(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.d < s2.d;
}


void CWptDB::clear()
{
    delWpt(wpts.keys());
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
        k2.comment  = wpts[k1]->comment.left(32);
        k2.icon     = wpts[k1]->icon;
        k2.lon      = wpts[k1]->lon;
        k2.lat      = wpts[k1]->lat;
        k2.d        = 0;

        k << k2;
    }

    return k;
}


CWpt * CWptDB::newWpt(float lon, float lat, float ele)
{
    CWpt * wpt = new CWpt(this);
    wpt->lon = lon * RAD_TO_DEG;
    wpt->lat = lat * RAD_TO_DEG;
    wpt->ele = ele;

    QSettings cfg;
    wpt->icon = cfg.value("waypoint/lastSymbol","").toString();

    CDlgEditWpt dlg(*wpt,theMainWindow->getCanvas());
    if(dlg.exec() == QDialog::Rejected)
    {
        delete wpt;
        return 0;
    }
    wpts[wpt->key()] = wpt;

    cfg.setValue("waypoint/lastSymbol",wpt->icon);

    emit sigChanged();
    emit sigModified();

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
    delete wpts.take(key);
    if(!silent) emit sigChanged();
    emit sigModified();
}


void CWptDB::delWpt(const QStringList& keys, bool saveSticky)
{
    QString key;
    foreach(key, keys)
    {
        delWpt(key,true, saveSticky);
    }

    emit sigChanged();
    emit sigModified();
}


void CWptDB::addWpt(CWpt * wpt, bool silent)
{
    if(wpts.contains(wpt->key()))
    {
        if(wpts[wpt->key()]->sticky)
        {
            delete wpt;
            return;
        }
        else
        {
            delWpt(wpt->key(), true);
        }
    }
    wpts[wpt->key()] = wpt;

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
    }
    emit sigChanged();
    emit sigModified();
}


void CWptDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& waypoints = gpx.elementsByTagName("wpt");
    uint N = waypoints.count();
    for(uint n = 0; n < N; ++n)
    {
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
            wpt->comment = waypoint.namedItem("desc").toElement().text();
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
        if(waypoint.namedItem("sym").isElement())
        {
            wpt->icon =  waypoint.namedItem("sym").toElement().text();
        }
        if(waypoint.namedItem("ele").isElement())
        {
            wpt->ele = waypoint.namedItem("ele").toElement().text().toDouble();
        }
        if(waypoint.namedItem("time").isElement())
        {
            QDateTime time = QDateTime::fromString(waypoint.namedItem("time").toElement().text(),"yyyy-MM-dd'T'hh:mm:ss'Z'");
            time.setTimeSpec(Qt::UTC);
                                 // - gpResources->getUTCOffset();
            wpt->timestamp = time.toTime_t();
        }

        if(waypoint.namedItem("extensions").isElement())
        {
            const QDomNode& ext = waypoint.namedItem("extensions");
            QMap<QString,QDomElement> extensionsmap = CGpx::mapChildElements(ext);
            QDomElement tmpelem = extensionsmap.value(CGpx::gpxx_ns + ":" + "WaypointExtension");

            if(!tmpelem.isNull())
            {
                QMap<QString,QDomElement> waypointextensionmap = CGpx::mapChildElements(tmpelem);

                tmpelem = waypointextensionmap.value(CGpx::gpxx_ns + ":" + "Proximity");
                if(!tmpelem.isNull())
                {
                    wpt->prx = tmpelem.text().toDouble();
                }
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

        if(wpt->lat == 1000 || wpt->lon == 1000 || wpt->name.isEmpty())
        {
            delete wpt;
            continue;
        }

        addWpt(wpt, true);
    }

    emit sigChanged();
}


void CWptDB::saveGPX(CGpx& gpx, const QStringList& keys)
{
    QString str;
    QDomElement root = gpx.documentElement();
    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        if((*wpt)->sticky)
        {
            ++wpt;
            continue;
        }

        if(!keys.isEmpty() && !keys.contains((*wpt)->key()))
        {
            ++wpt;
            continue;
        }

        QDomElement waypoint = gpx.createElement("wpt");
        root.appendChild(waypoint);
        str.sprintf("%1.8f", (*wpt)->lat);
        waypoint.setAttribute("lat",str);
        str.sprintf("%1.8f", (*wpt)->lon);
        waypoint.setAttribute("lon",str);

        if((*wpt)->ele != 1e25f)
        {
            QDomElement ele = gpx.createElement("ele");
            waypoint.appendChild(ele);
            QDomText _ele_ = gpx.createTextNode(QString::number((*wpt)->ele));
            ele.appendChild(_ele_);
        }

        QDateTime t = QDateTime::fromTime_t((*wpt)->timestamp);
        QDomElement time = gpx.createElement("time");
        waypoint.appendChild(time);
        QDomText _time_ = gpx.createTextNode(t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'"));
        time.appendChild(_time_);

        QDomElement name = gpx.createElement("name");
        waypoint.appendChild(name);
        QDomText _name_ = gpx.createTextNode((*wpt)->name);
        name.appendChild(_name_);

        if(!(*wpt)->comment.isEmpty())
        {
            QDomElement cmt = gpx.createElement("cmt");
            waypoint.appendChild(cmt);
            QDomText _cmt_ = gpx.createTextNode((*wpt)->comment);
            cmt.appendChild(_cmt_);
        }

        if(!(*wpt)->link.isEmpty())
        {
            QDomElement link = gpx.createElement("link");
            waypoint.appendChild(link);
            link.setAttribute("href",(*wpt)->link);
            QDomElement text = gpx.createElement("text");
            link.appendChild(text);
            QDomText _text_ = gpx.createTextNode((*wpt)->name);
            text.appendChild(_text_);
        }

        QDomElement sym = gpx.createElement("sym");
        waypoint.appendChild(sym);
        QDomText _sym_ = gpx.createTextNode((*wpt)->icon);
        sym.appendChild(_sym_);

        if((*wpt)->prx != 1e25f)
        {
            QDomElement extensions = gpx.createElement("extensions");
            waypoint.appendChild(extensions);
            QDomElement gpxx_ext = gpx.createElement("gpxx:WaypointExtension");
            extensions.appendChild(gpxx_ext);

            if((*wpt)->prx != 1e25f)
            {
                QDomElement proximity = gpx.createElement("gpxx:Proximity");
                gpxx_ext.appendChild(proximity);
                QDomText _proximity_ = gpx.createTextNode(QString::number((*wpt)->prx));
                proximity.appendChild(_proximity_);
            }
        }

        ++wpt;
    }
}


void CWptDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.waypoints(),QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    while(!stream.atEnd())
    {
        CWpt * wpt = new CWpt(this);
        stream >> *wpt;
        addWpt(wpt,true);
    }

    emit sigChanged();

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


void CWptDB::selWptByKey(const QString& key)
{
    CWptToolWidget * t = qobject_cast<CWptToolWidget*>(toolview);
    if(t)
    {
        t->selWptByKey(key);
    }
}


void CWptDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    IMap& map = CMapDB::self().getMap();

    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect.contains(QPoint(u,v)))
        {
            QPixmap icon = getWptIconByName((*wpt)->icon);
            QPixmap back = QPixmap(icon.size());
            back.fill(Qt::white);
            back.setMask(icon.alphaChannel().createMaskFromColor(Qt::black));
            // draw waypoint icon
            p.drawPixmap(u-8 , v-8, back);
            p.drawPixmap(u-8 , v-7, back);
            p.drawPixmap(u-8 , v-6, back);
            p.drawPixmap(u-7 , v-8, back);

            p.drawPixmap(u-7 , v-6, back);
            p.drawPixmap(u-6 , v-8, back);
            p.drawPixmap(u-6 , v-7, back);
            p.drawPixmap(u-6 , v-6, back);

            p.drawPixmap(u-7 , v-7, icon);

            if((*wpt)->prx != WPT_NOFLOAT)
            {
                XY pt1, pt2;

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

            CCanvas::drawText((*wpt)->name,p,QPoint(u,v - 10));

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
            ExifRational * p = (ExifRational*)exifEntry->data;
            if(exifEntry->components == 3)
            {
                //                 qDebug() << "lat" << exifEntry->components;
                //                 qDebug() <<  p[0].numerator <<  p[0].denominator << ((double)p[0].numerator / p[0].denominator);
                //                 qDebug() <<  p[1].numerator <<  p[1].denominator << ((double)p[1].numerator / (p[1].denominator * 60));
                //                 qDebug() <<  p[2].numerator <<  p[2].denominator << ((double)p[2].numerator / ((double)p[2].denominator * 3600.0));
                exifGPS.lat = (double)p[0].numerator/p[0].denominator + (double)p[1].numerator/(p[1].denominator * 60) + (double)p[2].numerator/((double)p[2].denominator * 3600.0);
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
            ExifRational * p = (ExifRational*)exifEntry->data;
            if(exifEntry->components == 3)
            {
                //                 qDebug() << "lon" << exifEntry->components;
                //                 qDebug() <<  p[0].numerator <<  p[0].denominator << ((double)p[0].numerator / p[0].denominator);
                //                 qDebug() <<  p[1].numerator <<  p[1].denominator << ((double)p[1].numerator / (p[1].denominator * 60));
                //                 qDebug() <<  p[2].numerator <<  p[2].denominator << ((double)p[2].numerator / ((double)p[2].denominator * 3600.0));
                exifGPS.lon = (double)p[0].numerator/p[0].denominator + (double)p[1].numerator/(p[1].denominator * 60) + (double)p[2].numerator/((double)p[2].denominator * 3600.0);
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

    QSettings cfg;
    QString path = cfg.value("path/images", "./").toString();
    path = QFileDialog::getExistingDirectory(0, tr("Select path..."), path, QFileDialog::DontUseNativeDialog);

    if(path.isEmpty()) return;

    cfg.setValue("path/images", path);

    QDir dir(path);
    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png";
    QStringList files = dir.entryList(filter, QDir::Files);
    QString file;

    foreach(file, files)
    {
        //         qDebug() << "---------------" << file << "---------------";

        ExifData * exifData = f_exif_data_new_from_file(dir.filePath(file).toLocal8Bit());

        exifGPS_t exifGPS;

        f_exif_data_foreach_content(exifData, exifDataForeachContentFunc, &exifGPS);

        CWpt * wpt      = new CWpt(this);
        wpt->lon        = exifGPS.lon * exifGPS.lon_sign;
        wpt->lat        = exifGPS.lat * exifGPS.lon_sign;
        wpt->timestamp  = exifGPS.timestamp;
        wpt->icon       = "Flag, Red";
        wpt->name       = file;

        qDebug() << wpt->name << wpt->lon << wpt->lat;

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
