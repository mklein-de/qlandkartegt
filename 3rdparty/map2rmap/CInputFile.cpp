#include "CInputFile.h"
#include "main.h"

#include <QtCore>
#include <stdio.h>
#include <ogr_spatialref.h>

extern "C"
{
    #include <jpeglib.h>
}

#define JPG_BLOCK_SIZE (TILESIZE*TILESIZE)

static std::vector<JOCTET> jpgbuf;
static void init_destination (j_compress_ptr cinfo)
{
    jpgbuf.resize(JPG_BLOCK_SIZE);
    cinfo->dest->next_output_byte   = &jpgbuf[0];
    cinfo->dest->free_in_buffer     = jpgbuf.size();
}

static boolean empty_output_buffer (j_compress_ptr cinfo)
{
    size_t oldsize = jpgbuf.size();
    jpgbuf.resize(oldsize + JPG_BLOCK_SIZE);
    cinfo->dest->next_output_byte   = &jpgbuf[oldsize];
    cinfo->dest->free_in_buffer     = jpgbuf.size() - oldsize;
    return true;
}
static void term_destination (j_compress_ptr cinfo)
{
    jpgbuf.resize(jpgbuf.size() - cinfo->dest->free_in_buffer);
}


quint32 CInputFile::nTilesTotal     = 0;
quint32 CInputFile::nTilesProcessed = 0;

CInputFile::CInputFile(const QString &filename, quint32 tileSize)
    : filename(filename)
    , tileSize(tileSize)
    , nTiles(0)
    , tileBuf08Bit(tileSize * tileSize,0)
    , tileBuf24Bit(tileSize * tileSize * 3,0)
    , tileBuf32Bit(tileSize * tileSize * 4,0)
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
    OGRSpatialReference oSRS_EPSG4326;
    oSRS_EPSG4326.importFromProj4("+init=epsg:4326");


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

//    qDebug();
//    qDebug() << filename;
//    qDebug() << ptr;

    if(oSRS.IsSame(&oSRS_EPSG31467))
    {
        compeProj   = "117,GK-System 9º (Zone 3),";
        compeDatum  = "WGS 84";
    }
    else if(oSRS.IsSame(&oSRS_EPSG31468))
    {
        compeProj   = "114,GK-System 12º (Zone 4),";
        compeDatum  = "WGS 84";
    }
    else if(oSRS.IsSame(&oSRS_EPSG4326))
    {
        compeProj   = "2,Mercator,";
        compeDatum  = "WGS 84";
    }
    else
    {
        fprintf(stderr,"\n%s\nprojection in file %s not recognized\n", ptr, filename.toLocal8Bit().data());
        exit(-1);
    }

//    qDebug() << compeProj + compeDatum;
    dataset->GetGeoTransform( adfGeoTransform );

    width   = dataset->GetRasterXSize();
    height  = dataset->GetRasterYSize();

    if(pj_is_latlong(pj))
    {
        xscale  = adfGeoTransform[1] * DEG_TO_RAD;
        yscale  = adfGeoTransform[5] * DEG_TO_RAD;
        xref1   = adfGeoTransform[0] * DEG_TO_RAD;
        yref1   = adfGeoTransform[3] * DEG_TO_RAD;
    }
    else
    {
        xscale  = adfGeoTransform[1];
        yscale  = adfGeoTransform[5];
        xref1   = adfGeoTransform[0];
        yref1   = adfGeoTransform[3];
    }
    xref2   = xref1 + width  * xscale;
    yref2   = yref1 + height * yscale;
}

CInputFile::~CInputFile()
{

}

void CInputFile::summarize()
{
    printf("\n\n--- %s ---", QFileInfo(filename).fileName().toLocal8Bit().data());
    for(int i = 0; i < levels.count(); i++)
    {
        level_t& level = levels[i];
        printf("\nLevel%i:", i);
        printf("\nwidth/height:  %i/%i [pixel]", level.width, level.height);
        if(pj_is_latlong(pj))
        {
            printf("\nxscale/yscale: %1.6f/%1.6f [°/pixel]", level.xscale * RAD_TO_DEG, level.yscale * RAD_TO_DEG);
        }
        else
        {
            printf("\nxscale/yscale: %1.6f/%1.6f [m/pixel]", level.xscale, level.yscale);
        }

        printf("\nTiles X/Y:     %i/%i", level.xTiles, level.yTiles);
        printf("\n");
    }

}

void CInputFile::getRef1Deg(double& lon, double& lat)
{
    PJ * wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    lon = xref1;
    lat = yref1;

    pj_transform(pj, wgs84, 1, 0, &lon, &lat, 0);

    pj_free(wgs84);

    lon *= RAD_TO_DEG;
    lat *= RAD_TO_DEG;
}

void CInputFile::getRef2Deg(double& lon, double& lat)
{
    PJ * wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    lon = xref2;
    lat = yref2;

    pj_transform(pj, wgs84, 1, 0, &lon, &lat, 0);

    pj_free(wgs84);

    lon *= RAD_TO_DEG;
    lat *= RAD_TO_DEG;
}



quint32 CInputFile::calcLevels(double scaleLimit)
{
    nTiles     = 0;
    qint32 nLevels  = 1;

    if(scaleLimit)
    {
        double s = xscale;
        while(s*2.1 < scaleLimit)
        {
            s = s * 2;
            nLevels++;
        }
    }
    else
    {
        quint32 w = width;
        while(w>>1 > TILESIZE)
        {
            w = w >> 1;
            nLevels++;
        }
    }

    for(int l = 0; l < nLevels; l++)
    {
        level_t level;

        level.xscale = xscale * (1 << l);
        level.yscale = yscale * (1 << l);
        level.width  = width  >> l;
        level.height = height >> l;
        level.xTiles = ceil(double(level.width) / tileSize);
        level.yTiles = ceil(double(level.height) / tileSize);

        levels << level;

        nTiles += level.xTiles * level.yTiles;

//        qDebug() << "level" << l << level.width << level.height << level.xTiles << level.yTiles << level.xscale << level.yscale;
    }

    nTilesTotal += nTiles;

    return levels.count();
}

void CInputFile::writeLevels(QDataStream& stream, double& scale, int quality, int subsampling)
{
    for(int i = 0; i < levels.count(); i++)
    {
        writeLevel(stream, i, scale, quality, subsampling);
    }

    scale = scale * (1 << levels.count());

}

void CInputFile::writeLevelOffsets(QDataStream& stream)
{
    for(int i = 0; i < levels.count(); i++)
    {
        stream << levels[i].offsetLevel;
    }

}

void CInputFile::writeLevel(QDataStream& stream, int l, double& scale, int quality, int subsampling)
{
    level_t& level = levels[l];


    level.width  = level.width * xscale / scale;
    level.height = level.height * xscale / scale;
    level.xTiles = ceil(double(level.width) / tileSize);
    level.yTiles = ceil(double(level.height) / tileSize);

    for(int y = 0; y < level.yTiles; y++)
    {
        for(int x = 0; x < level.xTiles; x++)
        {

            qint32 xoff = x * (tileSize << l) * scale / xscale;
            qint32 yoff = y * (tileSize << l) * scale / xscale;
            qint32 w1   = (tileSize << l) * scale / xscale;
            qint32 h1   = (tileSize << l) * scale / xscale;
            qint32 w2   = tileSize;
            qint32 h2   = tileSize;

            if((xoff + w1) > width)
            {
                w2 = w2 * (width - xoff) / w1;
                w1 = width - xoff;
            }
            if((yoff + h1) > height)
            {
                h2 = h2 * (height - yoff) / h1;
                h1 = height - yoff;
            }

            if(readTile(xoff, yoff, w1, h1, w2, h2, (quint32*)tileBuf32Bit.data()))
            {
                quint32 size = writeTile(w2, h2, (quint32*)tileBuf32Bit.data(), quality, subsampling);

                level.offsetJpegs << stream.device()->pos();
                stream << quint32(7) << size;
                stream.writeRawData((const char*)&jpgbuf[0], size);
            }
            else
            {
                fprintf(stderr, "\nFailed to read tile from source\n");
                exit(-1);
            }

            nTilesProcessed++;
            printProgress(nTilesProcessed, nTilesTotal);
        }
    }


    level.offsetLevel = stream.device()->pos();
    stream << level.width << -level.height;
    stream << level.xTiles << level.yTiles;
    for(int i = 0; i < level.offsetJpegs.count(); i++)
    {
        stream << level.offsetJpegs[i];
    }
}

bool CInputFile::readTile(qint32 xoff, qint32 yoff, qint32 w1, qint32 h1, qint32 w2, qint32 h2, quint32 *output)
{
    qint32 rasterBandCount = dataset->GetRasterCount();

    memset(output,-1, sizeof(quint32) * w2 * h2);

    if(rasterBandCount == 1)
    {
        GDALRasterBand * pBand;
        pBand = dataset->GetRasterBand(1);
        if(pBand->RasterIO(GF_Read, (int)xoff, (int)yoff, w1, h1, tileBuf08Bit.data(), w2, h2, GDT_Byte,0,0) == CE_Failure)
        {
            return false;
        }

        for(unsigned int i = 0; i < (w2 * h2); i++)
        {
            output[i] = colortable[tileBuf08Bit[i]];
        }
    }
    else
    {
        for(int b = 1; b <= rasterBandCount; ++b)
        {
            GDALRasterBand * pBand;
            pBand = dataset->GetRasterBand(b);

            quint32 mask = ~(0x000000FF << (8*(b-1)));

            if(pBand->RasterIO(GF_Read,(int)xoff,(int)yoff, w1, h1, tileBuf08Bit.data(),w2,h2,GDT_Byte,0,0) == CE_Failure)
            {
                return false;
            }

            for(unsigned int i = 0; i < (w2 * h2); i++)
            {
                quint32 pixel = output[i];

                pixel &= mask;
                pixel |= tileBuf08Bit[i] << (8*(b-1));
                output[i] = pixel;
            }
        }
    }

    return true;
}

quint32 CInputFile::writeTile(quint32 xsize, quint32 ysize, quint32 * raw_image, int quality, int subsampling)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];

    jpeg_destination_mgr destmgr    = {0};
    destmgr.init_destination        = init_destination;
    destmgr.empty_output_buffer     = empty_output_buffer;
    destmgr.term_destination        = term_destination;

    // convert from RGBA to RGB
    for(quint32 r = 0; r < ysize; r++)
    {
        for(quint32 c = 0; c < xsize; c++)
        {
            quint32 pixel = raw_image[r * xsize + c];
            tileBuf24Bit[r * xsize * 3 + c * 3]     =  pixel        & 0x0FF;
            tileBuf24Bit[r * xsize * 3 + c * 3 + 1] = (pixel >>  8) & 0x0FF;
            tileBuf24Bit[r * xsize * 3 + c * 3 + 2] = (pixel >> 16) & 0x0FF;
        }
    }

    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress(&cinfo);

    cinfo.dest              = &destmgr;
    cinfo.image_width       = xsize;
    cinfo.image_height      = ysize;
    cinfo.input_components  = 3;
    cinfo.in_color_space    = JCS_RGB;

    jpeg_set_defaults( &cinfo );

    if (subsampling != -1)
    {
        switch (subsampling)
        {
        case 422:  // 2x1, 1x1, 1x1 (4:2:2) : Medium
            {
                cinfo.comp_info[0].h_samp_factor = 2;
                cinfo.comp_info[0].v_samp_factor = 1;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }
        case 411:  // 2x2, 1x1, 1x1 (4:1:1) : High
            {
                cinfo.comp_info[0].h_samp_factor = 2;
                cinfo.comp_info[0].v_samp_factor = 2;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }
        case 444:  // 1x1 1x1 1x1 (4:4:4) : None
            {
                cinfo.comp_info[0].h_samp_factor = 1;
                cinfo.comp_info[0].v_samp_factor = 1;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }
        }
    }

    if (quality != -1)
    {
        jpeg_set_quality( &cinfo, quality, TRUE );
    }

    jpeg_start_compress( &cinfo, TRUE );

    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = (JSAMPLE*)&tileBuf24Bit.data()[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
    /* similar to read file, clean up after we're done compressing */
    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );

    return jpgbuf.size();
}

