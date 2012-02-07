#include "CInputFile.h"

#include <QtCore>
#include <stdio.h>
#include <ogr_spatialref.h>


CInputFile::CInputFile(const QString &filename)
    : filename(filename)
{
    double adfGeoTransform[6]   = {0};
    char projstr[1024]          = {0};
    OGRSpatialReference oSRS;

    OGRSpatialReference oSRS_EPSG31466;
    oSRS_EPSG31466.importFromProj4("+init=epsg:31466");
    OGRSpatialReference oSRS_EPSG31467;
    oSRS_EPSG31467.importFromProj4("+init=epsg:31467");
    OGRSpatialReference oSRS_EPSG31468;
    oSRS_EPSG31468.importFromProj4("+init=epsg:31468");
    OGRSpatialReference oSRS_EPSG31469;
    oSRS_EPSG31469.importFromProj4("+init=epsg:31469");


    dataset = (GDALDataset*)GDALOpen(filename.toLocal8Bit(),GA_ReadOnly);
    if(dataset == 0)
    {
        fprintf(stderr,"\nFailed to open %s\n", filename.toLocal8Bit().data());
        exit(-1);
    }


    char * ptr = projstr;

    strncpy(projstr,dataset->GetProjectionRef(),sizeof(projstr));
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);

    pj = pj_init_plus(ptr);
    if(pj == 0)
    {
        fprintf(stderr,"\nUnknown projection in file %s\n", filename.toLocal8Bit().data());
        exit(-1);
    }

    qDebug();
    qDebug() << filename;
    qDebug() << ptr;

    if(oSRS.IsSame(&oSRS_EPSG31467))
    {
        compeProj   = "117,GK-System 9º (Zone 3),";
        compeDatum  = "Potsdam Rauenberg DHDN";
    }
    else if(oSRS.IsSame(&oSRS_EPSG31468))
    {
        compeProj   = "114,GK-System 12º (Zone 4),";
        compeDatum  = "Potsdam Rauenberg DHDN";
    }
    else
    {
        fprintf(stderr,"\n%s\nprojection in file %s not recognized\n", ptr, filename.toLocal8Bit().data());
        exit(-1);
    }

    qDebug() << compeProj + compeDatum;
    dataset->GetGeoTransform( adfGeoTransform );

    width   = dataset->GetRasterXSize();
    height  = dataset->GetRasterYSize();
    xscale  = adfGeoTransform[1];
    yscale  = adfGeoTransform[5];
    xref1   = adfGeoTransform[0];
    yref1   = adfGeoTransform[3];

    qDebug() << QString("level0:") << width << height << xscale << yscale;

    GDALRasterBand * band = dataset->GetRasterBand(1);

    for(int i=0; i < band->GetOverviewCount(); i++)
    {
        GDALRasterBand * bandOvr = band->GetOverview(i);
        double w = bandOvr->GetXSize();
        double h = bandOvr->GetYSize();

        qDebug() << QString("level%1:").arg(i+1) << w << h << xscale * width/w << yscale * height/h;
    }


}

CInputFile::~CInputFile()
{

}

