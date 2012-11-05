/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CFILEGENERATOR_H
#define CFILEGENERATOR_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <proj_api.h>

class GDALDataset;

#define MAX_ZOOM_LEVEL 18

class CFileGenerator
{
    public:
        CFileGenerator(const QStringList &input, const QString &output, const QString& provider, const QString& product, int quality, int subsampling);
        virtual ~CFileGenerator();

        int start();

    private:

        struct scale_t
        {
            double xscale;
            double yscale;
        };

        struct file_level_t
        {
            double xscale;
            double yscale;
            int xsize;
            int ysize;
        };

        struct file_t
        {

            void convertPx2Deg(double& u, double& v);
            void convertDeg2Px(double& u, double& v);

            void roundDeg2Tile(double& u, double& v);
            void roundPx2Tile(double& u, double& v);

            QString name;
            GDALDataset * dataset;
            quint32 colortable[256];

            int xsize;
            int ysize;
            double xscale;
            double yscale;
            double lon1;
            double lat1;
            double lon2;
            double lat2;

            file_level_t level;
        };


        struct rmp_tile_t
        {
            int x1;
            int y1;
            int x2;
            int y2;

            double lon1;
            double lat1;
            double lon2;
            double lat2;

            qint32 offset;

        };

        struct rmp_big_tile_t
        {
            file_t * src;

            int x1;
            int y1;
            int x2;
            int y2;

            double lon1;
            double lat1;
            double lon2;
            double lat2;

            QVector<rmp_tile_t> tiles;
        };


        struct rmp_level_t
        {
            file_t * src;

            int x1;
            int y1;
            int x2;
            int y2;

            double lon1;
            double lat1;
            double lon2;
            double lat2;

            quint32 nTiles;

            QVector<rmp_big_tile_t> bigTiles;
        };

        struct rmp_dir_entry_t
        {
            rmp_dir_entry_t(){memset(this, 0, sizeof(rmp_dir_entry_t));}

            char name[9];
            char ext[7];
            quint32 offset;
            quint32 length;
        };

        struct rmp_file_t
        {
            int index;
            QString name;
            QVector<rmp_dir_entry_t> directory;
            QVector<rmp_level_t> levels;
        };


        projPJ  epsg4326;

        QStringList input;
        QString output;
        QString provider;
        QString product;
        int quality;
        int subsampling;

        QByteArray tileBuf08Bit;
        QByteArray tileBuf24Bit;

        quint32 nTilesTotal;
        quint32 nTilesProcessed;

        scale_t scales[MAX_ZOOM_LEVEL];

        friend bool qSortInFiles(CFileGenerator::file_t& f1, CFileGenerator::file_t& f2);

        void findBestLevelScale(file_level_t& scale);

        void setupOutFile(double lon1, double lat1, double lon2, double lat2, QList<file_t>& infiles, rmp_file_t &rmp);
        void setupBigTile(int x, int y, rmp_level_t &level, rmp_big_tile_t &bigTile);
        void setupTile(int x, int y, rmp_big_tile_t &bigTile, rmp_tile_t &tile);

        quint16 crc16(QDataStream& stream, qint32 length);

        void writeRmp(rmp_file_t& rmp);
        void writeDirectory(QDataStream& stream, rmp_file_t& rmp);
        void writeBmp2Bit(QDataStream& stream, rmp_file_t& rmp);
        void writeBmp4Bit(QDataStream& stream, rmp_file_t& rmp);
        void writeCvgMap(QDataStream& stream, rmp_file_t& rmp);
        void writeRmpIni(QDataStream& stream, rmp_file_t& rmp);
        void writeA00(QDataStream& stream, rmp_file_t& rmp, int i);
        void writeTLM(QDataStream& stream, rmp_file_t& rmp, int i);


        bool readTile(file_t& file, const qint32 xoff, const qint32 yoff, const qint32 w1, const qint32 h1, quint32 *output);
        quint32 writeTile(quint32 xsize, quint32 ysize, quint32 * raw_image, int quality, int subsampling);
};

#endif //CFILEGENERATOR_H

