/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CFileGenerator.cpp

  Module:      

  Description:

  Created:     10/30/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CFileGenerator.h"

#include <stdio.h>
#include <math.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#define TILESIZE            256

bool qSortInFiles(CFileGenerator::file_t& f1, CFileGenerator::file_t& f2)
{
    return f1.xscale < f2.xscale;
}


CFileGenerator::CFileGenerator(const QStringList& input, const QString& output, int quality, int subsampling)
    : input(input)
    , output(output)
    , quality(quality)
    , subsampling(subsampling)
{
    epsg4326 = pj_init_plus("+init=epsg:4326");
}

CFileGenerator::~CFileGenerator()
{
    if(epsg4326) pj_free(epsg4326);
}

int CFileGenerator::start()
{
    fprintf(stdout,"analyze input files:\n");

    OGRSpatialReference oSRS_EPSG4326;
    oSRS_EPSG4326.importFromProj4("+init=epsg:4326");

    foreach(const QString& filename, input)
    {

        OGRSpatialReference oSRS;
        char projstr[1024]          = {0};
        double adfGeoTransform[6]   = {0};
        file_t file;

        file.name = filename;

        file.dataset = (GDALDataset*)GDALOpen(filename.toLocal8Bit(),GA_ReadOnly);
        if(file.dataset == 0)
        {
            fprintf(stderr,"\nFailed to open %s\n", filename.toLocal8Bit().data());
            exit(-1);
        }

        char * ptr = projstr;
        strncpy(projstr,file.dataset->GetProjectionRef(),sizeof(projstr));
        oSRS.importFromWkt(&ptr);

        if(!oSRS.IsSame(&oSRS_EPSG4326))
        {
            fprintf(stderr,"\nBad projection of file %s. The file must have EPSG:4326.\n", filename.toLocal8Bit().data());
            exit(-1);
        }

        file.dataset->GetGeoTransform( adfGeoTransform );
        file.xsize = file.dataset->GetRasterXSize();
        file.ysize = file.dataset->GetRasterYSize();
        file.xscale = adfGeoTransform[1];
        file.yscale = adfGeoTransform[5];
        file.lon1   = adfGeoTransform[0];
        file.lat1   = adfGeoTransform[3];

        file.xtiles = ceil(float(file.xsize)/TILESIZE);
        file.ytiles = ceil(float(file.ysize)/TILESIZE);

        infiles << file;
    }

    qSort(infiles.begin(), infiles.end(), qSortInFiles);
    foreach(const file_t& file, infiles)
    {
        fprintf(stdout, "\n%s\n", file.name.toLocal8Bit().data());
        fprintf(stdout, "lon/lat:   %1.6f %1.6f\n", file.lon1, file.lat1);
        fprintf(stdout, "x/y scale: %1.6f %1.6f\n", file.xscale, file.yscale);
        fprintf(stdout, "x/y pixel: %i %i\n", file.xsize, file.ysize);
        fprintf(stdout, "x/y tiles: %i %i\n", file.xtiles, file.ytiles);
    }

    fprintf(stdout, "\n");

    file_t& file = infiles.first();

    int nBlocksX = ceil(float(file.xtiles)/8);
    int nBlocksY = ceil(float(file.ytiles)/8);
    int nSectX   = ceil(float(nBlocksX)/8);
    int nSectY   = ceil(float(nBlocksY)/8);

    fprintf(stdout, "x/y sect: %i %i\n", nSectX, nSectY);

    fprintf(stdout, "files: %i \n", nSectX * nSectY);


    return 0;
}
