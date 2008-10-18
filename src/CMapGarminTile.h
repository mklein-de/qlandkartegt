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

#ifndef CMAPGARMINTILE_H
#define CMAPGARMINTILE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QVector>
#include <QRectF>

#include "Garmin.h"

class QFile;
class QByteArray;
class QPainter;

class CMapGarminTile : public QObject
{
    Q_OBJECT;
    public:
        CMapGarminTile(QObject * parent);
        virtual ~CMapGarminTile();


        /// subfile part (TRE, RGN, ...) location information
        struct subfile_part_t
        {
            subfile_part_t() : offset(0), size(0){}
            /// file offset of subfile part
            quint32 offset;
            /// size of the subfile part
            quint32 size;
        };

        struct maplevel_t
        {
            bool inherited;
            quint8 level;
            quint8 bits;
        };

        /// subdivision  information
        struct subdiv_desc_t
        {
            quint32 n;
            /// section of next level
            quint16 next;
            /// end of section group
            bool terminate;
            /// offset into the subfile's RGN part
            quint32 rgn_start;
            /// end of section in RGN part (last offset = rgn_end - 1)
            quint32 rgn_end;

            /// there are points stored in the RGN subsection
            bool hasPoints;
            /// there are indexd points stored in the RGN subsection
            bool hasIdxPoints;
            /// there are polylines stored in the RGN subsection
            bool hasPolylines;
            /// there are polygons stored in the RGN subsection
            bool hasPolygons;

            /// the center longitude of the area covered by this subdivision
            qint32 iCenterLng;
            /// the center latiude of the area covered by this subdivision
            qint32 iCenterLat;

            /// north boundary of area covered by this subsection [°]
            double north;
            /// east boundary of area covered by this subsection [°]
            double east;
            /// south boundary of area covered by this subsection [°]
            double south;
            /// west boundary of area covered by this subsection [°]
            double west;

            /// area in meter coordinates covered by this subdivision [°]
            QRectF area;

            /// number of left shifts for RGN data
            quint32 shift;
            /// map level this subdivision is shown
            quint32 level;
            /// pointer to string table (LBL section) object
//             IGarminStrTbl * strtbl;

//             QVector<CGarminPoint>               points;
//             QVector<CGarminPoint>               pois;
//             QVector<CGarminPolygon>             polylines;
            /// polygons are stored as multimap. See CGarminMap::drawPolygons()
//             QMultiMap<quint16,CGarminPolygon>   polygons;
        };

        struct subfile_desc_t
        {
            subfile_desc_t() : north(0.0), east(0.0), south(0.0), west(0.0), isTransparent(false) {}

            /// the name of the subfile (not really needed)
            QString name;
            /// location information of all parts
            QMap<QString,subfile_part_t> parts;

            /// north boundary of area covered by this subfile [rad]
            double north;
            /// east boundary of area covered by this subfile [rad]
            double east;
            /// south boundary of area covered by this subfile [rad]
            double south;
            /// west boundary of area covered by this subfile [rad]
            double west;

            /// area in [°] covered by this subfile
            QRectF area;
            /// pointer collection to definition areas
//             QMap<QString,CGarminPolygon> definitionAreas;

            /// list of subdivisions
            QVector<subdiv_desc_t> subdivs;
            /// used maplevels
            QVector<maplevel_t> maplevels;
            /// bit 1 of POI_flags (TRE header @ 0x3F)
            bool isTransparent;
        };

        enum exce_e {eErrOpen, eErrAccess, errFormat, errLock};
        struct exce_t {
            exce_t(exce_e err, const QString& msg) : err(err), msg(msg){}
            exce_e err;
            QString msg;
        };

        /// read basic information from file
        void readBasics(const QString& filename);
        /// get access to map data
        const QMap<QString,subfile_desc_t>& getSubFiles(){return subfiles;}

        bool isTransparent(){return transparent;}

        /// draw file content
        /**
            @param p the painter
            @param viewport the actual view port to draw in [°]
        */
        void draw(QPainter& p, unsigned level, double scale, const QRectF& viewport);

    private:
        void readFile(QFile& file, quint32 offset, quint32 size, QByteArray& data);
        void readSubfileBasics(subfile_desc_t& subfile, QFile& file);
        void drawPolylines(QPainter& p, unsigned level, double scale, const QRectF& viewport);


#pragma pack(1)
        // Garmin IMG file header structure, to the start of the FAT blocks
        struct hdr_img_t
        {
            quint8  xorByte;             ///< 0x00000000
            quint8  byte0x00000001_0x0000000F[15];
            char    signature[7];        ///< 0x00000010 .. 0x00000016
            quint8  byte0x00000017_0x00000040[42];
            char    identifier[7];       ///< 0x00000041 .. 0x00000047
            quint8  byte0x00000048;
            char    desc1[20];           ///< 0x00000049 .. 0x0000005C
            quint8  byte0x0000005D_0x00000060[4];
            quint8  e1;                  ///< 0x00000061
            quint8  e2;                  ///< 0x00000062
            quint8  byte0x00000063_0x00000064[2];
            char    desc2[31];           ///< 0x00000065 .. 0x00000083
            quint8  byte0x00000084_0x0000040B[904];
            quint32 dataoffset;          ///< 0x0000040C .. 0x0000040F
            quint8  byte0x00000410_0x0000041F[16];
            quint16 blocks[240];         ///< 0x00000420 .. 0x000005FF

            quint32 blocksize(){return 1 << (e1 + e2);}
        };

        struct FATblock_t
        {
            quint8  flag;                ///< 0x00000000
            char    name[8];             ///< 0x00000001 .. 0x00000008
            char    type[3];             ///< 0x00000009 .. 0x0000000B
            quint32 size;                ///< 0x0000000C .. 0x0000000F
            quint16 part;                ///< 0x00000010 .. 0x00000011
            quint8  byte0x00000012_0x0000001F[14];
            quint16 blocks[240];         ///< 0x00000020 .. 0x000001FF
        };

        // common header of the RGN, TRE, LBL, NET, ... parts of the IMG file
        struct hdr_subfile_part_t
        {
            quint16 length;              ///< 0x00000000 .. 0x00000001
            char    type[10];            ///< 0x00000002 .. 0x0000000B
            quint8  byte0x0000000C;
            quint8  flag;                ///< 0x0000000D
            quint8  byte0x0000000E_0x00000014[7];
        };

        // TRE part header, to 0xB7
        struct hdr_tre_t : public hdr_subfile_part_t
        {
            quint24 northbound;          ///< 0x00000015 .. 0x00000017
            quint24 eastbound;           ///< 0x00000018 .. 0x0000001A
            quint24 southbound;          ///< 0x0000001B .. 0x0000001D
            quint24 westbound;           ///< 0x0000001E .. 0x00000020
            quint32 tre1_offset;         ///< 0x00000021 .. 0x00000024
            quint32 tre1_size;           ///< 0x00000025 .. 0x00000028
            quint32 tre2_offset;         ///< 0x00000029 .. 0x0000002C
            quint32 tre2_size;           ///< 0x0000002D .. 0x00000030
            quint32 tre3_offset;         ///< 0x00000031 .. 0x00000034
            quint32 tre3_size;           ///< 0x00000035 .. 0x00000038
            quint16 tre3_rec_size;       ///< 0x00000039 .. 0x0000003A
            quint8  byte0x0000003B_0x0000003E[4];
            quint8  POI_flags;           ///< 0x0000003F
            quint8  byte0x00000040_0x00000049[10];
            quint32 tre4_offset;         ///< 0x0000004A .. 0x0000004D
            quint32 tre4_size;           ///< 0x0000004E .. 0x00000051
            quint16 tre4_rec_size;       ///< 0x00000052 .. 0x00000053
            quint8  byte0x00000054_0x00000057[4];
            quint32 tre5_offset;         ///< 0x00000058 .. 0x0000005B
            quint32 tre5_size;           ///< 0x0000005C .. 0x0000005F
            quint16 tre5_rec_size;       ///< 0x00000060 .. 0x00000061
            quint8  byte0x00000062_0x00000065[4];
            quint32 tre6_offset;         ///< 0x00000066 .. 0x00000069
            quint32 tre6_size;           ///< 0x0000006A .. 0x0000006D
            quint16 tre6_rec_size;       ///< 0x0000006E .. 0x0000006F
            quint8  byte0x00000070_0x00000073[4];
            /*-----------------------------------------------------*/
            quint8  byte0x00000074_0x0000007B[8];
            quint32 tre7_offset;         ///< 0x0000007C .. 0x0000007F
            quint32 tre7_size;           ///< 0x00000080 .. 0x00000083
            quint16 tre7_rec_size;       ///< 0x00000084 .. 0x00000085
            quint8  byte0x00000086_0x00000089[4];
            quint32 tre8_offset;         ///< 0x0000008A .. 0x0000008D
            quint32 tre8_size;           ///< 0x0000008E .. 0x00000091
            quint8  byte0x00000092_0x00000099[8];
            /*-----------------------------------------------------*/
            quint8  key[20];             ///< 0x0000009A .. 0x000000AD
            quint32 tre9_offset;         ///< 0x000000AE .. 0x000000B1
            quint32 tre9_size;           ///< 0x000000B2 .. 0x000000B5
            quint16 tre9_rec_size;       ///< 0x000000B6 .. 0x000000B7

        };

        // RGN part header
        struct hdr_rgn_t : public hdr_subfile_part_t
        {
            quint32 offset;              ///< 0x00000015 .. 0x00000018
            quint32 length;              ///< 0x00000019 .. 0x0000000C
        };


#define TRE_MAP_LEVEL(r) ((r)->zoom & 0x0f)
#define TRE_MAP_INHER(r) (((r)->zoom & 0x80) != 0)

        // map level definition
        struct tre_map_level_t
        {
            quint8 zoom;
            quint8  bits;
            quint16 nsubdiv;
        };

        // map subdivision definition, without pointer to the lower level subparts
        struct tre_subdiv_t
        {
            quint24 rgn_offset;
            quint8  elements;
            quint24 center_lng;
            quint24 center_lat;
            quint16 width_trm;
        #define TRE_SUBDIV_WIDTH(r)    (gar_load(uint16_t, (r)->width_trm) & 0x7FFF)
        #define TRE_SUBDIV_TERM(r)     ((gar_load(uint16_t, (r)->width_trm) & 0x8000) != 0)
            quint16 height;
        };

        // pointer to the lower level subparts
        struct tre_subdiv_next_t : public tre_subdiv_t
        {
            quint16 next;
        };
#pragma pack(0)
        /// the tile's filename
        QString filename;
        /// xor mask for encrypted files
        quint8 mask;
        /// map description string
        QString mapdesc;
        /// hold all subfile descriptors
        /**
            In a normal *.img file there is only one subfile. However
            gmapsupp.img files can hold several subfiles each with it's
            own subfile parts.
        */
        QMap<QString,subfile_desc_t> subfiles;
        /// relay the transparent flags from the subfiles
        bool transparent;

};

#endif //CMAPGARMINTILE_H

