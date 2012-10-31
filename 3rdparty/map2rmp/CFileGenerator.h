/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CFileGenerator.h

  Module:      

  Description:

  Created:     10/30/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CFILEGENERATOR_H
#define CFILEGENERATOR_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <proj_api.h>

class GDALDataset;

class CFileGenerator
{
    public:
        CFileGenerator(const QStringList &input, const QString &output, int quality, int subsampling);
        virtual ~CFileGenerator();

        int start();

    private:

        struct file_t
        {

            void convertPx2Deg(double& u, double& v);
            void convertDeg2Px(double& u, double& v);

            QString name;
            GDALDataset * dataset;

            int xsize;
            int ysize;
            int xtiles;
            int ytiles;
            double xscale;
            double yscale;
            double lon1;
            double lat1;
        };


        struct rmp_tile_t
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

            QVector<rmp_big_tile_t> bigTiles;
        };

        struct rmp_file_t
        {
            int index;
            QString name;
            QString product;
            QString provider;

            QVector<rmp_level_t> levels;
        };


        projPJ  epsg4326;

        QStringList input;
        QString output;
        int quality;
        int subsampling;

        QList<file_t> infiles;
        QVector<rmp_file_t> outfiles;

        friend bool qSortInFiles(CFileGenerator::file_t& f1, CFileGenerator::file_t& f2);
        void setupOutFile(int x, int y, QList<file_t>& infiles, rmp_file_t &rmp);
        void setupBigTile(int x, int y, rmp_level_t &level, rmp_big_tile_t &bigTile);
        void setupTile(int x, int y, rmp_big_tile_t &bigTile, rmp_tile_t &tile);
};

#endif //CFILEGENERATOR_H

