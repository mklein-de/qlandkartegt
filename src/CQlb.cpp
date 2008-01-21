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

#include "CQlb.h"
#include "CWpt.h"

#include <QtCore>

CQlb::CQlb(QObject * parent)
    : QObject(parent)
{

}

CQlb::~CQlb()
{

}

CQlb& CQlb::operator <<(CWpt& wpt)
{
    QDataStream stream(&wpts, QIODevice::Append);
    stream << wpt;

    return *this;
}

void CQlb::load(const QString& filename)
{
    qint32 type;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    stream >> type;
    while(type != eEnd){

        switch(type){

            case eWpt:
                stream >> wpts;
                break;

            default:
                file.close();
                return;
        }

        stream >> type;
    }

    file.close();
}

void CQlb::save(const QString& filename)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    stream << (qint32)eWpt << wpts;
    stream << (qint32)eEnd;

    file.close();
}
