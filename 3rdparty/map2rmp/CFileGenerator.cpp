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
#include "CFileGenerator.h"

#include <stdio.h>
#include <math.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <QtCore>

#define TILE_SIZE       256

#define N_TILES_X       9
#define N_TILES_Y       9
#define N_BIG_TILES_X   9
#define N_BIG_TILES_Y   9

#define INDEX_BMP2BIT   0
#define INDEX_BMP4BIT   1
#define INDEX_CVGMAP    2
#define INDEX_RMPINI    3

#define OFFSET_1ST_DIR  4
#define OFFSET_2ND_DIR  5

static const char bmp2bit[] =
{
    73, 99, 111, 110, 32, 102, 105, 108, 101, 32, 118, 101,
    114, 115, 105, 111, 110, 32, 49, 46, 48, 46, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0,
    0, 1, 0, 0, 60, 0, 0, 0, -16, -1, -1, 127, -40, 0, 0, 0, -15, -1, -1, 127, 116, 1, 0,
    0, 0, 1, 0, 0, -1, -1, -1, -1, 88, 0, 0, 0, -104, 0, 0, 0, 0, 0, 0, 0, 16, 0, 16, 0, 4,
    2, 0, 0, 0, 0, 0, 0, 63, -1, -1, -4, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0,
    0, 12, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0, 0, 12, 48, 0, 0,
    12, 48, 0, 0, 12, 48, 0, 0, 12, 63, -1, -1, -4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1,
    -64, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1, -64, 3, -1, -1, -64, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -16, -1, -1, 127, -1, -1, -1, -1, -12, 0, 0, 0, 52, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 64, 0, 4, 8, 0, 0, 81, -3, -40, -70, -124, 24, -99, -19, 46,
    -36, -3, -21, -128, 98, 96, -75, 82, -99, 78, 108, -79, 18, -9, -36, -20, -88, -41, 84,
    -99, 88, 98, 40, 74, 93, 118, -31, -73, -62, -98, -43, -41, 12, -75, 60, 69, 54, -54,
    -40, 51, -73, -18, 36, 83, 86, -8, 80, -2, 105, -61, -122, 37, 114, 7, -7, 0, -1, -1,
    -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1,
    -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1,
    -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -15, -1, -1, 127, -1,
    -1, -1, -1, -112, 1, 0, 0, -88, 1, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 4, 8, 0, 0, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 48, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1,
    -1, -1, 0, -1, -1
};

static const char bmp4bit[] =
{
    73, 99, 111, 110, 32, 102, 105, 108, 101, 32, 118, 101,
    114, 115, 105, 111, 110, 32, 49, 46, 48, 46, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0,
    0, 1, 0, 0, 60, 0, 0, 0, -16, -1, -1, 127, 88, 1, 0, 0, -15, -1, -1, 127, -12, 1, 0, 0,
    0, 1, 0, 0, -1, -1, -1, -1, 88, 0, 0, 0, -40, 0, 0, 0, 0, 0, 0, 0, 16, 0, 16, 0, 4, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 51, 51, 51, 51, 51, 51, 48, 3, 0, 0, 0, 0, 0, 0, 48,
    3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0,
    0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48,
    3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0, 0, 0, 0, 48, 3, 0, 0, 0,
    0, 0, 0, 48, 3, 51, 51, 51, 51, 51, 51, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, -1, -1, -1, -1, -16, 0, 0,
    15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1, -1, -1, -16,
    0, 0, 15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1, -1, -1,
    -16, 0, 0, 15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1, -1, -1, -16, 0, 0, 15, -1, -1,
    -1, -1, -16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -16, -1, -1, 127, -1, -1, -1, -1, 116, 1, 0, 0, -76, 1, 0, 0, 0, 0, 0, 0, 1, 0, 64, 0,
    4, 8, 0, 0, 81, -3, -40, -70, -124, 24, -99, -19, 46, -36, -3, -21, -128, 98, 96, -75,
    82, -99, 78, 108, -79, 18, -9, -36, -20, -88, -41, 84, -99, 88, 98, 40, 74, 93, 118,
    -31, -73, -62, -98, -43, -41, 12, -75, 60, 69, 54, -54, -40, 51, -73, -18, 36, 83, 86,
    -8, 80, -2, 105, -61, -122, 37, 114, 7, -7, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1,
    -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1,
    -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1,
    -1, 0, -1, -1, -1, -1, -1, -1, 0, -15, -1, -1, 127, -1, -1, -1, -1, 16, 2, 0, 0, 40, 2,
    0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 4, 8, 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 97,
    98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 0, -1, -1, -1, -1, -1,
    -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1
};

static const char * cvgmap =
";Map Support File : Contains Meta Data Information about the Image\n"
"IMG_NAME = %name%\n"
"PRODUCT = %product%\n"
"PROVIDER = %provider%\n"
"IMG_DATE = %date%\n"
"IMG_VERSION = 31\n"
"Version = 31\n"
"BUILD =\n"
"VENDOR_ID = -1\n"
"REGION_ID = -1\n"
"MAP_TYPE = TNDB_RASTER_MAP\n"
"ADDITIONAL_COMMENTS = Created with map2rmp\n"
;

bool qSortInFiles(CFileGenerator::file_t& f1, CFileGenerator::file_t& f2)
{
    return f1.xscale < f2.xscale;
}

void CFileGenerator::file_t::convertPx2Deg(double& u, double& v)
{
    u = u*xscale  + lon1;
    v = v*yscale  + lat1;
}

void CFileGenerator::file_t::convertDeg2Px(double& u, double& v)
{
    u = (u - lon1) / xscale;
    v = (v - lat1) / yscale;
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

    QList<file_t> infiles;
    QVector<rmp_file_t> outfiles;

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

        file.xtiles = ceil(float(file.xsize)/TILE_SIZE);
        file.ytiles = ceil(float(file.ysize)/TILE_SIZE);

        if(file.xsize < TILE_SIZE || file.ysize < TILE_SIZE)
        {
            fprintf(stderr,"\nfile %s too small. Minimum size is %ix%i pixel.\n", filename.toLocal8Bit().data(),TILE_SIZE,TILE_SIZE);
            exit(-1);

        }

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

    int nBigTilesX  = ceil(float(file.xtiles)/N_TILES_X);
    int nBigTilesY  = ceil(float(file.ytiles)/N_TILES_Y);
    int nSectX      = ceil(float(nBigTilesX)/N_BIG_TILES_X);
    int nSectY      = ceil(float(nBigTilesY)/N_BIG_TILES_Y);

    fprintf(stdout, "x/y sect: %i %i\n", nSectX, nSectY);
    fprintf(stdout, "files: %i \n", nSectX * nSectY);

    outfiles.resize(nSectX * nSectY);

    for(int y = 0; y < nSectY; y++)
    {
        for(int x = 0; x < nSectX; x++)
        {
            const int index = x + y * nSectX;
            rmp_file_t& rmp = outfiles[index];
            rmp.index = index;

            setupOutFile(x, y, infiles, rmp);

            rmp.directory.resize(4 + rmp.levels.size() * 2);
            sprintf(rmp.directory[INDEX_BMP2BIT].name, "bmp2bit");
            sprintf(rmp.directory[INDEX_BMP2BIT].ext, "ics");
            sprintf(rmp.directory[INDEX_BMP4BIT].name, "bmp4bit");
            sprintf(rmp.directory[INDEX_BMP4BIT].ext, "ics");
            sprintf(rmp.directory[INDEX_RMPINI].name, "rmp");
            sprintf(rmp.directory[INDEX_RMPINI].ext, "ini");
            sprintf(rmp.directory[INDEX_CVGMAP].name, "cvg_map");
            sprintf(rmp.directory[INDEX_CVGMAP].ext, "msf");

            QFileInfo fi(rmp.name);
            QString name = fi.baseName().left(7);
            for(int i = 0; i < rmp.levels.size(); i++)
            {
                sprintf(rmp.directory[OFFSET_1ST_DIR + i * 2].name, "%s%i", name.toLocal8Bit().data(), i);
                sprintf(rmp.directory[OFFSET_1ST_DIR + i * 2].ext, "a00");
                sprintf(rmp.directory[OFFSET_2ND_DIR + i * 2].name, "%s%i", name.toLocal8Bit().data(), i);
                sprintf(rmp.directory[OFFSET_2ND_DIR + i * 2].ext, "tlm");
            }

        }
    }

    for(int i = 0; i < outfiles.size(); i++)
    {
        rmp_file_t& rmp = outfiles[i];
        writeRmp(rmp);
    }

    return 0;
}

void CFileGenerator::setupOutFile(int x, int y, QList<file_t>& infiles, rmp_file_t& rmp)
{
    QFileInfo fi(output);
    QDir dir(fi.absolutePath());

    rmp.name = dir.absoluteFilePath(fi.baseName() + QString("_%1").arg(rmp.index) + ".rmp");
    rmp.levels.resize(infiles.size());

    printf("\nsetup file: %s\n", rmp.name.toLocal8Bit().data());

    file_t& base = infiles[0];
    // 1 tile       256x256 px
    // 1 big tile   9x9 tiles
    // 1 level      9x9 big tiles
    double lon1 =  x        * N_BIG_TILES_X * N_TILES_X * TILE_SIZE;
    double lat1 =  y        * N_BIG_TILES_Y * N_TILES_Y * TILE_SIZE;
    double lon2 = (x + 1)   * N_BIG_TILES_X * N_TILES_X * TILE_SIZE;
    double lat2 = (y + 1)   * N_BIG_TILES_Y * N_TILES_Y * TILE_SIZE;

    if(lon2 > base.xsize) lon2 = base.xsize;
    if(lat2 > base.ysize) lat2 = base.ysize;

    //printf("base file area (px):  %i,%i (%ix%i)\n", int(lon1), int(lat1), int(lon2 - lon1), int(lat2 - lat1));
    base.convertPx2Deg(lon1, lat1);
    base.convertPx2Deg(lon2, lat2);
    //printf("base file area (deg): %f,%f to %f,%f)\n", lon1, lat1, lon2, lat2);

    for(int i = 0; i < infiles.size(); i++)
    {
        rmp_level_t& level = rmp.levels[i];
        level.src = &infiles[i];

        double u1 = lon1;
        double v1 = lat1;
        double u2 = lon2;
        double v2 = lat2;

        level.src->convertDeg2Px(u1,v1);
        level.src->convertDeg2Px(u2,v2);

        if(u1 < 0) u1 = 0;
        if(u2 < 0) u2 = 0;
        if(v1 < 0) v1 = 0;
        if(v2 < 0) v2 = 0;

        if(u1 > level.src->xsize) u1 = level.src->xsize;
        if(u2 > level.src->xsize) u2 = level.src->xsize;
        if(v1 > level.src->ysize) v1 = level.src->ysize;
        if(v2 > level.src->ysize) v2 = level.src->ysize;

        level.x1 = int(u1 + 0.5);
        level.y1 = int(v1 + 0.5);
        level.x2 = int(u2 + 0.5);
        level.y2 = int(v2 + 0.5);

        int w = level.x2 - level.x1;
        int h = level.y2 - level.y1;

        if(w < TILE_SIZE)
        {
            level.x1 = level.x1 - (TILE_SIZE - w);
        }

        if(h < TILE_SIZE)
        {
            level.y1 = level.y1 - (TILE_SIZE - h);
        }

        level.lon1 = level.x1;
        level.lat1 = level.y1;
        level.lon2 = level.x2;
        level.lat2 = level.y2;

        level.src->convertPx2Deg(level.lon1,level.lat1);
        level.src->convertPx2Deg(level.lon2,level.lat2);

        printf("level[%i] area (px):  %i,%i (%ix%i)\n", i, level.x1, int(level.y1), int(level.x2 - level.x1), int(level.y2 - level.y1));
//        printf("level[%i] area (deg): %f,%f to %f,%f)\n", i, level.lon1, level.lat1, level.lon2, level.lat2);

        int nBigTilesX = ceil(float(level.x2 - level.x1)/ (N_TILES_X * TILE_SIZE));
        int nBigTilesY = ceil(float(level.y2 - level.y1)/ (N_TILES_Y * TILE_SIZE));

        level.bigTiles.resize(nBigTilesX * nBigTilesY);
        for(int m = 0; m < nBigTilesY; m++)
        {
            for(int n = 0; n < nBigTilesX; n++)
            {
                const int index         = n + m * nBigTilesX;
                rmp_big_tile_t& bigTile = level.bigTiles[index];
                bigTile.src             = level.src;
                setupBigTile(n, m, level, bigTile);
            }
        }
    }

}

void CFileGenerator::setupBigTile(int x, int y, rmp_level_t& level, rmp_big_tile_t& bigTile)
{
    bigTile.x1 = level.x1 + x * N_TILES_X * TILE_SIZE;
    bigTile.y1 = level.y1 + y * N_TILES_Y * TILE_SIZE;

    bigTile.x2 = level.x1 + (x + 1) * N_TILES_X * TILE_SIZE;
    bigTile.y2 = level.y1 + (y + 1) * N_TILES_Y * TILE_SIZE;

    if(bigTile.x2 > level.x2) bigTile.x2 = level.x2;
    if(bigTile.y2 > level.y2) bigTile.y2 = level.y2;

    int w = bigTile.x2 - bigTile.x1;
    int h = bigTile.y2 - bigTile.y1;

    if(w < TILE_SIZE)
    {
        bigTile.x1 = bigTile.x1 - (TILE_SIZE - w);
    }

    if(h < TILE_SIZE)
    {
        bigTile.y1 = bigTile.y1 - (TILE_SIZE - h);
    }

    bigTile.lat1 = bigTile.x1;
    bigTile.lon1 = bigTile.y1;
    bigTile.lat2 = bigTile.x2;
    bigTile.lon2 = bigTile.y2;

    bigTile.src->convertPx2Deg(bigTile.lon1, bigTile.lat1);
    bigTile.src->convertPx2Deg(bigTile.lon2, bigTile.lat2);

//    printf("    big tile area (px):  %i,%i (%ix%i)\n", bigTile.x1, int(bigTile.y1), int(bigTile.x2 - bigTile.x1), int(bigTile.y2 - bigTile.y1));
//    printf("    big tile area (deg): %f,%f to %f,%f)\n", bigTile.lon1, bigTile.lat1, bigTile.lon2, bigTile.lat2);

    int nTilesX = ceil(float(bigTile.x2 - bigTile.x1)/TILE_SIZE);
    int nTilesY = ceil(float(bigTile.y2 - bigTile.y1)/TILE_SIZE);
    bigTile.tiles.resize(nTilesX * nTilesY);
    for(int m = 0; m < nTilesY; m++)
    {
        for(int n = 0; n < nTilesX; n++)
        {
            const int index = n + m * nTilesX;
            rmp_tile_t& tile  = bigTile.tiles[index];
            tile.src        = bigTile.src;
            setupTile(n, m, bigTile, tile);
        }
    }
}

void CFileGenerator::setupTile(int x, int y, rmp_big_tile_t &bigTile, rmp_tile_t &tile)
{
    tile.x1 = bigTile.x1 + x * TILE_SIZE;
    tile.y1 = bigTile.y1 + y * TILE_SIZE;

    tile.x2 = bigTile.x1 + (x + 1) * TILE_SIZE;
    tile.y2 = bigTile.y1 + (y + 1) * TILE_SIZE;

    if(tile.x2 > bigTile.x2) tile.x2 = bigTile.x2;
    if(tile.y2 > bigTile.y2) tile.y2 = bigTile.y2;

    int w = tile.x2 - tile.x1;
    int h = tile.y2 - tile.y1;

    if(w < TILE_SIZE)
    {
        tile.x1 = tile.x1 - (TILE_SIZE - w);
    }

    if(h < TILE_SIZE)
    {
        tile.y1 = tile.y1 - (TILE_SIZE - h);
    }

    tile.lat1 = tile.x1;
    tile.lon1 = tile.y1;
    tile.lat2 = tile.x2;
    tile.lon2 = tile.y2;

    bigTile.src->convertPx2Deg(tile.lon1, tile.lat1);
    bigTile.src->convertPx2Deg(tile.lon2, tile.lat2);

//    printf("      tile area (px):  %i,%i (%ix%i)\n", tile.x1, int(tile.y1), int(tile.x2 - tile.x1), int(tile.y2 - tile.y1));
//    printf("      tile area (deg): %f,%f to %f,%f)\n", tile.lon1, tile.lat1, tile.lon2, tile.lat2);

}

quint16 CFileGenerator::crc16(QDataStream& stream, qint32 length)
{
    bool evenByte   = true;
    quint16 crc     = 0;
    quint8 tmp8;

    for(int i = 0; i < length; i++)
    {
        stream >> tmp8;
        if(evenByte)
        {
            crc ^= tmp8 << 8;
        }
        else
        {
            crc ^= tmp8;
        }
        evenByte = !evenByte;
    }

    return crc;
}

void CFileGenerator::writeRmp(rmp_file_t& rmp)
{
    QFile file(rmp.name);
    if(!file.open(QIODevice::ReadWrite))
    {
        fprintf(stderr,"\nFailed to open %s\n", rmp.name.toLocal8Bit().data());
        exit(-1);
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    // 1st run for directory as place holder
    writeDirectory(stream, rmp);
    // write all data listed in directory
    writeBmp2Bit(stream, rmp);
    writeBmp4Bit(stream, rmp);
    writeCvgMap(stream,rmp);
    writeRmpIni(stream,rmp);

    // 2nd run to write real directory
    stream.device()->seek(0);
    writeDirectory(stream, rmp);
}

void CFileGenerator::writeDirectory(QDataStream& stream, rmp_file_t& rmp)
{
    char magic[30] = "MAGELLAN";

    stream.device()->seek(0);
    stream << quint32(rmp.directory.size()) << quint32(rmp.directory.size());
    foreach(const rmp_dir_entry_t& entry, rmp.directory)
    {
        stream.writeRawData(entry.name,sizeof(entry.name));
        stream.writeRawData(entry.ext,sizeof(entry.ext));
        stream << entry.offset << entry.length;
    }

    qint32 length   = stream.device()->pos();
    stream.device()->seek(0);
    quint16 crc     = crc16(stream, length);

    stream << crc;
    stream.writeRawData(magic, sizeof(magic));
}

void CFileGenerator::writeBmp2Bit(QDataStream& stream, rmp_file_t& rmp)
{
    rmp.directory[INDEX_BMP2BIT].offset = stream.device()->pos();
    rmp.directory[INDEX_BMP2BIT].length = sizeof(bmp2bit);
    stream.writeRawData(bmp2bit, sizeof(bmp2bit));
}

void CFileGenerator::writeBmp4Bit(QDataStream& stream, rmp_file_t& rmp)
{
    rmp.directory[INDEX_BMP4BIT].offset = stream.device()->pos();
    rmp.directory[INDEX_BMP4BIT].length = sizeof(bmp4bit);
    stream.writeRawData(bmp4bit, sizeof(bmp4bit));
}

void CFileGenerator::writeCvgMap(QDataStream& stream, rmp_file_t& rmp)
{
    QString cvg(cvgmap);
    cvg.replace("%name%",QFileInfo(rmp.name).baseName());
    cvg.replace("%product%","aaa");
    cvg.replace("%provider%","bbb");
    cvg.replace("%date%", QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
    cvg.replace("\n", "\015\012");

    rmp.directory[INDEX_CVGMAP].offset = stream.device()->pos();
    rmp.directory[INDEX_CVGMAP].length = cvg.size();
    stream.writeRawData(cvg.toLocal8Bit(), cvg.size());
}

void CFileGenerator::writeRmpIni(QDataStream& stream, rmp_file_t& rmp)
{
    QString ini("[T_Layers]\n");
    for(int i = 0; i < rmp.levels.size(); i++)
    {
        ini += QString("%1=%2\n").arg(i).arg(rmp.directory[OFFSET_1ST_DIR + i * 2].name);
    }
    ini.replace("\n", "\015\012");

    rmp.directory[INDEX_RMPINI].offset = stream.device()->pos();
    rmp.directory[INDEX_RMPINI].length = ini.size();
    stream.writeRawData(ini.toLocal8Bit(), ini.size());
}
