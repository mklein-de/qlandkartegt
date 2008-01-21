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
        throw(QObject::tr("This is not waypoint data."));
    }

    QList<wpt_head_entry_t> entries;

    while(1){
        wpt_head_entry_t entry;
        s >> entry.type >> entry.offset;
        if(entry.type == CWpt::eEnd) break;
        qDebug() << hex << entry.type << entry.offset;

        entries << entry;
    }

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()){

        switch(entry->type){
            case CWpt::eBase:
            {
                qint64 o = pos + entry->offset;
                dev->seek(o);
                s >> entry->data;

                QDataStream s1(&entry->data, QIODevice::ReadOnly);

                s1 >> wpt._key_;
                s1 >> wpt.sticky;
                s1 >> wpt.timestamp;
                s1 >> wpt.icon;
                s1 >> wpt.name;
                s1 >> wpt.comment;
                s1 >> wpt.lat;
                s1 >> wpt.lon;
                s1 >> wpt.altitude;
                s1 >> wpt.proximity;

                break;
            }

            case CWpt::eImage:
            {

                break;
            }

            default:;
        }

        ++entry;
    }
    return s;
}

/*
    32bit type
    32bit offset
    ....
    eEnd
    0x00000000

*/
QDataStream& operator <<(QDataStream& s, CWpt& wpt)
{
    QList<wpt_head_entry_t> entries;

    // prepare base data
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
    s1 << wpt.altitude;
    s1 << wpt.proximity;

    entries << entryBase;

    // prepare image data

    // add terminator
    wpt_head_entry_t entryEnd;
    entryEnd.type = CWpt::eEnd;
    entries << entryEnd;

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


CWpt::CWpt(QObject * parent)
    : QObject(parent)
    , timestamp(QDateTime::currentDateTime().toTime_t ())
    , icon("Star")
{

}

CWpt::~CWpt()
{

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

const QString CWpt::filename()
{
    QDateTime ts;
    QString str;

    ts.setTime_t(timestamp);
    str  = ts.toString("yyyy.MM.dd_hh.mm.ss_");
    str += name;
    str += ".wpt";

    return str;
}
