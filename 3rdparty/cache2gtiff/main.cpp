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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wctype.h>

#include <gdal_priv.h>
#include <projects.h>
#include <ogr_spatialref.h>

#include <QtCore>
#include <QtXml>

#include "CDiskCache.h"

#ifndef _MKSTR_1
#define _MKSTR_1(x)         #x
#define _MKSTR(x)           _MKSTR_1(x)
#endif

#define VER_STR             _MKSTR(VER_MAJOR)"."_MKSTR(VER_MINOR)"."_MKSTR(VER_STEP)
#define WHAT_STR            "cache2gtiff, Version " VER_STR



//bin/cache2gtiff -a 1 12.070069 49.052840 12.153837 48.998965 -c /tmp/qlandkarteqt-oeichler/cache -i /home/oeichler/dateien/Maps/bayern_dop_wms.xml -o test.tif

int main(int argc, char ** argv)
{
    PJ * pjsrc;
    int level;
    double lon1, lat1, lon2, lat2;
    QString infile;
    QString outfile;
    QString cachePath;

    printf("\n****** %s ******\n", WHAT_STR);

    if(argc < 2)
    {
        fprintf(stderr,"\nusage: cache2gtiff -a level lon1 lat1 lon2 lat2 -c <path> -i <file|url> -o <file> \n");
        fprintf(stderr,"\n");
        fprintf(stderr,"  -a The level and the area to export.\n");
        fprintf(stderr,"     The level is an integer from 1..19\n");
        fprintf(stderr,"     All lon/lat values are in degree\n");
        fprintf(stderr,"     lon1, lat1 is the top left corner of the area\n");
        fprintf(stderr,"     lon2, lat2 is the bottom right corner of the area\n");
        fprintf(stderr,"  -c The path to the tile cache\n");
        fprintf(stderr,"  -i The xml definition or an url of the server\n");
        fprintf(stderr,"  -o the target geotiff filename\n");
        fprintf(stderr,"\n");
        fprintf(stderr,"\n");
        exit(-1);
    }

    GDALAllRegister();

    int skip_next_arg = 0;
    for(int i = 1; i < (argc - 1); i++)
    {
        while(skip_next_arg)
        {
            skip_next_arg--;
            continue;
        }

        if (argv[i][0] == '-')
        {
            if (towupper(argv[i][1]) == 'A')
            {
                level   = atol(argv[i+1]);
                lon1    = atof(argv[i+2]);
                lat1    = atof(argv[i+3]);
                lon2    = atof(argv[i+4]);
                lat2    = atof(argv[i+5]);
                skip_next_arg = 5;
                continue;
            }
            else if (towupper(argv[i][1]) == 'C')
            {
                cachePath = QString(argv[i+1]);
                skip_next_arg = 1;
                continue;
            }
            else if (towupper(argv[i][1]) == 'I')
            {
                infile = QString(argv[i+1]);
                skip_next_arg = 1;
                continue;
            }
            else if (towupper(argv[i][1]) == 'O')
            {
                outfile = QString(argv[i+1]);
                skip_next_arg = 1;
                continue;
            }

        }
    }

    CDiskCache diskCache(cachePath,0);


    QFile file(infile);
    if(!file.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Failed to open %s\n", infile.toLocal8Bit().data());
        exit(-1);
    }

    QString msg;
    int line, column;
    QDomDocument dom;
    if(!dom.setContent(&file, true, &msg, &line, &column))
    {
        file.close();
        fprintf(stderr, "Failed to read: %s\nline %i, column %i:\n %s", infile.toLocal8Bit().data(), line, column, msg.toLocal8Bit().data());
        exit(-1);
    }
    file.close();

    QDomElement gdal        = dom.firstChildElement("GDAL_WMS");
    QDomElement service     = gdal.firstChildElement("Service");
    QDomElement datawindow  = gdal.firstChildElement("DataWindow");

    QString srs = service.firstChildElement("SRS").text();

    srs = srs.toLower();
    if(srs.startsWith("epsg"))
    {
        QString str = QString("+init=%1").arg(srs);
        pjsrc = pj_init_plus(str.toLocal8Bit());
        qDebug() << "projection:" << str.toLocal8Bit();
    }
    else
    {
        pjsrc = pj_init_plus(srs.toLocal8Bit());
        qDebug() << "projection:" << srs.toLocal8Bit();
    }


    if(pjsrc == 0)
    {
        fprintf(stderr, "Unknown projection %s", srs.toLocal8Bit().data());
        exit(-1);
    }


    int blockSizeX  = gdal.firstChildElement("BlockSizeX").text().toUInt();
    int blockSizeY  = gdal.firstChildElement("BlockSizeY").text().toUInt();

    int xsize_px    = datawindow.firstChildElement("SizeX").text().toInt();
    int ysize_px    = datawindow.firstChildElement("SizeY").text().toInt();

    double xref1, yref1, xref2, yref2;
    if (pj_is_latlong(pjsrc))
    {
        xref1 = datawindow.firstChildElement("UpperLeftX").text().toDouble()  * DEG_TO_RAD;
        yref1 = datawindow.firstChildElement("UpperLeftY").text().toDouble()  * DEG_TO_RAD;
        xref2 = datawindow.firstChildElement("LowerRightX").text().toDouble() * DEG_TO_RAD;
        yref2 = datawindow.firstChildElement("LowerRightY").text().toDouble() * DEG_TO_RAD;
    }
    else
    {
        xref1 = datawindow.firstChildElement("UpperLeftX").text().toDouble();
        yref1 = datawindow.firstChildElement("UpperLeftY").text().toDouble();
        xref2 = datawindow.firstChildElement("LowerRightX").text().toDouble();
        yref2 = datawindow.firstChildElement("LowerRightY").text().toDouble();
    }

    double xscale   = (xref2 - xref1) / xsize_px;
    double yscale   = (yref2 - yref1) / ysize_px;



    GDALDestroyDriverManager();
    printf("\n");
    return 0;
}
