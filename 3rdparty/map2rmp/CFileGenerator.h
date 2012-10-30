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

        friend bool qSortInFiles(CFileGenerator::file_t& f1, CFileGenerator::file_t& f2);

        projPJ  epsg4326;


        QStringList input;
        QString output;
        int quality;
        int subsampling;

        QList<file_t> infiles;
};

#endif //CFILEGENERATOR_H

