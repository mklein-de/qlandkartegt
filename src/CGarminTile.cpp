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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/
#include "CGarminTile.h"
#include "CGarminStrTbl8.h"
#include "CGarminStrTbl6.h"
#include "CGarminStrTblUtf8.h"
#include "Platform.h"
#include "IMap.h"
#include "CMapDB.h"

#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <assert.h>

#undef DEBUG_SHOW_SECT_DESC
#undef DEBUG_SHOW_TRE_DATA
#undef DEBUG_SHOW_MAPLEVEL_DATA
#undef DEBUG_SHOW_SUBDIV_DATA
#undef DEBUG_SHOW_POLY_DATA

#undef DEBUG_SHOW_SECTION_BORDERS

CGarminTile::CGarminTile(IMap * parent)
: QObject(parent)
, transparent(false)
{
}


CGarminTile::~CGarminTile()
{
}


void CGarminTile::readFile(QFile& file, quint32 offset, quint32 size, QByteArray& data)
{
    file.seek(offset);
    data = file.read(size);

    if((quint32)data.size() != size) {
        throw exce_t(eErrOpen, tr("Failed to read: ") + filename);
    }

    quint8 * p = (quint8*)data.data();
    for(quint32 i = 0; i < size; ++i) {
        *p++ ^= mask;
    }

}


void CGarminTile::readBasics(const QString& fn)
{
    char tmpstr[64];

    filename        = fn;
    qint64 fsize    = QFileInfo(fn).size();

    //     qDebug() << "++++" << filename << "++++";
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        throw exce_t(eErrOpen, tr("Failed to open: ") + filename);
    }
    file.read((char*)&mask, 1);

    //     {
    //         QByteArray data;
    //         readFile(file,0,file.size(),data);
    //
    //         QFile f(filename + ".dbg");
    //         f.open(QIODevice::WriteOnly);
    //         f.write(data);
    //         f.close();
    //     }

    // read hdr_img_t
    QByteArray imghdr;
    readFile(file, 0, sizeof(hdr_img_t), imghdr);
    hdr_img_t * pImgHdr = (hdr_img_t*)imghdr.data();

    if(strncmp(pImgHdr->signature,"DSKIMG",7) != 0) {
        throw exce_t(errFormat,tr("Bad file format: ") + filename);
    }
    if(strncmp(pImgHdr->identifier,"GARMIN",7) != 0) {
        throw exce_t(errFormat,tr("Bad file format: ") + filename);
    }

    mapdesc  = QByteArray((const char*)pImgHdr->desc1,20);
    mapdesc += pImgHdr->desc2;
    //     qDebug() << mapdesc;

    size_t blocksize = pImgHdr->blocksize();

    // 1st read FAT
    QByteArray FATblock;
    readFile(file, sizeof(hdr_img_t), sizeof(FATblock_t), FATblock);
    const FATblock_t * pFATBlock = (const FATblock_t * )FATblock.data();

    size_t dataoffset = sizeof(hdr_img_t);

    // skip dummy blocks at the beginning
    while(dataoffset < (size_t)fsize) {
        if(pFATBlock->flag != 0x00) {
            break;
        }
        dataoffset += sizeof(FATblock_t);
        readFile(file, dataoffset, sizeof(FATblock_t), FATblock);
        pFATBlock = (const FATblock_t * )FATblock.data();
    }

    // start of new subfile part
    /*
        It is taken for granted that the single subfile parts are not
        fragmented within the file. Thus it is not really neccessary to
        store and handle all block sequence numbers. Just the first one
        will give us the offset. This also implies that it is not necessary
        to care about FAT blocks with a non-zero part number.

        2007-03-31: Garmin's world base map seems to be coded different.
                    The part field seems to be rather a bit field than
                    a part number. As the total subfile size is given
                    for the first part only (for all otheres it's zero)
                    I use it to identify the 1st part of a subfile

        2007-05-26: Gmapsupp images by Sendmap code quite some bull shit,
                    too. The size is stored in every part and they do have
                    a part number. I introduced a set of subfile names
                    storing the subfile's name and type. The first part
                    with a size info and it's name / type not stored in the
                    set is used to get the location information.
    */
    QSet<QString> subfileNames;
    while(dataoffset < (size_t)fsize) {
        if(pFATBlock->flag != 0x01) {
            break;
        }

        memcpy(tmpstr,pFATBlock->name,sizeof(pFATBlock->name) + sizeof(pFATBlock->type));
        tmpstr[sizeof(pFATBlock->name) + sizeof(pFATBlock->type)] = 0;

        if(gar_load(uint32_t, pFATBlock->size) != 0 && !subfileNames.contains(tmpstr) && tmpstr[0] != 0x20) {
            subfileNames << tmpstr;

            memcpy(tmpstr,pFATBlock->name,sizeof(pFATBlock->name));
            tmpstr[sizeof(pFATBlock->name)] = 0;

            // skip MAPSORC.MPS section
            if(strcmp(tmpstr,"MAPSOURC") && strcmp(tmpstr,"SENDMAP2")) {

                subfile_desc_t& subfile = subfiles[tmpstr];
                subfile.name = tmpstr;

                memcpy(tmpstr,pFATBlock->type,sizeof(pFATBlock->type));
                tmpstr[sizeof(pFATBlock->type)] = 0;

                subfile_part_t& part = subfile.parts[tmpstr];
                part.size   = gar_load(uint32_t, pFATBlock->size);
                part.offset = gar_load(uint16_t, pFATBlock->blocks[0]) * blocksize;
            }

        }

        dataoffset += sizeof(FATblock_t);
        readFile(file, dataoffset, sizeof(FATblock_t), FATblock);
        pFATBlock = (const FATblock_t * )FATblock.data();
    }

    if((dataoffset == sizeof(hdr_img_t)) || (dataoffset >= (size_t)fsize)) {
        throw exce_t(errFormat,tr("Failed to read file structure: ") + filename);
    }

    // gmapsupp.img files do not have a data offset field
    if(gar_load(uint32_t, pImgHdr->dataoffset) == 0) {
        pImgHdr->dataoffset = gar_load(uint32_t, dataoffset);
    }

    // sometimes there are dummy blocks at the end of the FAT
    if(gar_load(uint32_t, pImgHdr->dataoffset) != dataoffset) {
        dataoffset = gar_load(uint32_t, pImgHdr->dataoffset);
    }

#ifdef DEBUG_SHOW_SECT_DESC
    {
        QMap<QString,subfile_desc_t>::const_iterator subfile = subfiles.begin();
        while(subfile != subfiles.end()) {
            //             qDebug() << "--- subfile" << subfile->name << "---";
            QMap<QString,subfile_part_t>::const_iterator part = subfile->parts.begin();
            while(part != subfile->parts.end()) {
                //                 qDebug() << part.key() << hex << part->offset << part->size;
                ++part;
            }
            ++subfile;
        }
    }
#endif                       //DEBUG_SHOW_SECT_DESC

    QMap<QString,subfile_desc_t>::iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()) {
        subfile->name = mapdesc.trimmed();

        if((*subfile).parts.contains("GMP")) throw exce_t(errFormat,tr("File is NT format. Unable to read: ") + filename);

        readSubfileBasics(*subfile, file);
        ++subfile;
    }
}


static quint32 rgnoff = 0;
void CGarminTile::readSubfileBasics(subfile_desc_t& subfile, QFile& file)
{
    quint32 i;
    // test for mandatory subfile parts
    if(!(subfile.parts.contains("TRE") && subfile.parts.contains("RGN"))) return;

    //     qDebug() << "++++" << file.fileName() << "++++";

    void (*minno)(hdr_tre_t*,QByteArray&) = 0;
    minno = (void (*)(hdr_tre_t*,QByteArray&))QLibrary::resolve(QDir::home().filePath(".config/QLandkarteGT/mellon.so"),"minno");

    QByteArray trehdr;
    readFile(file, subfile.parts["TRE"].offset, sizeof(hdr_tre_t), trehdr);
    hdr_tre_t * pTreHdr = (hdr_tre_t * )trehdr.data();

    subfile.isTransparent   = pTreHdr->POI_flags & 0x0002;
    transparent             = subfile.isTransparent ? true : transparent;

#ifdef DEBUG_SHOW_TRE_DATA
    qDebug() << "+++" << subfile.name << "+++";
    qDebug() << "TRE header length  :" << gar_load(uint16_t, pTreHdr->length);
    qDebug() << "TRE1 offset        :" << hex << gar_load(uint32_t, pTreHdr->tre1_offset);
    qDebug() << "TRE1 size          :" << dec << gar_load(uint32_t, pTreHdr->tre1_size);
    qDebug() << "TRE2 offset        :" << hex << gar_load(uint32_t, pTreHdr->tre2_offset);
    qDebug() << "TRE2 size          :" << dec << gar_load(uint32_t, pTreHdr->tre2_size);
#endif                       // DEBUG_SHOW_TRE_DATA

    // read map boundaries from header
    qint32 i32;
    i32 = gar_ptr_load(int24_t, pTreHdr->northbound);
    subfile.north = GARMIN_RAD(i32);
    i32 = gar_ptr_load(int24_t, pTreHdr->eastbound);
    subfile.east = GARMIN_RAD(i32);
    i32 = gar_ptr_load(int24_t, pTreHdr->southbound);
    subfile.south = GARMIN_RAD(i32);
    i32 = gar_ptr_load(int24_t, pTreHdr->westbound);
    subfile.west = GARMIN_RAD(i32);

    if(subfile.east == subfile.west) {
        subfile.east = -subfile.east;
    }

    subfile.area = QRectF(QPointF(subfile.west, subfile.north), QPointF(subfile.east, subfile.south));

#ifdef DEBUG_SHOW_TRE_DATA
    qDebug() << "bounding area (rad)" << subfile.north << subfile.east << subfile.south << subfile.west;
    qDebug() << "bounding area (\260)" << subfile.area;
#endif                       // DEBUG_SHOW_TRE_DATA

    QByteArray maplevel;
    readFile(file, subfile.parts["TRE"].offset + gar_load(uint32_t, pTreHdr->tre1_offset), gar_load(uint32_t, pTreHdr->tre1_size), maplevel);
    const tre_map_level_t * pMapLevel = (const tre_map_level_t * )maplevel.data();

    if(minno) minno(pTreHdr, maplevel);

    if(pTreHdr->flag & 0x80) {
        throw exce_t(errLock,tr("File contains locked / encypted data. Garmin does not "
            "want you to use this file with any other software than "
            "the one supplied by Garmin."));
    }

    quint32 nlevels             = gar_load(uint32_t, pTreHdr->tre1_size) / sizeof(tre_map_level_t);
    quint32 nsubdivs            = 0;
    quint32 nsubdivs_last       = 0;
    // count subsections
    for(i=0; i<nlevels; ++i) {
        maplevel_t ml;
        ml.inherited    = TRE_MAP_INHER(pMapLevel);
        ml.level        = TRE_MAP_LEVEL(pMapLevel);
        ml.bits         = pMapLevel->bits;
        subfile.maplevels << ml;
        nsubdivs       += gar_load(uint16_t, pMapLevel->nsubdiv);
        nsubdivs_last   = gar_load(uint16_t, pMapLevel->nsubdiv);
#ifdef DEBUG_SHOW_MAPLEVEL_DATA
        qDebug() << "level" << TRE_MAP_LEVEL(pMapLevel) << "inherited" << TRE_MAP_INHER(pMapLevel)
            << "bits" << pMapLevel->bits << "#subdivs" << gar_load(uint16_t, pMapLevel->nsubdiv);
#endif                   // DEBUG_SHOW_MAPLEVEL_DATA
        ++pMapLevel;
    }

    quint32 nsubdivs_next = nsubdivs - nsubdivs_last;

    //////////////////////////////////
    // read subdivision information
    //////////////////////////////////
    // point to first map level definition
    pMapLevel = (const tre_map_level_t * )maplevel.data();
    // number of subdivisions per map level
    quint32 nsubdiv = gar_load(uint16_t, pMapLevel->nsubdiv);

    // point to first 16 byte subdivision definition entry
    QByteArray subdiv_n;
    readFile(file, subfile.parts["TRE"].offset + gar_load(uint32_t, pTreHdr->tre2_offset), gar_load(uint32_t, pTreHdr->tre2_size), subdiv_n);
    tre_subdiv_next_t * pSubDivN = (tre_subdiv_next_t*)subdiv_n.data();

    QVector<subdiv_desc_t> subdivs;
    subdivs.resize(nsubdivs);
    QVector<subdiv_desc_t>::iterator subdiv      = subdivs.begin();
    QVector<subdiv_desc_t>::iterator subdiv_prev = subdivs.end();

    // absolute offset of RGN data
    QByteArray rgnhdr;
    readFile(file, subfile.parts["RGN"].offset, sizeof(hdr_rgn_t), rgnhdr);
    hdr_rgn_t * pRgnHdr = (hdr_rgn_t*)rgnhdr.data();
    quint32 rgnoff = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->offset);

    quint32 rgnOffPolyg2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->offset_polyg2);
    quint32 rgnOffPolyl2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->offset_polyl2);
    quint32 rgnOffPoint2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->offset_point2);

    quint32 rgnLenPolyg2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->length_polyg2);
    quint32 rgnLenPolyl2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->length_polyl2);
    quint32 rgnLenPoint2 = /*subfile.parts["RGN"].offset +*/ gar_load(uint32_t, pRgnHdr->length_point2);

    //     qDebug() << "***" << hex << subfile.parts["RGN"].offset << (subfile.parts["RGN"].offset + subfile.parts["RGN"].size);
    //     qDebug() << "+++" << hex << rgnOffPolyg2 << (rgnOffPolyg2 + rgnLenPolyg2);
    //     qDebug() << "+++" << hex << rgnOffPolyl2 << (rgnOffPolyl2 + rgnLenPolyl2);
    //     qDebug() << "+++" << hex << rgnOffPoint2 << (rgnOffPoint2 + rgnLenPoint2);

    // parse all 16 byte subdivision entries
    for(i=0; i<nsubdivs_next; ++i, --nsubdiv) {
        qint32 cx,cy;
        qint32 width, height;

        subdiv->n = i;
        subdiv->next         = gar_load(uint16_t, pSubDivN->next);
        subdiv->terminate    = TRE_SUBDIV_TERM(pSubDivN);
        subdiv->rgn_start    = gar_ptr_load(uint24_t, pSubDivN->rgn_offset);
        subdiv->rgn_start   += rgnoff;
        // skip if this is the first entry
        if(subdiv_prev != subdivs.end()) {
            subdiv_prev->rgn_end = subdiv->rgn_start;
        }

        subdiv->hasPoints    = pSubDivN->elements & 0x10;
        subdiv->hasIdxPoints = pSubDivN->elements & 0x20;
        subdiv->hasPolylines = pSubDivN->elements & 0x40;
        subdiv->hasPolygons  = pSubDivN->elements & 0x80;

        // if all subdivisions of this level have been parsed, switch to the next one
        if(nsubdiv == 0) {
            ++pMapLevel;
            nsubdiv = gar_load(uint16_t, pMapLevel->nsubdiv);
        }

        subdiv->level = TRE_MAP_LEVEL(pMapLevel);
        subdiv->shift = 24 - pMapLevel->bits;

        cx = gar_ptr_load(uint24_t, pSubDivN->center_lng);
        subdiv->iCenterLng = cx;
        cy = gar_ptr_load(uint24_t, pSubDivN->center_lat);
        subdiv->iCenterLat = cy;
        width   = TRE_SUBDIV_WIDTH(pSubDivN) << subdiv->shift;
        height  = gar_load(uint16_t, pSubDivN->height) << subdiv->shift;

        subdiv->north = GARMIN_RAD(cy + height + 1);
        subdiv->south = GARMIN_RAD(cy - height);
        subdiv->east  = GARMIN_RAD(cx + width + 1);
        subdiv->west  = GARMIN_RAD(cx - width);

        subdiv->area = QRectF(QPointF(subdiv->west, subdiv->north), QPointF(subdiv->east, subdiv->south));

        subdiv->offsetPoints2       = 0;
        subdiv->lengthPoints2       = 0;
        subdiv->offsetPolylines2    = 0;
        subdiv->lengthPolylines2    = 0;
        subdiv->offsetPolygons2     = 0;
        subdiv->lengthPolygons2     = 0;

        subdiv_prev = subdiv;
        ++pSubDivN; ++subdiv;
    }

    // switch to last map level
    ++pMapLevel;
    // witch pointer to 14 byte subdivision sections
    tre_subdiv_t* pSubDivL = pSubDivN;
    // parse all 14 byte subdivision entries of last map level
    for(; i<nsubdivs; ++i) {
        qint32 cx,cy;
        qint32 width, height;
        subdiv->n = i;
        subdiv->next         = 0;
        subdiv->terminate    = TRE_SUBDIV_TERM(pSubDivL);
        subdiv->rgn_start    = gar_ptr_load(uint24_t, pSubDivL->rgn_offset);
        subdiv->rgn_start   += rgnoff;
        subdiv_prev->rgn_end = subdiv->rgn_start;
        subdiv->hasPoints    = pSubDivL->elements & 0x10;
        subdiv->hasIdxPoints = pSubDivL->elements & 0x20;
        subdiv->hasPolylines = pSubDivL->elements & 0x40;
        subdiv->hasPolygons  = pSubDivL->elements & 0x80;

        subdiv->level = TRE_MAP_LEVEL(pMapLevel);
        subdiv->shift = 24 - pMapLevel->bits;

        cx = gar_ptr_load(uint24_t, pSubDivL->center_lng);
        subdiv->iCenterLng = cx;
        cy = gar_ptr_load(uint24_t, pSubDivL->center_lat);
        subdiv->iCenterLat = cy;
        width   = TRE_SUBDIV_WIDTH(pSubDivL) << subdiv->shift;
        height  = gar_load(uint16_t, pSubDivL->height) << subdiv->shift;

        subdiv->north = GARMIN_RAD(cy + height + 1);
        subdiv->south = GARMIN_RAD(cy - height);
        subdiv->east  = GARMIN_RAD(cx + width + 1);
        subdiv->west  = GARMIN_RAD(cx - width);

        subdiv->area = QRectF(QPointF(subdiv->west, subdiv->north), QPointF(subdiv->east, subdiv->south));

        subdiv->offsetPoints2       = 0;
        subdiv->lengthPoints2       = 0;
        subdiv->offsetPolylines2    = 0;
        subdiv->lengthPolylines2    = 0;
        subdiv->offsetPolygons2     = 0;
        subdiv->lengthPolygons2     = 0;

        subdiv_prev = subdiv;
        ++pSubDivL; ++subdiv;
    }
    subdivs.last().rgn_end = pRgnHdr->hdr_rgn_t::length;

    // read extended NT elements

//     qDebug() << "yyy" << gar_load(uint32_t, pTreHdr->tre7_rec_size);
    if((pTreHdr->hdr_subfile_part_t::length >= 0x9A) && pTreHdr->tre7_size && (gar_load(uint32_t, pTreHdr->tre7_rec_size) >= sizeof(tre_subdiv2_t))) {

        rgnoff = subfile.parts["RGN"].offset;
        //         qDebug() << subdivs.count() << (pTreHdr->tre7_size / pTreHdr->tre7_rec_size) << pTreHdr->tre7_rec_size;
        QByteArray subdiv2;
        readFile(file, subfile.parts["TRE"].offset + gar_load(uint32_t, pTreHdr->tre7_offset), gar_load(uint32_t, pTreHdr->tre7_size), subdiv2);
        tre_subdiv2_t * pSubDiv2    = (tre_subdiv2_t*)subdiv2.data();

        const quint32 entries1 = gar_load(uint32_t, pTreHdr->tre7_size) / gar_load(uint32_t, pTreHdr->tre7_rec_size);
        const quint32 entries2 = subdivs.size();

        bool skipPois = true;
        if(gar_load(uint32_t, pTreHdr->tre7_rec_size) == sizeof(tre_subdiv2_t)) skipPois = false;

//         for(int i = 0; i < pTreHdr->tre7_rec_size; ++i){
//             if(i%4 == 0) fprintf(stderr,"\n");
//             fprintf(stderr,"%02X ", ((quint8*)pSubDiv2)[i]);
//         }
//         fprintf(stderr,"\n");


        subdiv       = subdivs.begin();
        subdiv_prev  = subdivs.begin();
        subdiv->offsetPolygons2  = gar_load(uint32_t, pSubDiv2->offsetPolygons) + rgnOffPolyg2;
        subdiv->offsetPolylines2 = gar_load(uint32_t, pSubDiv2->offsetPolyline) + rgnOffPolyl2;
        subdiv->offsetPoints2    = skipPois ? 0 : gar_load(uint32_t, pSubDiv2->offsetPoints)   + rgnOffPoint2;

        ++subdiv;
        pSubDiv2 = reinterpret_cast<tre_subdiv2_t*>((quint8*)pSubDiv2 + pTreHdr->tre7_rec_size);

        while(subdiv != subdivs.end()) {

//             for(int i = 0; i < pTreHdr->tre7_rec_size; ++i){
//                 if(i%4 == 0) fprintf(stderr,"\n");
//                 fprintf(stderr,"%02X ", ((quint8*)pSubDiv2)[i]);
//             }
//             fprintf(stderr,"\n");

            subdiv->offsetPolygons2          = gar_load(uint32_t, pSubDiv2->offsetPolygons) + rgnOffPolyg2;
            subdiv->offsetPolylines2         = gar_load(uint32_t, pSubDiv2->offsetPolyline) + rgnOffPolyl2;
            subdiv->offsetPoints2            = skipPois ? 0 : gar_load(uint32_t, pSubDiv2->offsetPoints)   + rgnOffPoint2;

            subdiv_prev->lengthPolygons2     = subdiv->offsetPolygons2    - subdiv_prev->offsetPolygons2;
            subdiv_prev->lengthPolylines2    = subdiv->offsetPolylines2   - subdiv_prev->offsetPolylines2;
            subdiv_prev->lengthPoints2       = skipPois ? 0 : subdiv->offsetPoints2      - subdiv_prev->offsetPoints2;

            subdiv_prev = subdiv;

            ++subdiv;
            pSubDiv2 = reinterpret_cast<tre_subdiv2_t*>((quint8*)pSubDiv2 + pTreHdr->tre7_rec_size);
        }

        subdiv_prev->lengthPolygons2  = rgnOffPolyg2 + rgnLenPolyg2 - subdiv_prev->offsetPolygons2;
        subdiv_prev->lengthPolylines2 = rgnOffPolyl2 + rgnLenPolyl2 - subdiv_prev->offsetPolylines2;
        subdiv_prev->lengthPoints2    = skipPois ? 0 : rgnOffPoint2 + rgnLenPoint2 - subdiv_prev->offsetPoints2;

    }

    subfile.subdivs = subdivs;

#ifdef DEBUG_SHOW_SUBDIV_DATA
    {
        QVector<subdiv_desc_t>::iterator subdiv = subfile.subdivs.begin();
        while(subdiv != subfile.subdivs.end()) {
            qDebug() << "--- subdiv" << subdiv->n << "---";
            qDebug() << "RGN start          " << hex << subdiv->rgn_start;
            qDebug() << "RGN end            " << hex << subdiv->rgn_end;
            qDebug() << "center lng         " << GARMIN_DEG(subdiv->iCenterLng);
            qDebug() << "center lat         " << GARMIN_DEG(subdiv->iCenterLat);
            qDebug() << "has points         " << subdiv->hasPoints;
            qDebug() << "has indexed points " << subdiv->hasIdxPoints;
            qDebug() << "has polylines      " << subdiv->hasPolylines;
            qDebug() << "has polygons       " << subdiv->hasPolygons;
            qDebug() << "bounding area (m)  " << subdiv->area.topLeft() << subdiv->area.bottomRight();
            qDebug() << "map level          " << subdiv->level;
            qDebug() << "left shifts        " << subdiv->shift;

            qDebug() << "polyg off.         " << hex << subdiv->offsetPolygons2;
            qDebug() << "polyg len.         " << hex << subdiv->lengthPolygons2;
            qDebug() << "polyl off.         " << hex << subdiv->offsetPolylines2;
            qDebug() << "polyl len.         " << hex << subdiv->lengthPolylines2;
            qDebug() << "point off.         " << hex << subdiv->offsetPoints2;
            qDebug() << "point len.         " << hex << subdiv->lengthPoints2;
            ++subdiv;
        }
    }
#endif                       // DEBUG_SHOW_SUBDIV_DATA

    //     qDebug() << "***" << hex << subfile.parts["RGN"].offset << (subfile.parts["RGN"].offset + subfile.parts["RGN"].size);
    //     qDebug() << "+++" << hex << rgnOffPolyg2 << (rgnOffPolyg2 + pRgnHdr->length_polyg2);
    //     qDebug() << "+++" << hex << rgnOffPolyl2 << (rgnOffPolyl2 + pRgnHdr->length_polyl2);
    //     qDebug() << "+++" << hex << rgnOffPoint2 << (rgnOffPoint2 + pRgnHdr->length_point2);

    if(subfile.parts.contains("LBL")) {
        QByteArray lblhdr;
        readFile(file, subfile.parts["LBL"].offset, sizeof(hdr_lbl_t), lblhdr);
        hdr_lbl_t * pLblHdr = (hdr_lbl_t*)lblhdr.data();

        quint32 offsetLbl1 = subfile.parts["LBL"].offset + gar_load(uint32_t, pLblHdr->lbl1_offset);
        quint32 offsetLbl6 = subfile.parts["LBL"].offset + gar_load(uint32_t, pLblHdr->lbl6_offset);

        quint32 offsetNet1  = 0;
        hdr_net_t * pNetHdr = 0;
        if(subfile.parts.contains("NET")) {
            QByteArray nethdr;
            readFile(file, subfile.parts["NET"].offset, sizeof(hdr_net_t), nethdr);
            pNetHdr = (hdr_net_t*)nethdr.data();
            offsetNet1 = subfile.parts["NET"].offset + gar_load(uint32_t, pNetHdr->net1_offset);
        }

        quint16 codepage = 0;
        if(gar_load(uint16_t, pLblHdr->length) > 0xAA) {
            codepage = gar_load(uint16_t, pLblHdr->codepage);
        }

        //         qDebug() << file.fileName() << hex << offsetLbl1 << offsetLbl6 << offsetNet1;

        switch(pLblHdr->coding) {
            case 0x06:
                subfile.strtbl = new CGarminStrTbl6(codepage, mask, this);
                subfile.strtbl->registerLBL1(offsetLbl1, pLblHdr->lbl1_length, pLblHdr->addr_shift);
                subfile.strtbl->registerLBL6(offsetLbl6, pLblHdr->lbl6_length);
                if(pNetHdr) subfile.strtbl->registerNET1(offsetNet1, pNetHdr->net1_length, pNetHdr->net1_addr_shift);
                break;

            case 0x09:
                subfile.strtbl = new CGarminStrTbl8(codepage, mask, this);
                subfile.strtbl->registerLBL1(offsetLbl1, pLblHdr->lbl1_length, pLblHdr->addr_shift);
                subfile.strtbl->registerLBL6(offsetLbl6, pLblHdr->lbl6_length);
                if(pNetHdr) subfile.strtbl->registerNET1(offsetNet1, pNetHdr->net1_length, pNetHdr->net1_addr_shift);
                break;

            case 0x0A:
                subfile.strtbl = new CGarminStrTblUtf8(codepage, mask, this);
                subfile.strtbl->registerLBL1(offsetLbl1, pLblHdr->lbl1_length, pLblHdr->addr_shift);
                subfile.strtbl->registerLBL6(offsetLbl6, pLblHdr->lbl6_length);
                if(pNetHdr) subfile.strtbl->registerNET1(offsetNet1, pNetHdr->net1_length, pNetHdr->net1_addr_shift);
                break;

            default:;
            qWarning() << "Unknown label coding" << hex << pLblHdr->coding;
        }

    }

}


void CGarminTile::loadVisibleData(bool fast, polytype_t& polygons, polytype_t& polylines, pointtype_t& points, pointtype_t& pois, unsigned level, const QRectF& viewport)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QMap<QString,subfile_desc_t>::const_iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()) {
        //         qDebug() << "subfile:" << subfile->area << viewport << subfile->area.intersects(viewport);
        if(!subfile->area.intersects(viewport)) {
            ++subfile;
            continue;
        }

        QByteArray rgndata;
        readFile(file, subfile->parts["RGN"].offset, subfile->parts["RGN"].size, rgndata);

        //         qDebug() << "rgn range" << hex << subfile->parts["RGN"].offset << (subfile->parts["RGN"].offset + subfile->parts["RGN"].size);

        const QVector<subdiv_desc_t>&  subdivs = subfile->subdivs;
        // collect polylines
        QVector<subdiv_desc_t>::const_iterator subdiv = subdivs.begin();
        while(subdiv != subdivs.end()) {
            //             if(subdiv->level == level) qDebug() << "subdiv:" << subdiv->level << level <<  subdiv->area << viewport << subdiv->area.intersects(viewport);
            if(subdiv->level != level || !subdiv->area.intersects(viewport)) {
                ++subdiv;
                continue;
            }
            loadSubDiv(file, *subdiv, subfile->strtbl, rgndata, fast, polylines, polygons, points, pois);

#ifdef DEBUG_SHOW_SECTION_BORDERS
            IMap& map = CMapDB::self().getMap();
            const QRectF& a = subdiv->area;
            double u[2] = {a.left(), a.right()};
            double v[2] = {a.top(), a.bottom()};
            map.convertRad2Pt(u,v,2);

            p.setPen(QPen(Qt::magenta,2));
            p.setBrush(Qt::NoBrush);
            p.drawRect(u[0], v[0], u[1] - u[0], v[1] - v[0]);
#endif
            ++subdiv;
        }
        ++subfile;
    }

    file.close();
}


void CGarminTile::loadSubDiv(QFile& file, const subdiv_desc_t& subdiv, IGarminStrTbl * strtbl, const QByteArray& rgndata, bool fast, polytype_t& polylines, polytype_t& polygons, pointtype_t& points, pointtype_t& pois)
{
    if(subdiv.rgn_start == subdiv.rgn_end && !subdiv.lengthPolygons2 && !subdiv.lengthPolylines2 && !subdiv.lengthPoints2) return;

    //     qDebug() << "---------" << file.fileName() << "---------";

    const quint8 * pRawData = (quint8*)rgndata.data();

    quint32 opnt = 0, oidx = 0, opline = 0, opgon = 0;
    quint32 objCnt = subdiv.hasIdxPoints + subdiv.hasPoints + subdiv.hasPolylines + subdiv.hasPolygons;

    quint16 * pOffset = (quint16*)(pRawData + subdiv.rgn_start);

    // test for points
    if(subdiv.hasPoints) {
        opnt = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
    }
    // test for indexed points
    if(subdiv.hasIdxPoints) {
        if(opnt) {
            oidx = gar_load(uint16_t, *pOffset);
            oidx += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            oidx = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }

    }
    // test for polylines
    if(subdiv.hasPolylines) {
        if(opnt || oidx) {
            opline = gar_load(uint16_t, *pOffset);
            opline += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            opline = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }
    }
    // test for polygons
    if(subdiv.hasPolygons) {
        if(opnt || oidx || opline) {
            opgon = gar_load(uint16_t, *pOffset);
            opgon += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            opgon = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }
    }

#ifdef DEBUG_SHOW_POLY_DATA
    qDebug() << "--- Subdivision" << subdiv.n << "---";
    qDebug() << "adress:" << hex << subdiv.rgn_start << "- " << subdiv.rgn_end;
    qDebug() << "points:            " << hex << opnt;
    qDebug() << "indexed points:    " << hex << oidx;
    qDebug() << "polylines:         " << hex << opline;
    qDebug() << "polygons:          " << hex << opgon;
#endif                       // DEBUG_SHOW_POLY_DATA

    const quint8 *  pData;
    const quint8 *  pEnd;

    // decode points
    if(subdiv.hasPoints && !fast) {
        pData = pRawData + opnt;
        pEnd  = pRawData + (oidx ? oidx : opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            points.push_back(CGarminPoint());
            CGarminPoint& p = points.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl) {
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels)
                    : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
        }
    }

    // decode indexed points
    if(subdiv.hasIdxPoints && !fast) {
        pData = pRawData + oidx;
        pEnd  = pRawData + (opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            pois.push_back(CGarminPoint());
            CGarminPoint& p = pois.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl) {
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels)
                    : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
        }
    }

    // decode polylines
    if(subdiv.hasPolylines) {
        CGarminPolygon::cnt = 0;
        pData = pRawData + opline;
        pEnd  = pRawData + (opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            polylines.push_back(CGarminPolygon());

            CGarminPolygon& p   = polylines.last();

            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, true, pData, pEnd);
            if(strtbl && !p.lbl_in_NET && p.lbl_info && !fast) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
            else if(strtbl && p.lbl_in_NET && p.lbl_info && !fast) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
            }
        }
    }

    // decode polygons
    if(subdiv.hasPolygons && !fast && !isTransparent()) {
        CGarminPolygon::cnt = 0;
        pData = pRawData + opgon;
        pEnd  = pRawData + subdiv.rgn_end;

        while(pData < pEnd) {
            polygons.push_back(CGarminPolygon());
            CGarminPolygon& p   = polygons.last();

            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, false, pData, pEnd);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
            else if(strtbl && p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
            }
        }
    }

//         qDebug() << "--- Subdivision" << subdiv.n << "---";
//         qDebug() << "adress:" << hex << subdiv.rgn_start << "- " << subdiv.rgn_end;
//         qDebug() << "polyg off: " << hex << subdiv.offsetPolygons2;
//         qDebug() << "polyg len: " << hex << subdiv.lengthPolygons2 << dec << subdiv.lengthPolygons2;
//         qDebug() << "polyg end: " << hex << subdiv.lengthPolygons2 + subdiv.offsetPolygons2;
//         qDebug() << "polyl off: " << hex << subdiv.offsetPolylines2;
//         qDebug() << "polyl len: " << hex << subdiv.lengthPolylines2 << dec << subdiv.lengthPolylines2;
//         qDebug() << "polyl end: " << hex << subdiv.lengthPolylines2 + subdiv.offsetPolylines2;
//         qDebug() << "point off: " << hex << subdiv.offsetPoints2;
//         qDebug() << "point len: " << hex << subdiv.lengthPoints2 << dec << subdiv.lengthPoints2;
//         qDebug() << "point end: " << hex << subdiv.lengthPoints2 + subdiv.offsetPoints2;

    if(subdiv.lengthPolygons2 && !fast && !isTransparent()) {
        pData   = pRawData + subdiv.offsetPolygons2;
        pEnd    = pData + subdiv.lengthPolygons2;
        while(pData < pEnd) {
            polygons.push_back(CGarminPolygon());
            CGarminPolygon& p   = polygons.last();
            //             qDebug() << "rgn offset:" << hex << (rgnoff + (pData - pRawData));
            pData += p.decode2(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, false, pData, pEnd);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
        }
    }

    if(subdiv.lengthPolylines2) {
        pData   = pRawData + subdiv.offsetPolylines2;
        pEnd    = pData + subdiv.lengthPolylines2;
        while(pData < pEnd) {
            polylines.push_back(CGarminPolygon());
            CGarminPolygon& p   = polylines.last();
            //             qDebug() << "rgn offset:" << hex << (rgnoff + (pData - pRawData));
            pData += p.decode2(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, true, pData, pEnd);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
        }
    }

    if(subdiv.lengthPoints2 && !fast) {
        pData   = pRawData + subdiv.offsetPoints2;
        pEnd    = pData + subdiv.lengthPoints2;
        while(pData < pEnd) {
            pois.push_back(CGarminPoint());
            CGarminPoint& p   = pois.last();
            //             qDebug() << "rgn offset:" << hex << (rgnoff + (pData - pRawData));
            pData += p.decode2(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData, pEnd);

            if(strtbl) {
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels)
                    : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
        }
    }
}


void CGarminTile::loadPolygonsOfType(polytype_t& polygons, quint16 type, unsigned level)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QMap<QString,subfile_desc_t>::const_iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()) {

        QByteArray rgndata;
        readFile(file, subfile->parts["RGN"].offset, subfile->parts["RGN"].size, rgndata);

        const QVector<subdiv_desc_t>&  subdivs = subfile->subdivs;
        // collect polylines
        QVector<subdiv_desc_t>::const_iterator subdiv = subdivs.begin();
        while(subdiv != subdivs.end()) {

            if(subdiv->level != level) {
                ++subdiv;
                continue;
            }

            if(subdiv->rgn_start == subdiv->rgn_end) return;

            const quint8 * pRawData = (quint8*)rgndata.data();

            quint32 opnt = 0, oidx = 0, opline = 0, opgon = 0;
            quint32 objCnt = subdiv->hasIdxPoints + subdiv->hasPoints + subdiv->hasPolylines + subdiv->hasPolygons;

            quint16 * pOffset = (quint16*)(pRawData + subdiv->rgn_start);

            // test for points
            if(subdiv->hasPoints) {
                opnt = (objCnt - 1) * sizeof(quint16) + subdiv->rgn_start;
            }
            // test for indexed points
            if(subdiv->hasIdxPoints) {
                if(opnt) {
                    oidx = gar_load(uint16_t, *pOffset);
                    oidx += subdiv->rgn_start;
                    ++pOffset;
                }
                else {
                    oidx = (objCnt - 1) * sizeof(quint16) + subdiv->rgn_start;
                }

            }
            // test for polylines
            if(subdiv->hasPolylines) {
                if(opnt || oidx) {
                    opline = gar_load(uint16_t, *pOffset);
                    opline += subdiv->rgn_start;
                    ++pOffset;
                }
                else {
                    opline = (objCnt - 1) * sizeof(quint16) + subdiv->rgn_start;
                }
            }
            // test for polygons
            if(subdiv->hasPolygons) {
                if(opnt || oidx || opline) {
                    opgon = gar_load(uint16_t, *pOffset);
                    opgon += subdiv->rgn_start;
                    ++pOffset;
                }
                else {
                    opgon = (objCnt - 1) * sizeof(quint16) + subdiv->rgn_start;
                }
            }

            const quint8 *  pData;
            const quint8 *  pEnd;

            // decode polygons
            if(subdiv->hasPolygons) {
                pData = pRawData + opgon;
                pEnd  = pRawData + subdiv->rgn_end;
                while(pData < pEnd) {
                    polygons.push_back(CGarminPolygon());
                    CGarminPolygon& p = polygons.last();
                    pData += p.decode(subdiv->iCenterLng, subdiv->iCenterLat, subdiv->shift, false, pData, pEnd);

                    if(p.type != type) {
                        polygons.pop_back();
                    }
                    else {

                        if(subfile->strtbl && !p.lbl_in_NET && p.lbl_info) {
                            subfile->strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
                        }
                        else if(subfile->strtbl && p.lbl_in_NET && p.lbl_info) {
                            subfile->strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
                        }
                    }
                }
            }

            ++subdiv;
        }
        ++subfile;
    }
}


void CGarminTile::createIndex(QSqlDatabase& db)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QSqlQuery query(db);

    QMap<QString,subfile_desc_t>::const_iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()) {
        quint8 level = subfile->maplevels.last().level;

        query.prepare("INSERT INTO subfiles (name, filename) VALUES (:name, :filename)");
        query.bindValue(":name", subfile.key());
        query.bindValue(":filename", filename);
        if(!query.exec()) {
            qDebug() << query.lastError();
        }

        query.prepare("Select id FROM subfiles WHERE filename = :filename");
        query.bindValue(":filename", filename);
        if(!query.exec()) {
            qDebug() << query.lastError();
        }
        query.next();
        quint32 idSubfile = query.value(0).toUInt();

        QByteArray rgndata;
        readFile(file, subfile->parts["RGN"].offset, subfile->parts["RGN"].size, rgndata);

        const QVector<subdiv_desc_t>&  subdivs = subfile->subdivs;
        // collect polylines
        QVector<subdiv_desc_t>::const_iterator subdiv = subdivs.begin();
        while(subdiv != subdivs.end()) {

            if(subdiv->level != level) {
                ++subdiv;
                continue;
            }

            createIndexSubDiv(file, idSubfile, *subdiv, subfile->strtbl, rgndata, db);

            ++subdiv;
        }

        ++subfile;
    }
}


void CGarminTile::createIndexSubDiv(QFile& file, quint32 idSubfile, const subdiv_desc_t& subdiv, IGarminStrTbl * strtbl, const QByteArray& rgndata, QSqlDatabase& db)
{
    if(subdiv.rgn_start == subdiv.rgn_end) return;

    const quint8 * pRawData = (quint8*)rgndata.data();

    quint32 opnt = 0, oidx = 0, opline = 0, opgon = 0;
    quint32 objCnt = subdiv.hasIdxPoints + subdiv.hasPoints + subdiv.hasPolylines + subdiv.hasPolygons;

    quint16 * pOffset = (quint16*)(pRawData + subdiv.rgn_start);

    // test for points
    if(subdiv.hasPoints) {
        opnt = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
    }
    // test for indexed points
    if(subdiv.hasIdxPoints) {
        if(opnt) {
            oidx = gar_load(uint16_t, *pOffset);
            oidx += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            oidx = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }

    }
    // test for polylines
    if(subdiv.hasPolylines) {
        if(opnt || oidx) {
            opline = gar_load(uint16_t, *pOffset);
            opline += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            opline = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }
    }
    // test for polygons
    if(subdiv.hasPolygons) {
        if(opnt || oidx || opline) {
            opgon = gar_load(uint16_t, *pOffset);
            opgon += subdiv.rgn_start;
            ++pOffset;
        }
        else {
            opgon = (objCnt - 1) * sizeof(quint16) + subdiv.rgn_start;
        }
    }

#ifdef DEBUG_SHOW_POLY_DATA
    qDebug() << "--- Subdivision" << subdiv.n << "---";
    qDebug() << "adress:" << hex << subdiv.rgn_start << "- " << subdiv.rgn_end;
    qDebug() << "points:            " << hex << opnt;
    qDebug() << "indexed points:    " << hex << oidx;
    qDebug() << "polylines:         " << hex << opline;
    qDebug() << "polygons:          " << hex << opgon;
#endif                       // DEBUG_SHOW_POLY_DATA

    const quint8 *  pData;
    const quint8 *  pEnd;

    // decode polylines
    if(subdiv.hasPolylines) {
        pData = pRawData + opline;
        pEnd  = pRawData + (opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            CGarminPolygon p;
            quint32 offset = pData - pRawData;

            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, true, pData, pEnd);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
            else if(strtbl && p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
            }

            if(!p.labels.isEmpty() && !(0x20 <= p.type && p.type <= 0x25)) {
                double lon1 = p.u[0];
                double lat1 = p.v[0];
                double lon2 = lon1;
                double lat2 = lat1;
                const int size = p.u.size();
                for(int i = 0; i < size; ++i) {
                    const double u = p.u[i];
                    const double v = p.v[i];
                    if(u < lon1) lon1 = u;
                    if(u > lon2) lon2 = u;
                    if(v > lat1) lat1 = v;
                    if(v < lat2) lat2 = v;
                }

                QSqlQuery query(db);
                query.prepare(QString("INSERT INTO polylines (type, subfile, subdiv, offset, lon1, lat1, lon2, lat2, label) VALUES (%1, %2, %3, %4, %5, %6, %7, %8, :label)").arg(p.type).arg(idSubfile).arg(subdiv.n).arg(offset).arg(lon1).arg(lat1).arg(lon2).arg(lat2));
                query.bindValue(":label", p.labels.join(" ").simplified());
                if(!query.exec()) {
                    qDebug() << query.lastError();
                    qDebug() << query.lastQuery();
                }
            }
        }
    }

    // decode indexed points
    if(subdiv.hasIdxPoints) {
        pData = pRawData + oidx;
        pEnd  = pRawData + (opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            CGarminPoint p;
            quint32 offset = pData - pRawData;

            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl) {
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels) : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }

            if(!p.labels.isEmpty()) {
                QSqlQuery query(db);
                query.prepare(QString("INSERT INTO points (type, subfile, subdiv, offset, lon1, lat1, label) VALUES (%1, %2, %3, %4, %5, %6, :label)").arg(p.type).arg(idSubfile).arg(subdiv.n).arg(offset).arg(p.lon).arg(p.lat));
                query.bindValue(":label", p.labels.join(" ").simplified());
                if(!query.exec()) {
                    qDebug() << query.lastError();
                    qDebug() << query.lastQuery();
                }
            }
        }
    }

    // decode points
    if(subdiv.hasPoints) {
        pData = pRawData + opnt;
        pEnd  = pRawData + (oidx ? oidx : opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            CGarminPoint p;
            quint32 offset = pData - pRawData;

            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl) {
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels) : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
            if(!p.labels.isEmpty()) {
                QSqlQuery query(db);
                query.prepare(QString("INSERT INTO points (type, subfile, subdiv, offset, lon1, lat1, label) VALUES (%1, %2, %3, %4, %5, %6, :label)").arg(p.type).arg(idSubfile).arg(subdiv.n).arg(offset).arg(p.lon).arg(p.lat));
                query.bindValue(":label", p.labels.join(" ").simplified());
                if(!query.exec()) {
                    qDebug() << query.lastError();
                    qDebug() << query.lastQuery();
                }
            }
        }
    }

}


void CGarminTile::readPolyline(const QString& subfile, quint32 n, quint32 offset, polytype_t& polylines)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray rgndata;
    readFile(file, subfiles[subfile].parts["RGN"].offset, subfiles[subfile].parts["RGN"].size, rgndata);
    subdiv_desc_t& subdiv = subfiles[subfile].subdivs[n];

    const quint8 *  pData = (quint8*)rgndata.data() + offset;

    polylines.push_back(CGarminPolygon());
    CGarminPolygon& p = polylines.last();
    pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, true, pData, 0);

    file.close();
}


void CGarminTile::readPoint(const QString& subfile, quint32 n, quint32 offset, pointtype_t& point)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray rgndata;
    readFile(file, subfiles[subfile].parts["RGN"].offset, subfiles[subfile].parts["RGN"].size, rgndata);
    subdiv_desc_t& subdiv = subfiles[subfile].subdivs[n];

    const quint8 *  pData = (quint8*)rgndata.data() + offset;

    point.push_back(CGarminPoint());
    CGarminPoint& p = point.last();
    pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);

    file.close();
}
