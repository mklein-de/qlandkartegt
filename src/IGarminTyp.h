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

#ifndef IGARMINTYP_H
#define IGARMINTYP_H

#include <QtGui>

class IGarminTyp : public QObject
{
    Q_OBJECT;
    public:
        IGarminTyp(QObject * parent);
        virtual ~IGarminTyp();

        struct polyline_property
        {
            polyline_property(): type(0), pen0(Qt::magenta,3), known(false), showText(true){};
            polyline_property(quint16 type, const QColor& color0, qreal width, Qt::PenStyle style)
                : type(type)
                , pen0(QBrush(color0), width, style, Qt::RoundCap, Qt::RoundJoin)
                , known(true)
                , showText(true)
                {}
            quint16 type;
            QPen    pen0;
            QFont   font;
            bool    known;
            QImage  pixmap;
            bool   showText;
        };

        struct polygon_property
        {
            polygon_property() : type(0), pen(Qt::magenta), brush(Qt::magenta, Qt::BDiagPattern), known(false){}
            polygon_property(quint16 type, const Qt::PenStyle pensty, const QColor& brushColor, Qt::BrushStyle pattern)
                : type(type)
                , pen(pensty)
                , brush(brushColor, pattern)
                , known(true)
                {pen.setWidth(1);}
            polygon_property(quint16 type, const QColor& penColor, const QColor& brushColor, Qt::BrushStyle pattern)
                : type(type)
                , pen(penColor,1)
                , brush(brushColor, pattern)
                , known(true)
                {}
            quint16 type;
            QPen    pen;
            QBrush  brush;
            QFont   font;
            bool    known;
        };

        /// decode typ file
        /**
            This pure virtual function has to be implemented in every subclass. It should
            be the only public function needed. The typ file is read and it's content is
            stored in the passed map/list objects.

            @param in input data stream
            @param polygons reference to polygon properties map
            @param polylines reference to polyline properties map
            @param drawOrder reference to list of polygon draw orders
            @param points reference to point properties map

        */
        virtual bool decode(QDataStream& in, QMap<quint32, polygon_property>& polygons, QMap<quint32, polyline_property>& polylines, QList<quint32> drawOrder, QMap<quint32, QImage>& points) = 0;

    protected:
        virtual bool parseHeader(QDataStream& in);
        virtual bool parseDrawOrder(QDataStream& in, QList<quint32> drawOrder);
        virtual bool parsePolygon(QDataStream& in, QMap<quint32, polygon_property>& polygons);


        struct typ_section_t
        {
            typ_section_t() : dataOffset(0), dataLength(0), arrayOffset(0), arrayModulo(0), arraySize(0){};
            quint32  dataOffset;
            quint32  dataLength;
            quint32  arrayOffset;
            quint16  arrayModulo;
            quint32  arraySize;
        } ;

        quint16 version;
        quint16 codepage;
        quint16 year;
        quint8  month;
        quint8  day;
        quint8  hour;
        quint8  minutes;
        quint8  seconds;

        quint16 fid;
        quint16 pid;

        typ_section_t sectPoints;
        typ_section_t sectPolylines;
        typ_section_t sectPolygons;
        typ_section_t sectOrder;


};

#endif //IGARMINTYP_H

