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
#include <QtCore>

#include <stdio.h>
#include <string.h>

#define DBG

IGarminTyp::IGarminTyp(format_e format, QObject * parent)
: QObject(parent)
, format(format)
{

}

IGarminTyp::~IGarminTyp()
{

}

void IGarminTyp::decodeBitmap(QDataStream &in, QImage &img, int w, int h, int bpp)
{
    int x = 0,j = 0;
    quint8 color;
    for (int y = 0; y < h; y++) {
        while ( x < w ) {
            in >> color;

            for ( int i = 0; (i < (8 / bpp)) && (x < w) ; i++ ) {
                int value;
                if ( i > 0 ) {
                    value = (color >>= bpp);
                }
                else {
                    value = color;
                }
                if ( bpp == 4) value = value & 0xf;
                if ( bpp == 2) value = value & 0x3;
                if ( bpp == 1) value = value & 0x1;
                img.setPixel(x,y,value);
                //                 qDebug() << QString("value(%4) pixel at (%1,%2) is 0x%3 j is %5").arg(x).arg(y).arg(value,0,16).arg(color).arg(j);
                x += 1;
            }
            j += 1;
        }
        x = 0;
    }
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

    in.device()->seek(0x0c);
    in >> version >> year >> month >> day >> hour >> minutes >> seconds >> codepage;
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
    qDebug() << "Version:" << version << "Codepage:" << codepage;
    qDebug() << "PID" << hex << pid << "FID" << hex << fid;
    qDebug() << "Points     doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPoints.dataOffset << "\t" << sectPoints.dataLength << "\t" << sectPoints.arrayOffset << "\t" << sectPoints.arrayModulo << "\t" << sectPoints.arrayOffset;
    qDebug() << "Polylines  doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPolylines.dataOffset << "\t" << sectPolylines.dataLength << "\t" << sectPolylines.arrayOffset << "\t" << sectPolylines.arrayModulo << "\t" << sectPolylines.arrayOffset;
    qDebug() << "Polygons   doff/dlen/aoff/amod/asize:" << hex << "\t" << sectPolygons.dataOffset << "\t" << sectPolygons.dataLength << "\t" << sectPolygons.arrayOffset << "\t" << sectPolygons.arrayModulo << "\t" << sectPolygons.arrayOffset;
    qDebug() << "Order      doff/dlen/aoff/amod/asize:" << hex << "\t" << sectOrder.dataOffset << "\t" << sectOrder.dataLength << "\t" << sectOrder.arrayOffset << "\t" << sectOrder.arrayModulo << "\t" << sectOrder.arrayOffset;
#endif

    return true;
}

bool IGarminTyp::parseDrawOrder(QDataStream& in, QList<quint32>& drawOrder)
{
    if(sectOrder.arrayModulo != 5) {
        return false;
    }

    if((sectOrder.arraySize % sectOrder.arrayModulo) != 0) {
        return true;
    }

    in.device()->seek(sectOrder.arrayOffset);

    int i,n;
    quint8  typ;
    quint32 subtyp;

    int count=1;

    const int N = sectOrder.arraySize / sectOrder.arrayModulo;

    for (unsigned  i = 0; i < N; i++) {
        in >> typ >>  subtyp;
//         qDebug() << hex << typ << subtyp;
        if (typ == 0) {
            count++;
        }
        else if(subtyp == 0) {
#ifdef DBG
            qDebug() << QString("Type 0x%1 is priority %2").arg(typ,0,16).arg(count);
#endif
            int idx = drawOrder.indexOf(typ);
            if(idx != -1) {
                drawOrder.move(idx,0);
            }
        }
        else if(format == eNT){
            quint32 exttyp = 0x010000 | (typ << 8);
            quint32 mask = 0x1;

            for(n=0; n < 0x20; ++n){
                if(subtyp & mask){
                    drawOrder.push_front(exttyp|n);
#ifdef DBG
                    qDebug() << QString("Type 0x%1 is priority %2").arg(exttyp|n,0,16).arg(count);
#endif
                }
                mask = mask << 1;
            }

        }
        else if(format == eNorm){
            quint32 exttyp = 0x010000 | (typ << 8) | (subtyp >> 16);
            drawOrder.push_front(exttyp);
#ifdef DBG
            qDebug() << QString("Type 0x%1 is priority %2").arg(exttyp,0,16).arg(count);
#endif

        }
    }

#ifdef DBG
    for(unsigned i = 0; i < drawOrder.size(); ++i){
        if(i && i%16 == 0) printf(" \n");
        printf("%06X ", drawOrder[i]);
    }
    printf(" \n");
#endif

   return true;
}


bool IGarminTyp::parsePolygon(QDataStream& in, QMap<quint32, polygon_property>& polygons)
{
    bool tainted = false;

    if(!sectPolygons.arrayModulo || ((sectPolygons.arraySize % sectPolygons.arrayModulo) != 0)) {
        return true;
    }

    QTextCodec * codec = QTextCodec::codecForName(QString("CP%1").arg(codepage).toLatin1());

    const int N = sectPolygons.arraySize / sectPolygons.arrayModulo;
    for (int element = 0; element < N; element++) {
        quint16 t16_1, t16_2, subtyp;
        quint8  t8;
        quint32 typ, offset;
        bool hasLocalization = false;
        bool hasTextColor = false;
        quint8 ctyp;
        QImage xpmDay(32,32, QImage::Format_Indexed8);
        QImage xpmNight(32,32, QImage::Format_Indexed8);
        quint8 r,g,b;
        quint8 langcode;

        in.device()->seek( sectPolygons.arrayOffset + (sectPolygons.arrayModulo * element ) );

        if (sectPolygons.arrayModulo == 5) {
            in >> t16_1 >> t16_2 >> t8;
            offset = t16_2|(t8<<16);
        }
        else if (sectPolygons.arrayModulo == 4) {
            in >> t16_1 >> t16_2;
            offset = t16_2;
        }
        else if (sectPolygons.arrayModulo == 3) {
            in >> t16_1 >> t8;
            offset = t8;
        }

        t16_2   = (t16_1 >> 5) | (( t16_1 & 0x1f) << 11);
        typ     = t16_2 & 0x7F;
        subtyp  = t16_1 & 0x1F;

        if(t16_1 & 0x2000) {
            typ = 0x10000|(typ << 8)|subtyp;
        }

        in.device()->seek(sectPolygons.dataOffset + offset);
        in >> t8;
        hasLocalization = t8 & 0x10;
        hasTextColor    = t8 & 0x20;
        ctyp            = t8 & 0x0F;

#ifdef DBG
        qDebug() << "Polygon typ:" << hex << typ << "ctype:" << ctyp << "offset:" << (sectPolygons.dataOffset + offset) << "orig data:" << t16_1;
#endif

        polygon_property& property = polygons[typ];

        switch(ctyp){
            case 0x06:
            {
                // day & night single color
                in >> b >> g >> r;
                property.brushDay      = QBrush(qRgb(r,g,b));
                property.brushNight    = QBrush(qRgb(r,g,b));
                property.pen           = Qt::NoPen;
                property.known         = true;

                break;
            }
            case 0x07:
            {
                // day single color & night single color
                in >> b >> g >> r;
                property.brushDay      = QBrush(qRgb(r,g,b));
                in >> b >> g >> r;
                property.brushNight    = QBrush(qRgb(r,g,b));
                property.pen           = Qt::NoPen;
                property.known         = true;

                break;
            }
            case 0x08:
            {
                // day & night two color
                xpmDay.setNumColors(2);

                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmDay.setColor(0, qRgb(r,g,b) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmDay);
                property.pen      = Qt::NoPen;
                property.known    = true;
                break;
            }

            case 0x09:
            {
                //day two color & night two color
                xpmDay.setNumColors(2);
                xpmNight.setNumColors(2);

                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmDay.setColor(0, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmNight.setColor(1, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmNight.setColor(0, qRgb(r,g,b) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                memcpy(xpmNight.bits(), xpmDay.bits(), (32*32));
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmNight);
                property.pen      = Qt::NoPen;
                property.known    = true;

                break;
            }
            case 0x0B:
            {
                // day one color, transparent & night two color
                xpmDay.setNumColors(2);
                xpmNight.setNumColors(2);
                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                xpmDay.setColor(0, qRgba(255,255,255,0) );

                in >> b >> g >> r;
                xpmNight.setColor(1, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmNight.setColor(0, qRgb(r,g,b) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                memcpy(xpmNight.bits(), xpmDay.bits(), (32*32));
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmNight);
                property.pen      = Qt::NoPen;
                property.known    = true;
                break;
            }

            case 0x0D:
            {
                // day two color & night one color, transparent
                xpmDay.setNumColors(2);
                xpmNight.setNumColors(2);
                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                in >> b >> g >> r;
                xpmDay.setColor(0, qRgb(r,g,b) );

                in >> b >> g >> r;
                xpmNight.setColor(1, qRgb(r,g,b) );
                xpmNight.setColor(0, qRgba(255,255,255,0) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                memcpy(xpmNight.bits(), xpmDay.bits(), (32*32));
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmNight);
                property.pen      = Qt::NoPen;
                property.known    = true;

                break;
            }
            case 0x0E:
            {
                // day & night one color, transparent
                xpmDay.setNumColors(2);
                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                xpmDay.setColor(0, qRgba(255,255,255,0) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmDay);
                property.pen      = Qt::NoPen;
                property.known    = true;

                break;
            }
            case 0x0F:
            {
                // day one color, transparent & night one color, transparent
                xpmDay.setNumColors(2);
                xpmNight.setNumColors(2);
                in >> b >> g >> r;
                xpmDay.setColor(1, qRgb(r,g,b) );
                xpmDay.setColor(0, qRgba(255,255,255,0) );

                in >> b >> g >> r;
                xpmNight.setColor(1, qRgb(r,g,b) );
                xpmNight.setColor(0, qRgba(255,255,255,0) );

                decodeBitmap(in, xpmDay, 32, 32, 1);
                memcpy(xpmNight.bits(), xpmDay.bits(), (32*32));
                property.brushDay.setTextureImage(xpmDay);
                property.brushNight.setTextureImage(xpmNight);
                property.pen      = Qt::NoPen;
                property.known    = true;

                break;
            }

            default:
                if(!tainted) {
                    QMessageBox::warning(0, tr("Warning..."), tr("This is a typ file with unknown polygon encoding. Please report!"), QMessageBox::Abort, QMessageBox::Abort);
                    tainted = true;
                }
                qDebug() << "Failed polygon:" << typ << subtyp << hex << typ << subtyp << ctyp;
        }

        if(hasLocalization){
            qint16 len;
            quint8 n = 1;

            in >> t8;
            len = t8;


            if(!(t8 & 0x01)){
                n = 2;
                in >> t8;
                len |= t8 << 8;
            }

            len -= n;
            while(len > 0){
                QByteArray str;
                in >> langcode;
                len -= 2*n;
                while(len > 0){

                    in >> t8;
                    len -= 2*n;

                    if(t8 == 0) break;

                    str += t8;

                }
                property.strings[langcode] = codec->toUnicode(str);
#ifdef DBG
                qDebug() << len << langcode << property.strings[langcode];
#endif
            }
        }

        if(hasTextColor){
            in >> t8;
            property.labelType = (label_type_e)(t8 & 0x07);

            if(t8 & 0x08){
                in >> r >> g >> b;
                property.colorLabelDay = qRgb(r,g,b);
            }

            if(t8 & 0x10){
                in >> r >> g >> b;
                property.colorLabelNight = qRgb(r,g,b);
            }
#ifdef DBG
            qDebug() << "ext. label: type" << property.labelType << "day" << property.colorLabelDay << "night" << property.colorLabelNight;
#endif
        }
    }

    return true;
}

bool IGarminTyp::parsePolyline(QDataStream& in, QMap<quint32, polyline_property>& polylines)
{
    bool tainted = false;

    if(!sectPolylines.arrayModulo || ((sectPolylines.arraySize % sectPolylines.arrayModulo) != 0)) {
        return true;
    }

    QTextCodec * codec = QTextCodec::codecForName(QString("CP%1").arg(codepage).toLatin1());

    const int N = sectPolylines.arraySize / sectPolylines.arrayModulo;
    for (int element = 0; element < N; element++) {
        quint16 t16_1, t16_2, subtyp;
        quint8  t8_1, t8_2;
        quint32 typ, offset;
        bool hasLocalization = false;
        bool hasTextColor = false;
        bool renderMode = false;
        quint8 ctyp, rows;
        quint8 r,g,b;
        quint8 langcode;

        in.device()->seek( sectPolylines.arrayOffset + (sectPolylines.arrayModulo * element ) );

        if (sectPolylines.arrayModulo == 5) {
            in >> t16_1 >> t16_2 >> t8_1;
            offset = t16_2|(t8_1<<16);
        }
        else if (sectPolylines.arrayModulo == 4) {
            in >> t16_1 >> t16_2;
            offset = t16_2;
        }
        else if (sectPolylines.arrayModulo == 3) {
            in >> t16_1 >> t8_1;
            offset = t8_1;
        }

        t16_2   = (t16_1 >> 5) | (( t16_1 & 0x1f) << 11);
        typ     = t16_2 & 0x7F;
        subtyp  = t16_1 & 0x1F;

        if(t16_1 & 0x2000) {
            typ = 0x10000|(typ << 8)|subtyp;
        }


        in.device()->seek(sectPolylines.dataOffset + offset);
        in >> t8_1 >> t8_2;
        ctyp = t8_1 & 0x07;
        rows = t8_1 >> 3;

        hasLocalization = t8_2 & 0x01;
        renderMode      = t8_2 & 0x02;
        hasTextColor    = t8_2 & 0x04;


#ifdef DBG
        qDebug() << "Polyline typ:" << hex << typ << "ctyp:" << ctyp << "offset:" << (sectPolylines.dataOffset + offset) << "orig data:" << t16_1;
#endif

        polyline_property& property = polylines[typ];
#ifdef DBG
        qDebug() << "rows" << rows << "t8_2" << hex << t8_2;
#endif

        switch(ctyp){
            case 0x00:
            {
                if(rows){
                    QImage xpm(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm.setColor(1, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm.setColor(0, qRgb(r,g,b) );
                    decodeBitmap(in, xpm, 32, rows, 1);
                    property.imgDay     = xpm;
                    property.imgNight   = xpm;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1, w2;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penBorderDay   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderNight = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> w1 >> w2;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.penBorderDay.setWidth(w2);
                    property.penBorderNight.setWidth(w2);
                    property.hasBorder  = w2 > w1;
                    property.hasPixmap  = false;
                    property.known      = true;
                }

                break;
            }
            case 0x01:
            {
                if(rows){
                    QImage xpm1(32, rows, QImage::Format_Indexed8 );
                    QImage xpm2(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm1.setColor(1, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm1.setColor(0, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm2.setColor(1, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm2.setColor(0, qRgb(r,g,b) );
                    decodeBitmap(in, xpm1, 32, rows, 1);
                    memcpy(xpm2.bits(), xpm1.bits(), (32*rows));
                    property.imgDay     = xpm1;
                    property.imgNight   = xpm2;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1, w2;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penBorderDay   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penBorderNight = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> w1 >> w2;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.penBorderDay.setWidth(w2);
                    property.penBorderNight.setWidth(w2);
                    property.hasBorder  = w2 > w1;
                    property.hasPixmap  = false;
                    property.known      = true;
                }
                break;
            }
            case 0x03:
            {
                if(rows){
                    QImage xpm1(32, rows, QImage::Format_Indexed8 );
                    QImage xpm2(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm1.setColor(1, qRgb(r,g,b) );
                    xpm1.setColor(0, qRgba(255,255,255,0) );
                    in >> b >> g >> r;
                    xpm2.setColor(1, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm2.setColor(0, qRgb(r,g,b) );
                    decodeBitmap(in, xpm1, 32, rows, 1);
                    memcpy(xpm2.bits(), xpm1.bits(), (32*rows));
                    property.imgDay     = xpm1;
                    property.imgNight   = xpm2;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1, w2;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderDay   = QPen(Qt::NoPen);
                    in >> b >> g >> r;
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penBorderNight = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> w1 >> w2;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.penBorderDay.setWidth(w2);
                    property.penBorderNight.setWidth(w2);
                    property.hasBorder  = w2 > w1;
                    property.hasPixmap  = false;
                    property.known      = true;
                }

                break;
            }
            case 0x05:
            {
                if(rows){
                    QImage xpm1(32, rows, QImage::Format_Indexed8 );
                    QImage xpm2(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm1.setColor(1, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm1.setColor(0, qRgb(r,g,b) );
                    in >> b >> g >> r;
                    xpm2.setColor(1, qRgb(r,g,b) );
                    xpm2.setColor(0, qRgba(255,255,255,0) );
                    decodeBitmap(in, xpm1, 32, rows, 1);
                    memcpy(xpm2.bits(), xpm1.bits(), (32*rows));
                    property.imgDay     = xpm1;
                    property.imgNight   = xpm2;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penBorderDay   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    in >> b >> g >> r;
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderNight = QPen(Qt::NoPen);
                    in >> w1;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.hasBorder  = false;
                    property.hasPixmap  = false;
                    property.known      = true;
                }
                break;
            }
            case 0x06:
            {
                if(rows){
                    QImage xpm(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm.setColor(1, qRgb(r,g,b) );
                    xpm.setColor(0, qRgba(255,255,255,0) );
                    decodeBitmap(in, xpm, 32, rows, 1);
                    property.imgDay     = xpm;
                    property.imgNight   = xpm;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderDay   = QPen(Qt::NoPen);
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderNight = QPen(Qt::NoPen);
                    in >> w1;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.hasBorder  = false;
                    property.hasPixmap  = false;
                    property.known      = true;
                }
                break;
            }
            case 0x07:
            {
                if(rows){
                    QImage xpm1(32, rows, QImage::Format_Indexed8 );
                    QImage xpm2(32, rows, QImage::Format_Indexed8 );
                    in >> b >> g >> r;
                    xpm1.setColor(1, qRgb(r,g,b) );
                    xpm1.setColor(0, qRgba(255,255,255,0) );
                    in >> b >> g >> r;
                    xpm2.setColor(1, qRgb(r,g,b) );
                    xpm2.setColor(0, qRgba(255,255,255,0) );
                    decodeBitmap(in, xpm1, 32, rows, 1);
                    memcpy(xpm2.bits(), xpm1.bits(), (32*rows));
                    property.imgDay     = xpm1;
                    property.imgNight   = xpm2;
                    property.hasPixmap  = true;
                    property.known      = true;
                }
                else{
                    quint8 w1;
                    in >> b >> g >> r;
                    property.penLineDay     = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderDay   = QPen(Qt::NoPen);
                    in >> b >> g >> r;
                    property.penLineNight   = QPen(QBrush(qRgb(r,g,b)), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    property.penBorderNight = QPen(Qt::NoPen);
                    in >> w1;
                    property.penLineDay.setWidth(w1);
                    property.penLineNight.setWidth(w1);
                    property.hasBorder  = false;
                    property.hasPixmap  = false;
                    property.known      = true;
                }
                break;
            }

            default:
                if(!tainted) {
                    QMessageBox::warning(0, tr("Warning..."), tr("This is a typ file with unknown polyline encoding. Please report!"), QMessageBox::Abort, QMessageBox::Abort);
                    tainted = true;
                }

                qDebug() << "Failed polyline" <<  hex << ":" << typ <<  ctyp << rows ;
                continue;
        }

    }
    return true;
}

