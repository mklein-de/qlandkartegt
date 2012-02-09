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
#include <wctype.h>

#include <gdal_priv.h>
#include <projects.h>
#include <ogr_spatialref.h>

#include <QtCore>

#include "CInputFile.h"

#ifndef _MKSTR_1
#define _MKSTR_1(x)         #x
#define _MKSTR(x)           _MKSTR_1(x)
#endif

#define VER_STR             _MKSTR(VER_MAJOR)"."_MKSTR(VER_MINOR)"."_MKSTR(VER_STEP)
#define WHAT_STR            "map2rmap, Version " VER_STR

#define TILESIZE            256

/// the target lon/lat WGS84 projection
static PJ * wgs84 = 0;

static bool qSortInFiles(CInputFile& f1, CInputFile& f2)
{
    return f1.getXScale() < f2.getXScale();
}

int main(int argc, char ** argv)
{
    quint32 nLevels             =  0;
    quint64 posMapDataOffset    =  0;
    quint64 posMapData          =  0;
    int quality                 = -1;
    int subsampling             = -1;
    int skip_next_arg           =  0;
    QString outfile;
    QList<CInputFile> infiles;



    printf("\n****** %s ******\n", WHAT_STR);

    if(argc < 2)
    {
        fprintf(stderr,"\nusage: map2rpm -q <1..100> -s <411|422|444> <file1> <file2> ... <fileN> <outputfile>\n");
        fprintf(stderr,"\n");
        fprintf(stderr,"  -q The JPEG quality from 1 to 100. Default is 75 \n");
        fprintf(stderr,"  -s The chroma subsampling. Default is 411  \n");

        fprintf(stderr,"\n");
        fprintf(stderr,"\n");
        exit(-1);
    }

    GDALAllRegister();
    wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");


    for(int i = 1; i < (argc - 1); i++)
    {

        while (skip_next_arg)
        {
            skip_next_arg--;
            continue;
        }

        if (argv[i][0] == '-')
        {
            if (towupper(argv[i][1]) == 'Q')
            {
                quality = atol(argv[i+1]);
                skip_next_arg = 1;
                continue;
            }
            else if (towupper(argv[i][1]) == 'S')
            {
                subsampling = atol(argv[i+1]);
                skip_next_arg = 1;
                continue;
            }
        }

        infiles << CInputFile(argv[i], TILESIZE);
    }
    outfile = argv[argc-1];

    qSort(infiles.begin(), infiles.end(), qSortInFiles);

    for(int i=0; i < infiles.size() - 1; i++)
    {
        nLevels += infiles[i].calcLevels(infiles[i+1].getXScale());
    }
    nLevels += infiles.last().calcLevels(0.0);

    CInputFile& base = infiles.first();

    QFile file(outfile);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // write file header
    stream.writeRawData("CompeGPSRasterImage",19);
    stream << quint32(10) << quint32(7) << quint32(0);
    stream << base.getWidth() << -base.getHeight();
    stream << quint32(24) << quint32(1);
    stream << quint32(TILESIZE) << quint32(TILESIZE);

    posMapDataOffset = file.pos();

    stream << quint64(0); // MapDataOffset
    stream << quint32(0);
    stream << nLevels;
    for(int i = 0; i < nLevels; i++)
    {
        stream << quint64(0);
    }

    // write layers

    for(int i = 0; i < infiles.size(); i++)
    {
        infiles[i].writeLevels(stream, quality, subsampling);
    }

    posMapData = file.pos();

    file.seek(posMapDataOffset);
    stream << posMapData;
    stream << quint32(0);
    stream << nLevels;
    for(int i = 0; i < infiles.size(); i++)
    {
        infiles[i].writeLevelOffsets(stream);
    }

    double lon1, lat1;
    double lon2, lat2;
    base.getRef1Deg(lon1, lat1);
    base.getRef2Deg(lon2, lat2);

    QString mapdata;
    mapdata += "CompeGPS MAP File\r\n";
    mapdata += "<Header>\r\n";
    mapdata += "Version=2\r\n";
    mapdata += "VerCompeGPS=QLandkarte GT\r\n";
    mapdata += "Projection=2,Mercator,\r\n";
    mapdata += "Coordinates=1\r\n";
    mapdata += "Datum=WGS 84\r\n";
    mapdata += "</Header>\r\n";
    mapdata += "<Map>\r\n";
    mapdata += "Bitmap=" + QFileInfo(outfile).fileName() + "\r\n";
    mapdata += "BitsPerPixel=0\r\n";
    mapdata += QString("BitmapWidth=%1\r\n").arg(base.getWidth());
    mapdata += QString("BitmapHeight=%1\r\n").arg(base.getHeight());
    mapdata += "Type=10\r\n";
    mapdata += "</Map>\r\n";
    mapdata += "<Calibration>\r\n";
    mapdata += QString("P0=0,0,A,%1,%2\r\n").arg(lon1,0,'f').arg(lat1,0,'f');
    mapdata += QString("P1=%3,0,A,%1,%2\r\n").arg(lon2,0,'f').arg(lat1,0,'f').arg(base.getWidth() - 1);
    mapdata += QString("P2=%3,%4,A,%1,%2\r\n").arg(lon2,0,'f').arg(lat2,0,'f').arg(base.getWidth() - 1).arg(base.getHeight() - 1);
    mapdata += QString("P4=0,%3,A,%1,%2\r\n").arg(lon1,0,'f').arg(lat2,0,'f').arg(base.getHeight() - 1);
    mapdata += "</Calibration>\r\n";
    mapdata += "<MainPolygonBitmap>\r\n";
    mapdata += QString("M0=0,0\r\n");
    mapdata += QString("M1=%1,0\r\n").arg(base.getWidth());
    mapdata += QString("M2=%1,%2\r\n").arg(base.getWidth()).arg(base.getHeight());
    mapdata += QString("M3=0,%1\r\n").arg(base.getHeight());
    mapdata += "</MainPolygonBitmap>\r\n";

    file.seek(posMapData);
    stream << quint32(1) << mapdata.size();
    stream.writeRawData(mapdata.toAscii(), mapdata.size());

    pj_free(wgs84);
    GDALDestroyDriverManager();
    printf("\n");

    return 0;
}
