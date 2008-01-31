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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CWpt.h"

#include <QtCore>

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

QDir CWpt::path(_MKSTR(MAPPATH) "/wpt");

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

    if(strncmp(magic,"QLWpt   ",9)){
        dev->seek(pos);
//         throw(QObject::tr("This is not waypoint data."));
        return s;
    }

    QList<wpt_head_entry_t> entries;

    while(1){
        wpt_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CWpt::eEnd) break;
    }

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()){
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type){
            case CWpt::eBase:
            {

                QDataStream s1(&entry->data, QIODevice::ReadOnly);

                s1 >> wpt._key_;
                s1 >> wpt.sticky;
                s1 >> wpt.timestamp;
                s1 >> wpt.icon;
                s1 >> wpt.name;
                s1 >> wpt.comment;
                s1 >> wpt.lat;
                s1 >> wpt.lon;
                s1 >> wpt.ele;
                s1 >> wpt.prx;
                s1 >> wpt.link;

                break;
            }

            case CWpt::eImage:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                CWpt::image_t img;

                wpt.images.clear();

                s1 >> img.offset;
                while(img.offset){
                    wpt.images << img;
                    s1 >> img.offset;
                }

                QList<CWpt::image_t>::iterator image = wpt.images.begin();
                while(image != wpt.images.end()){
                    s1.device()->seek(image->offset);
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

    s1 << wpt._key_;
    s1 << wpt.sticky;
    s1 << wpt.timestamp;
    s1 << wpt.icon;
    s1 << wpt.name;
    s1 << wpt.comment;
    s1 << wpt.lat;
    s1 << wpt.lon;
    s1 << wpt.ele;
    s1 << wpt.prx;
    s1 << wpt.link;

    entries << entryBase;

    //---------------------------------------
    // prepare image data
    //---------------------------------------
    wpt_head_entry_t entryImage;
    entryImage.type = CWpt::eImage;
    QDataStream s2(&entryImage.data, QIODevice::WriteOnly);

    // write place holder for image offset
    QList<CWpt::image_t>::iterator image = wpt.images.begin();
    while(image != wpt.images.end()){
        s2 << (quint32)0;
        ++image;
    }
    // offset terminator
    s2 << (quint32)0;

    // write image data and store the actual offset
    image = wpt.images.begin();
    while(image != wpt.images.end()){
        image->offset = (quint32)s2.device()->pos();
        s2 << image->info;
        s2 << image->pixmap;
        ++image;
    }

    // finally write image offset table
    (quint32)s2.device()->seek(0);
    image = wpt.images.begin();
    while(image != wpt.images.end()){
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
    while(entry != entries.end()){
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end()){
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end()){
        s << entry->data;
        ++entry;
    }

    return s;
}

void operator >>(QFile& f, CWpt& wpt)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s >> wpt;
    f.close();
}

void operator <<(QFile& f, CWpt& wpt)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s << wpt;
    f.close();
}



CWpt::CWpt(QObject * parent)
    : QObject(parent)
    , sticky(false)
    , timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
    , icon("Star")
    , lat(1000)
    , lon(1000)
    , ele(WPT_NOFLOAT)
    , prx(WPT_NOFLOAT)

{

}

CWpt::~CWpt()
{
    qDebug() << "CWpt::~CWpt()";
}

void CWpt::genKey()
{
    _key_ = QString("%1%2").arg(timestamp).arg(name);
}

const QString& CWpt::key()
{
    if(_key_.isEmpty()) genKey();
    return _key_;
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
