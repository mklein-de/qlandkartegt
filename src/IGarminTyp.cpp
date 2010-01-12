/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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

#include "IGarminTyp.h"

#define DBG

IGarminTyp::IGarminTyp(QObject * parent)
: QObject(parent)
{

}

IGarminTyp::~IGarminTyp()
{

}

bool IGarminTyp::parseHeader(QDataStream& in)
{
    int i;
    QString garmintyp;
    quint8 byte;

    for(i = 0; i < 10; ++i) {
        in >> byte;
        garmintyp.append(byte);
    }
    garmintyp.append(0);
    if(garmintyp != "GARMIN TYP") {
        qDebug() << "CMapTDB::readTYP() not a known typ file";
        return false;
    }

    /* reading typ creation date string */
    quint16 startDate, endDate, year;
    quint8 month, day, hour, minutes, seconds;

    in.device()->seek(0x0c);
    in >> startDate >> year >> month >> day >> hour >> minutes >> seconds >> endDate;
    month -= 1;                  /* Month are like Microsoft starting 0 ? */
    year += 1900;

    /* Reading points / lines / polygons struct */
    in >> sectPoints.dataOffset >> sectPoints.dataLength;
    in >> sectPolylines.dataOffset >> sectPolylines.dataLength;
    in >> sectPolygons.dataOffset >> sectPolygons.dataLength;

    in >> pid >> fid;


    /* Read Array datas */
    in >> sectPoints.arrayOffset >> sectPoints.arrayModulo >> sectPoints.arraySize;
    in >> sectPolylines.arrayOffset  >> sectPolylines.arrayModulo  >> sectPolylines.arraySize;
    in >> sectPolygons.arrayOffset >> sectPolygons.arrayModulo >> sectPolygons.arraySize;
    in >> sectOrder.arrayOffset >> sectOrder.arrayModulo >> sectOrder.arraySize;

#ifdef DBG
    qDebug() << "PID" << hex << pid << "FID" << hex << fid;
    qDebug() << "Points     doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPoints.dataOffset << "\t" << sectPoints.dataLength << "\t" << sectPoints.arrayOffset << "\t" << sectPoints.arrayModulo << "\t" << sectPoints.arrayOffset;
    qDebug() << "Polylines  doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPolylines.dataOffset << "\t" << sectPolylines.dataLength << "\t" << sectPolylines.arrayOffset << "\t" << sectPolylines.arrayModulo << "\t" << sectPolylines.arrayOffset;
    qDebug() << "Polygons   doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPolygons.dataOffset << "\t" << sectPolygons.dataLength << "\t" << sectPolygons.arrayOffset << "\t" << sectPolygons.arrayModulo << "\t" << sectPolygons.arrayOffset;
    qDebug() << "Order      doff/dlen/aoff/amod/asize:" << hex << "\t" << sectOrder.dataOffset << "\t" << sectOrder.dataLength << "\t" << sectOrder.arrayOffset << "\t" << sectOrder.arrayModulo << "\t" << sectOrder.arrayOffset;
#endif

    return true;
}

bool IGarminTyp::parseDrawOrder(QDataStream& in, QList<quint32> drawOrder)
{
    if(sectOrder.arrayModulo != 5) {
        return false;
    }

    if(!sectOrder.arrayModulo || ((sectOrder.arraySize % sectOrder.arrayModulo) != 0)) {
        return false;
    }

    in.device()->seek(sectOrder.arrayOffset);

    quint16 typ, subtyp;
    quint8 a2;
    int count=1;

    for (unsigned  i = 0; i < (sectOrder.arraySize / 5); i++) {
        in >> typ >> a2 >> subtyp;
        if (typ == 0) {
            count++;
        }
        else if(typ < 0x80) {
#ifdef DBG
            qDebug() << QString("Type 0x%1 is priority %2").arg(typ,0,16).arg(count);
#endif
            int idx = drawOrder.indexOf(typ);
            if(idx != -1) {
                drawOrder.move(idx,0);
            }
        }
        else{
            quint32 exttyp = (typ << 8)|subtyp;
            qDebug() << QString("Type 0x%1 is priority %2").arg(exttyp,0,16).arg(count);
            drawOrder.push_front(exttyp);

        }
    }

#ifdef DBG
for(unsigned i = 0; i < drawOrder.size(); ++i){
        if(i && i%16 == 0) printf(" \n");
        printf("%02X ", drawOrder[i]);
    }
    printf(" \n");
#endif

   return true;
}
