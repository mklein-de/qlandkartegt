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

#include "CWpt.h"
#include "CWptDB.h"
#include "WptIcons.h"
#include "IUnit.h"

#include <QtCore>
#include <QtXml>

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

QDir CWpt::path(_MKSTR(MAPPATH) "/wpt");

const QString CWpt::html =  QObject::tr(""
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>"
"<html>"
"   <head>"
"       <META HTTP-EQUIV='CACHE-CONTROL'' CONTENT='NO-CACHE'>"
"       <meta http-equiv='Content-Typ' content='text/html; charset=utf-8'>"
"       <style type='text/css'>"
"           p, li { white-space: pre-wrap; }"
"           td {padding-top: 10px;}"
"           th {background-color: lightBlue;}"
"       </style>"
"   </head>"
"   <body style=' font-family:'Sans'; font-size:9pt; font-weight:400; font-style:normal;'>"
"       <p>${info}</p>"
"   </body>"
"</html>"
"");


struct wpt_head_entry_t
{
    wpt_head_entry_t() : type(CWpt::eEnd), offset(0) {}
    qint32      type;
    quint32     offset;
    QByteArray  data;
};

QDataStream& operator >>(QDataStream& s, CWpt& wpt)
{
    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

    if(strncmp(magic,"QLWpt   ",9))
    {
        dev->seek(pos);
        //         throw(QObject::tr("This is not waypoint data."));
        return s;
    }

    QList<wpt_head_entry_t> entries;

    while(1)
    {
        wpt_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CWpt::eEnd) break;
    }

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type)
        {
            case CWpt::eBase:
            {
                QString icon;
                QString key;

                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                s1 >> key;
                s1 >> wpt.sticky;
                s1 >> wpt.timestamp;
                s1 >> icon;
                s1 >> wpt.name;
                s1 >> wpt.comment;
                s1 >> wpt.lat;
                s1 >> wpt.lon;
                s1 >> wpt.ele;
                s1 >> wpt.prx;
                s1 >> wpt.link;
                s1 >> wpt.description;

                wpt.setIcon(icon);
                wpt.setKey(key);
                break;
            }

            case CWpt::eImage:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                CWpt::image_t img;

                wpt.images.clear();

                s1 >> img.offset;
                while(img.offset)
                {
                    wpt.images << img;
                    s1 >> img.offset;
                }

                QList<CWpt::image_t>::iterator image = wpt.images.begin();
                while(image != wpt.images.end())
                {
                    s1.device()->seek(image->offset);
                    s1 >> image->filePath;
                    s1 >> image->info;
                    s1 >> image->pixmap;
                    ++image;
                }
                break;
            }

            default:;
        }

        ++entry;
    }
    return s;
}


QDataStream& operator <<(QDataStream& s, CWpt& wpt)
{
    QList<wpt_head_entry_t> entries;

    //---------------------------------------
    // prepare base data
    //---------------------------------------
    wpt_head_entry_t entryBase;
    entryBase.type = CWpt::eBase;
    QDataStream s1(&entryBase.data, QIODevice::WriteOnly);
    s1.setVersion(QDataStream::Qt_4_5);

    s1 << wpt.getKey();
    s1 << wpt.sticky;
    s1 << wpt.timestamp;
    s1 << wpt.iconString;
    s1 << wpt.name;
    s1 << wpt.comment;
    s1 << wpt.lat;
    s1 << wpt.lon;
    s1 << wpt.ele;
    s1 << wpt.prx;
    s1 << wpt.link;
    s1 << wpt.description;

    entries << entryBase;

    //---------------------------------------
    // prepare image data
    //---------------------------------------
    wpt_head_entry_t entryImage;
    entryImage.type = CWpt::eImage;
    QDataStream s2(&entryImage.data, QIODevice::WriteOnly);
    s2.setVersion(QDataStream::Qt_4_5);

    // write place holder for image offset
    QList<CWpt::image_t>::iterator image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        s2 << (quint32)0;
        ++image;
    }
    // offset terminator
    s2 << (quint32)0;

    // write image data and store the actual offset
    image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        image->offset = (quint32)s2.device()->pos();
        s2 << image->filePath;
        s2 << image->info;
        s2 << image->pixmap;
        ++image;
    }

    // finally write image offset table
    (quint32)s2.device()->seek(0);
    image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        s2 << image->offset;
        ++image;
    }

    entries << entryImage;

    //---------------------------------------
    // prepare terminator
    //---------------------------------------
    wpt_head_entry_t entryEnd;
    entryEnd.type = CWpt::eEnd;
    entries << entryEnd;

    //---------------------------------------
    //---------------------------------------
    // now start to actually write data;
    //---------------------------------------
    //---------------------------------------
    // write magic key
    s.writeRawData("QLWpt   ",9);

    // calculate offset table
    quint32 offset = entries.count() * 8 + 9;

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->data;
        ++entry;
    }

    return s;
}


void operator >>(QFile& f, CWpt& wpt)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s >> wpt;
    f.close();
}


void operator <<(QFile& f, CWpt& wpt)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s << wpt;
    f.close();
}


CWpt::CWpt(CWptDB * parent)
: IItem(parent)
, sticky(false)
//, timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
//, icon("")
, lat(1000)
, lon(1000)
, ele(WPT_NOFLOAT)
, prx(WPT_NOFLOAT)
{
    setIcon("Small City");
}


CWpt::~CWpt()
{
    qDebug() << "CWpt::~CWpt()";
}

void CWpt::setIcon(const QString& str)
{
    iconString = str;
    iconPixmap = getWptIconByName(str);
}

QString CWpt::getInfo()
{
    QString str = getName();
    if(timestamp != 0x00000000 && timestamp != 0xFFFFFFFF)
    {
        if(str.count()) str += "\n";
        QDateTime time = QDateTime::fromTime_t(timestamp);
        time.setTimeSpec(Qt::LocalTime);
        str += time.toString();
    }

    if(ele != WPT_NOFLOAT)
    {
        if(str.count()) str += "\n";
        QString val, unit;
        IUnit::self().meter2elevation(ele, val, unit);
        str += tr("elevation: %1 %2").arg(val).arg(unit);
    }

    if(prx != WPT_NOFLOAT)
    {
        if(str.count()) str += "\n";
        QString val, unit;
        IUnit::self().meter2distance(prx, val, unit);
        str += tr("proximity: %1 %2").arg(val).arg(unit);
    }

    if(link.count())
    {
        if(str.count()) str += "\n";
        if(link.count() < 50)
        {
            str += "http://" + link;
        }
        else
        {
            str += "http://" + link.left(47) + "...";
        }
    }

    if(description.count())
    {
        if(str.count()) str += "\n";

        if(description.count() < 200)
        {
            str += description;
        }
        else
        {
            str += description.left(197) + "...";
        }
    }

    if(comment.count())
    {
        if(str.count()) str += "\n";

        if(comment.count() < 200)
        {
            str += comment;
        }
        else
        {
            str += comment.left(197) + "...";
        }

    }
    return str;
}

const QString CWpt::filename(const QDir& dir)
{
    QDateTime ts;
    QString str;

    ts.setTime_t(timestamp);
    str  = ts.toString("yyyy.MM.dd_hh.mm.ss_");
    str += name;
    str += ".wpt";

    return dir.filePath(str);
}

QString CWpt::getEntry(const QString& tag, QDomNode& parent)
{
    if(parent.namedItem(tag).isElement())
    {
        return parent.namedItem(tag).toElement().text();
    }

    return "";
}

void CWpt::loadGpxExt(const QDomNode& wpt)
{
    QDomNode gpxCache = wpt.namedItem("cache");

    geocache = geocache_t();

    if(gpxCache.isNull())
    {
        return;
    }
    const QDomNamedNodeMap& attr = gpxCache.attributes();
    geocache.id         = attr.namedItem("id").nodeValue().toInt();
    geocache.archived   = attr.namedItem("archived").nodeValue().toLocal8Bit() == "True";
    geocache.available  = attr.namedItem("available").nodeValue().toLocal8Bit() == "True";

    geocache.name       = getEntry("name",gpxCache);
    geocache.owner      = getEntry("placed_by",gpxCache);
    geocache.type       = getEntry("type",gpxCache);
    geocache.container  = getEntry("container",gpxCache);
    geocache.difficulty = getEntry("difficulty",gpxCache).toInt();
    geocache.terrain    = getEntry("terrain",gpxCache).toInt();

    geocache.hasData = true;

    qDebug() << "xxxxxxxxxx";
}

void CWpt::setEntry(const QString& tag, const QString& val, QDomDocument& gpx, QDomElement& parent)
{
    if(!val.isEmpty())
    {
        QDomElement element = gpx.createElement(tag);
        parent.appendChild(element);
        QDomText text = gpx.createTextNode(val);
        element.appendChild(text);
    }
}

void CWpt::saveGpxExt(QDomNode& wpt)
{
    if(!geocache.hasData)
    {
        return;
    }
    QDomDocument gpx       = wpt.ownerDocument();
    QDomElement gpxCache   = gpx.createElement("groundspeak:cache");

//    gpxCache.setAttribute("xmlns:groundspeak", "http://www.groundspeak.com/cache/1/0");
    gpxCache.setAttribute("id", geocache.id);
    gpxCache.setAttribute("archived", geocache.archived ? "True" : "False");
    gpxCache.setAttribute("available", geocache.available ? "True" : "False");

    setEntry("groundspeak:name", geocache.name, gpx, gpxCache);
    setEntry("groundspeak:placed_by", geocache.owner, gpx, gpxCache);
    setEntry("groundspeak:type", geocache.owner, gpx, gpxCache);
    setEntry("groundspeak:container", geocache.owner, gpx, gpxCache);
    setEntry("groundspeak:difficulty", geocache.owner, gpx, gpxCache);
    setEntry("groundspeak:terrain", geocache.owner, gpx, gpxCache);

    wpt.appendChild(gpxCache);

}

QString CWpt::getExtInfo()
{
    QString info = tr("No additional information.");

    QString cpytext = html;
    cpytext = cpytext.replace("${info}", info);

    return cpytext;
}
