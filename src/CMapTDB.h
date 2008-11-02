/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CMAPTDB_H
#define CMAPTDB_H

#include "IMap.h"
#include "CGarminTile.h"
#include <QMap>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

class CGarminTile;
class QTimer;
class QTextDocument;

class CMapTDB : public IMap
{
    Q_OBJECT;
    public:
        CMapTDB(const QString& key, const QString& filename, CCanvas * parent);
        CMapTDB(const QString& key, const QString& filename);
        virtual ~CMapTDB();

        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void convertM2Pt(double* u, double* v, int n);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        const QString& getName(){return name;}
        void draw(QPainter& p);
        void draw();
        void draw(const XY& p1, const XY& p2, const QSize& size, QPainter& p);
        void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale);
        void registerDEM(CMapDEM& dem);

    protected:
        virtual void convertRad2Pt(double* u, double* v, int n);
        void resize(const QSize& s);
        bool eventFilter( QObject * watched, QEvent * event );

    private slots:
        void slotResetFastDraw();

    private:
        void setup();
        struct strlbl_t
        {
            QPoint  pt;
            QRect   rect;
            QString str;
        };

        void readTDB(const QString& filename);
        bool processPrimaryMapData();
        void drawPolylines(QPainter& p, polytype_t& lines);
        void drawPolygons(QPainter& p, polytype_t& lines);
        void drawPoints(QPainter& p, pointtype_t& points);
        void drawPois(QPainter& p, pointtype_t& points);
        void drawLabels(QPainter& p, QVector<strlbl_t> lbls);
        void setFastDraw();

        void getInfoPoints(const QPoint& pt, QMultiMap<QString, QString>& dict);
        void getInfoPois(const QPoint& pt, QMultiMap<QString, QString>& dict);
        void getInfoPolygons(const QPoint& pt, QMultiMap<QString, QString>& dict);
        void getInfoPolylines(QPoint& pt, QMultiMap<QString, QString>& dict);

#pragma pack(1)
        struct tdb_hdr_t
        {
            quint8  type;
            quint16 size;
        };

        struct tdb_product_t : public tdb_hdr_t
        {
            quint32 id;
            quint16 version;
            char *  name[1];
        };

        struct tdb_map_t : public tdb_hdr_t
        {
            quint32 id;
            quint32 country;
            qint32  north;
            qint32  east;
            qint32  south;
            qint32  west;
            char    name[1];
        };

        struct tdb_map_size_t
        {
            quint16 dummy;
            quint16 count;
            quint32 sizes[1];
        };

        struct tdb_copyright_t
        {
            quint8  type;
            quint16 count;
            quint8  flag;
            char    str[1];
        };

        struct tdb_copyrights_t : public tdb_hdr_t
        {
            tdb_copyright_t entry;
        };
#ifdef WIN32
#pragma pack()
#else
#pragma pack(0)
#endif

        struct tile_t
        {
            quint32 id;
            QString key;
            QString name;
            //             std::string cname;
            QString file;
            double north;
            double east;
            double south;
            double west;
            QRectF area;
            //             QVector<XY> definitionArea;
            CGarminTile * img;
            quint32 memSize;
        };

        struct map_level_t
        {
            quint8 bits;
            quint8 level;
            bool useBaseMap;

            bool operator==(const map_level_t &ml) {
                if (ml.bits != bits || ml.level != level || ml.useBaseMap != useBaseMap)
                    return false;
                else
                    return true;
            }

            static bool GreaterThan(const map_level_t &ml1, const map_level_t &ml2) {
                return ml1.bits < ml2.bits;
            }
        };

        /// scale entry
        struct scale_t
        {
            /// scale name
            QString label;
            /// scale factor
            double scale;
            /// minimum bits required for this resolution
            quint32 bits;
        };

        /// tdb filename
        QString filename;
        /// copyright string
        QString copyright;
        /// map collection name
        QString name;
        /// basemap filename
        QString basemap;
        /// north boundary of basemap []
        double north;
        /// east boundary of basemap []
        double east;
        /// south boundary of basemap []
        double south;
        /// west boundary of basemap []
        double west;
        /// the area in [m] covered by the basemap
        QRectF area;
        /// set true for encrypted maps
        bool encrypted;
        /// the unlock key
        QString mapkey;
        /// the basemap tile;
        CGarminTile * baseimg;
        /// high detail map tiles
        QMap<QString,tile_t> tiles;
        /// combined maplevels of basemap & submap tiles
        QVector<map_level_t> maplevels;
        /// flag for transparent maps
        bool isTransparent;

        bool needRedraw;

        /// different scale entries indexed by idxZoom,
        static scale_t scales[];
        /// the used scale
        double zoomFactor;
        /// top left corner as long / lat [rad]
        XY topLeft;
        /// top bottom right as long / lat [rad]
        XY bottomRight;

        static const QString polyline_typestr[];
        struct polyline_property
        {
            polyline_property(): type(0), pen(Qt::magenta), known(false){};
            polyline_property(quint16 type, const QColor& color, qreal width, Qt::PenStyle style)
                : type(type)
                , pen(QBrush(color), width, style, Qt::RoundCap, Qt::RoundJoin)
                , known(true)
                {}
            quint16 type;
            QPen    pen;
            QFont   font;
            bool    known;
        };

        QVector<polyline_property> polylineProperties;

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

        QVector<polygon_property> polygonProperties;

        bool doFastDraw;
        QTimer * timerFastDraw;

        polytype_t polygons;
        polytype_t polylines;
        pointtype_t points;
        pointtype_t pois;

        QFontMetrics      fm;
        QVector<strlbl_t> labels;

        QTextDocument * info;
        QString         infotext;
        QPoint          topLeftInfo;

        QPoint          pointFocus;
};
#endif                           //CMAPTDB_H
