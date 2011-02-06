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

            case CWpt::eGeoCache:
            {
                quint32 N, n;
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                wpt.geocache = CWpt::geocache_t();
                CWpt::geocache_t& cache = wpt.geocache;

                s1 >> (quint8&)cache.service;
                s1 >> cache.hasData;
                s1 >> cache.id;
                s1 >> cache.available;
                s1 >> cache.archived;
                s1 >> cache.difficulty;
                s1 >> cache.terrain;
                s1 >> cache.status;
                s1 >> cache.name;
                s1 >> cache.owner;
                s1 >> cache.ownerId;
                s1 >> cache.type;
                s1 >> cache.container;
                s1 >> cache.shortDesc;
                s1 >> cache.longDesc;
                s1 >> cache.hint;
                s1 >> cache.country;
                s1 >> cache.state;
                s1 >> cache.locale;

                s1 >> N;

                for(n = 0; n < N; n++)
                {
                    CWpt::geocachelog_t log;

                    s1 >> log.id;
                    s1 >> log.date;
                    s1 >> log.type;
                    s1 >> log.finderId;
                    s1 >> log.finder;
                    s1 >> log.text;

                    cache.logs << log;
                }

                cache.hasData = true;

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
    // prepare geocache data
    //---------------------------------------
    if(wpt.geocache.hasData)
    {

        wpt_head_entry_t entryGeoCache;
        entryGeoCache.type = CWpt::eGeoCache;
        QDataStream s3(&entryGeoCache.data, QIODevice::WriteOnly);
        s3.setVersion(QDataStream::Qt_4_5);

        CWpt::geocache_t& cache = wpt.geocache;

        s3 << (quint8)cache.service;
        s3 << cache.hasData;
        s3 << cache.id;
        s3 << cache.available;
        s3 << cache.archived;
        s3 << cache.difficulty;
        s3 << cache.terrain;
        s3 << cache.status;
        s3 << cache.name;
        s3 << cache.owner;
        s3 << cache.ownerId;
        s3 << cache.type;
        s3 << cache.container;
        s3 << cache.shortDesc;
        s3 << cache.longDesc;
        s3 << cache.hint;
        s3 << cache.country;
        s3 << cache.state;
        s3 << cache.locale;

        s3 << cache.logs.count();

        foreach(const CWpt::geocachelog_t& log, cache.logs)
        {
            s3 << log.id;
            s3 << log.date;
            s3 << log.type;
            s3 << log.finderId;
            s3 << log.finder;
            s3 << log.text;
        }

        entries << entryGeoCache;
    }
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

QString CWpt::getEntry(const QString& tag, const QDomNode& parent)
{
    if(parent.namedItem(tag).isElement())
    {
        return parent.namedItem(tag).toElement().text();
    }

    return "";
}

QString CWpt::getEntryHtml(const QString& tag, const QDomNode& parent)
{
    const QDomNode node = parent.namedItem(tag);
    if(node.isElement())
    {
        const QDomElement& elem      = node.toElement();
        const QDomNamedNodeMap& attr = elem.attributes();

        if(attr.namedItem("html").nodeValue().toLocal8Bit().toLower() == "true")
        {
            return elem.text();
        }

        return "<p>" + elem.text() + "</p>";
    }

    return "";
}

void CWpt::loadGpxExt(const QDomNode& wpt)
{
    geocache = geocache_t();

    QDomNode gpxCache = wpt.namedItem("cache");
    if(gpxCache.isNull())
    {
        const QDomNode& extension = wpt.namedItem("extensions");

        gpxCache = extension.namedItem("cache");
        if(gpxCache.isNull())
        {
            return;
        }

        loadOcExt(gpxCache);
    }
    else
    {
        loadGcExt(gpxCache);
    }
}

void CWpt::loadGcExt(const QDomNode& gpxCache)
{
    geocache.service = eGC;
    const QDomNamedNodeMap& attr = gpxCache.attributes();
    geocache.id = attr.namedItem("id").nodeValue().toInt();

    geocache.archived   = attr.namedItem("archived").nodeValue().toLocal8Bit() == "True";
    geocache.available  = attr.namedItem("available").nodeValue().toLocal8Bit() == "True";
    if(geocache.archived)
    {
        geocache.status = "Archived";
    }
    else if(geocache.available)
    {
        geocache.status = "Available";
    }
    else
    {
        geocache.status = "Not Available";
    }

    geocache.name       = getEntry("name",gpxCache);
    geocache.owner      = getEntry("placed_by",gpxCache);
    geocache.type       = getEntry("type",gpxCache);
    geocache.container  = getEntry("container",gpxCache);
    geocache.difficulty = getEntry("difficulty",gpxCache).toFloat();
    geocache.terrain    = getEntry("terrain",gpxCache).toFloat();
    geocache.shortDesc  = getEntryHtml("short_description",gpxCache);
    geocache.longDesc   = getEntryHtml("long_description",gpxCache);
    geocache.hint       = getEntry("encoded_hints",gpxCache);
    geocache.country    = getEntry("country",gpxCache);
    geocache.state      = getEntry("state",gpxCache);

    const QDomNodeList& logs = gpxCache.toElement().elementsByTagName("log");
    uint N = logs.count();

    for(uint n = 0; n < N; ++n)
    {
        const QDomNode& log = logs.item(n);
        const QDomNamedNodeMap& attr = log.attributes();

        geocachelog_t geocachelog;
        geocachelog.id      = attr.namedItem("id").nodeValue().toUInt();
        geocachelog.date    = getEntry("date", log);
        geocachelog.type    = getEntry("type", log);
        if(log.namedItem("finder").isElement())
        {
            const QDomNamedNodeMap& attr = log.namedItem("finder").attributes();
            geocachelog.finderId = attr.namedItem("id").nodeValue();
        }

        geocachelog.finder  = getEntry("finder", log);
        geocachelog.text    = getEntryHtml("text", log);

        geocache.logs << geocachelog;

    }
    geocache.hasData = true;
}

void CWpt::loadOcExt(const QDomNode& gpxCache)
{
    geocache.service = eOC;
    const QDomNamedNodeMap& attr = gpxCache.attributes();
    geocache.id = attr.namedItem("id").nodeValue().toInt();
    geocache.status = attr.namedItem("status").nodeValue();
    if(geocache.status == "Available")
    {
        geocache.available = true;
        geocache.archived  = false;
    }
    else
    {
        geocache.available = false;
        geocache.archived  = true;
    }


    geocache.name       = getEntry("name",gpxCache);
    geocache.owner      = getEntry("owner",gpxCache);
    {
        const QDomNamedNodeMap& attr = gpxCache.namedItem("owner").attributes();
        geocache.ownerId = attr.namedItem("userid").nodeValue();
    }

    geocache.type       = getEntry("type",gpxCache);
    geocache.container  = getEntry("container",gpxCache);
    geocache.difficulty = getEntry("difficulty",gpxCache).toFloat();
    geocache.terrain    = getEntry("terrain",gpxCache).toFloat();
    geocache.shortDesc  = getEntryHtml("short_description",gpxCache);
    geocache.longDesc   = getEntryHtml("long_description",gpxCache);
    geocache.hint       = getEntry("encoded_hints",gpxCache);
    geocache.country    = getEntry("country",gpxCache);
    geocache.state      = getEntry("state",gpxCache);
    geocache.locale     = getEntry("locale",gpxCache);

    const QDomNodeList& logs = gpxCache.toElement().elementsByTagName("log");
    uint N = logs.count();

    for(uint n = 0; n < N; ++n)
    {
        const QDomNode& log = logs.item(n);
        const QDomNamedNodeMap& attr = log.attributes();

        geocachelog_t geocachelog;
        geocachelog.id      = attr.namedItem("id").nodeValue().toUInt();
        geocachelog.date    = getEntry("date", log);
        geocachelog.type    = getEntry("type", log);
        if(log.namedItem("finder").isElement())
        {
            const QDomNamedNodeMap& attr = log.namedItem("finder").attributes();
            geocachelog.finderId = attr.namedItem("id").nodeValue();
        }

        geocachelog.finder  = getEntry("finder", log);
        geocachelog.text    = getEntryHtml("text", log);

        geocache.logs << geocachelog;

    }
    geocache.hasData = true;
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

void CWpt::setEntryHtml(const QString& tag, const QString& val, QDomDocument& gpx, QDomElement& parent)
{
    if(!val.isEmpty())
    {
        QDomElement element = gpx.createElement(tag);
        parent.appendChild(element);
        QDomText text = gpx.createCDATASection(val);
        element.appendChild(text);
        element.setAttribute("html","True");
    }
}

void CWpt::saveGpxExt(QDomNode& wpt)
{
    if(!geocache.hasData)
    {
        return;
    }

//    QString str;
//    QDomDocument gpx       = wpt.ownerDocument();
//    QDomElement gpxCache   = gpx.createElement("groundspeak:cache");

//    gpxCache.setAttribute("xmlns:groundspeak", "http://www.groundspeak.com/cache/1/0");
//    gpxCache.setAttribute("id", geocache.id);
//    gpxCache.setAttribute("archived", geocache.archived ? "True" : "False");
//    gpxCache.setAttribute("available", geocache.available ? "True" : "False");

//    setEntry("groundspeak:name", geocache.name, gpx, gpxCache);
//    setEntry("groundspeak:placed_by", geocache.owner, gpx, gpxCache);
//    setEntry("groundspeak:type", geocache.type, gpx, gpxCache);
//    setEntry("groundspeak:container", geocache.container, gpx, gpxCache);
//    str.sprintf("%1.1f", geocache.difficulty);
//    setEntry("groundspeak:difficulty", str, gpx, gpxCache);
//    str.sprintf("%1.1f", geocache.terrain);
//    setEntry("groundspeak:terrain", str, gpx, gpxCache);
//    setEntryHtml("groundspeak:short_description", geocache.shortDesc, gpx, gpxCache);
//    setEntryHtml("groundspeak:long_description", geocache.longDesc, gpx, gpxCache);
//    setEntry("groundspeak:encoded_hints", geocache.hint, gpx, gpxCache);

//    if(!geocache.logs.isEmpty())
//    {
//        QDomElement gpxLogs = gpx.createElement("groundspeak:logs");
//        gpxCache.appendChild(gpxLogs);

//        foreach(const geocachelog_t& log, geocache.logs)
//        {
//            QDomElement gpxLog = gpx.createElement("groundspeak:log");
//            gpxLogs.appendChild(gpxLog);

//            gpxLog.setAttribute("id", log.id);
//            setEntry("date", log.date, gpx, gpxLog);
//            setEntry("type", log.type, gpx, gpxLog);

//            QDomElement finder = gpx.createElement("groundspeak:finder");
//            gpxLog.appendChild(finder);

//            QDomText _finder_ = gpx.createCDATASection(log.finder);
//            finder.appendChild(_finder_);
//            finder.setAttribute("id", log.finderId);

//            setEntryHtml("text", log.text, gpx, gpxLog);
//        }
//    }

//    wpt.appendChild(gpxCache);

}

void CWpt::saveGcExt(QDomNode& gpxCache)
{

}

void CWpt::saveOcExt(QDomNode& gpxCache)
{

}


QString CWpt::getExtInfo()
{
    QString info = tr("No additional information.");

    if(geocache.hasData)
    {
        info  = "<h1>" + geocache.name + " by " + geocache.owner + "</h1>";
        info += "<p>";
        info += tr("<b>Type:</b> %1 <b>Container:</b> %2 ").arg(geocache.type).arg(geocache.container);
        info += tr("<b>Difficulty:</b> %1 <b>Terrain:</b> %2 ").arg(geocache.difficulty,0,'f',1).arg(geocache.terrain,0,'f',1);
        info += "</p>";
        info += geocache.shortDesc;
        info += geocache.longDesc;

        foreach(const geocachelog_t& log, geocache.logs)
        {
            info += "<p>";
            info += "<p>" + log.date + " " + log.finder + " " + log.type + "<p>";
            info += log.text;
            info += "</p>";
        }

    }

    QString cpytext = html;
    cpytext = cpytext.replace("${info}", info);

    return cpytext;
}
