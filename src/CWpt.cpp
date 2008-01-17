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

QDataStream& operator >>(QDataStream& s, CWpt& wpt)
{
    qint32 id;

    s >> id;
    while(id != CWpt::eEnd){
        if(id == CWpt::eBase){
            s >> wpt._key_;
            s >> wpt.timestamp;
            s >> wpt.icon;
            s >> wpt.name;
            s >> wpt.comment;
            s >> wpt.lat;
            s >> wpt.lon;
            s >> wpt.altitude;
            s >> wpt.proximity;
        }

        s >> id;
    }

    return s;
}

QDataStream& operator <<(QDataStream& s, CWpt& wpt)
{

    s << CWpt::eBase;
    s << wpt._key_;
    s << wpt.timestamp;
    s << wpt.icon;
    s << wpt.name;
    s << wpt.comment;
    s << wpt.lat;
    s << wpt.lon;
    s << wpt.altitude;
    s << wpt.proximity;

    s << CWpt::eEnd;

    return s;
}

CWpt::CWpt(const QString& name, QObject * parent)
    : QObject(parent)
    , timestamp(QDateTime::currentDateTime().toTime_t ())
    , name(name)

{
    _key_ = QString("%1%2").arg(timestamp).arg(name);
}

CWpt::CWpt(QObject * parent)
    : QObject(parent)
{
}

CWpt::~CWpt()
{

}

QString CWpt::filename()
{
    QDateTime ts;
    QString str;

    ts.setTime_t(timestamp);
    str  = ts.toString("yyyy.MM.dd_hh.mm.ss_");
    str += name;
    str += ".wpt";

    return str;
}
