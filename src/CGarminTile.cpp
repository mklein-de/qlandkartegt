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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#include "CGarminTile.h"
#include "CGarminStrTbl8.h"
#include "Platform.h"
#include "IMap.h"
#include "CMapDB.h"

#include <QtGui>

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

    if((quint32)data.size() != size){
        throw exce_t(eErrOpen, tr("Failed to read: ") + filename);
    }

    quint8 * p = (quint8*)data.data();
    for(quint32 i = 0; i < size; ++i){
        *p++ ^= mask;
    }

}

void CGarminTile::readBasics(const QString& fn)
{
    char tmpstr[64];

    filename        = fn;
    qint64 fsize    = QFileInfo(fn).size();

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        throw exce_t(eErrOpen, tr("Failed to open: ") + filename);
    }
    file.read((char*)&mask, 1);

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
            qDebug() << "--- subfile" << subfile->name << "---";
            QMap<QString,subfile_part_t>::const_iterator part = subfile->parts.begin();
            while(part != subfile->parts.end()) {
                qDebug() << part.key() << hex << part->offset << part->size;
                ++part;
            }
            ++subfile;
        }
    }
#endif                       //DEBUG_SHOW_SECT_DESC

    QMap<QString,subfile_desc_t>::iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()) {
        subfile->name = mapdesc.trimmed();
        readSubfileBasics(*subfile, file);
        ++subfile;
    }
}

void CGarminTile::readSubfileBasics(subfile_desc_t& subfile, QFile& file)
{
    quint32 i;
    // test for mandatory subfile parts
    if(!(subfile.parts.contains("TRE") && subfile.parts.contains("RGN"))) return;

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

//         if(!subdiv->area.isValid()) {
//             qDebug() << subdiv->north << subdiv->east << subdiv->south << subdiv->west << subdiv->area;
//         }

//         subdiv->strtbl = strtbl;

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
//         if(!subdiv->area.isValid()) {
//             qDebug() << subdiv->north << subdiv->east << subdiv->south << subdiv->west << subdiv->area;
//         }

//         subdiv->strtbl = strtbl;

        subdiv_prev = subdiv;
        ++pSubDivL; ++subdiv;
    }
    subdivs.last().rgn_end = /*subfile.parts["RGN"].offset +*/ subfile.parts["RGN"].size;


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
            ++subdiv;
        }
    }
#endif // DEBUG_SHOW_SUBDIV_DATA

    if(subfile.parts.contains("LBL")){
        QByteArray lblhdr;
        readFile(file, subfile.parts["LBL"].offset, sizeof(hdr_lbl_t), lblhdr);
        hdr_lbl_t * pLblHdr = (hdr_lbl_t*)lblhdr.data();

        quint32 offsetLbl1 = subfile.parts["LBL"].offset + gar_load(uint32_t, pLblHdr->lbl1_offset);
        quint32 offsetLbl6 = subfile.parts["LBL"].offset + gar_load(uint32_t, pLblHdr->lbl6_offset);

        quint32 offsetNet1  = 0;
        hdr_net_t * pNetHdr = 0;
        if(subfile.parts.contains("NET")){
            QByteArray nethdr;
            readFile(file, subfile.parts["NET"].offset, sizeof(hdr_net_t), nethdr);
            pNetHdr = (hdr_net_t*)nethdr.data();
            offsetNet1 = subfile.parts["NET"].offset + gar_load(uint32_t, pNetHdr->net1_offset) + gar_load(uint32_t, pLblHdr->length);
        }

        quint16 codepage = 0;
        if(gar_load(uint16_t, pLblHdr->length) > 0xAA){
            codepage = gar_load(uint16_t, pLblHdr->codepage);
        }

//         qDebug() << file.fileName() << hex << offsetLbl1 << offsetLbl6 << offsetNet1;

        switch(pLblHdr->coding) {
            case 0x06:
                qDebug() << "6bit";
//                 tbl = new CGarminStrTbl6(pDataLBL,sizeLBL,parent);
                break;

            case 0x09:
//                 tbl = new CGarminStrTbl8(pDataLBL,sizeLBL,parent);
                subfile.strtbl = new CGarminStrTbl8(codepage, mask, this);
                subfile.strtbl->registerLBL1(offsetLbl1, pLblHdr->lbl1_length, pLblHdr->addr_shift);
                subfile.strtbl->registerLBL6(offsetLbl6, pLblHdr->lbl6_length);
                if(pNetHdr) subfile.strtbl->registerNET1(offsetNet1, pNetHdr->net1_length, pNetHdr->net1_addr_shift);
                break;

            case 0x0A:
                qDebug() << "utf-8";
//                 tbl = new CGarminStrTbl10(pDataLBL,sizeLBL,parent);
                break;

            default:;
                qWarning() << "Unknown label coding" << hex << pLblHdr->coding;
        }

    }


}


void CGarminTile::loadVisibleData(polytype_t& polygons, polytype_t& polylines, pointtype_t& points, pointtype_t& pois, unsigned level, double scale, const QRectF& viewport)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        return;
    }


    QMap<QString,subfile_desc_t>::const_iterator subfile = subfiles.begin();
    while(subfile != subfiles.end()){
        if(!subfile->area.intersects(viewport)){
            ++subfile;
            continue;
        }

        QByteArray rgndata;
        readFile(file, subfile->parts["RGN"].offset, subfile->parts["RGN"].size, rgndata);

        const QVector<subdiv_desc_t>&  subdivs = subfile->subdivs;
        // collect polylines
        QVector<subdiv_desc_t>::const_iterator subdiv = subdivs.begin();
        while(subdiv != subdivs.end()){

            if(subdiv->level != level || !subdiv->area.intersects(viewport)){
                ++subdiv;
                continue;
            }

            loadSuvDiv(file, *subdiv, subfile->strtbl, rgndata, polylines, polygons, points, pois);

#ifdef DEBUG_SHOW_SECTION_BORDERS
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
}

void CGarminTile::loadSuvDiv(QFile& file, const subdiv_desc_t& subdiv, IGarminStrTbl * strtbl, const QByteArray& rgndata, polytype_t& polylines, polytype_t& polygons, pointtype_t& points, pointtype_t& pois)
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

    // decode points
    if(subdiv.hasPoints) {
        pData = pRawData + opnt;
        pEnd  = pRawData + (oidx ? oidx : opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            points.push_back(CGarminPoint());
            CGarminPoint& p = points.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl){
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels)
                         : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
        }
    }

    // decode indexed points
    if(subdiv.hasIdxPoints) {
        pData = pRawData + oidx;
        pEnd  = pRawData + (opline ? opline : opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            pois.push_back(CGarminPoint());
            CGarminPoint& p = pois.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, pData);
            if(strtbl){
                p.isLbl6 ? strtbl->get(file, p.lbl_ptr, IGarminStrTbl::poi, p.labels)
                         : strtbl->get(file, p.lbl_ptr, IGarminStrTbl::norm, p.labels);
            }
        }
    }

    // decode polylines
    if(subdiv.hasPolylines) {
        pData = pRawData + opline;
        pEnd  = pRawData + (opgon ? opgon : subdiv.rgn_end);
        while(pData < pEnd) {
            polylines.push_back(CGarminPolygon());
            CGarminPolygon& p = polylines.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, true, pData);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
            else if(strtbl && p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
            }

//             qDebug() << p.labels;
        }

    }

    // decode polygons
    if(subdiv.hasPolygons) {
        pData = pRawData + opgon;
        pEnd  = pRawData + subdiv.rgn_end;
        while(pData < pEnd) {
            polygons.push_back(CGarminPolygon());
            CGarminPolygon& p = polygons.last();
            pData += p.decode(subdiv.iCenterLng, subdiv.iCenterLat, subdiv.shift, false, pData);
            if(strtbl && !p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::norm, p.labels);
            }
            else if(strtbl && p.lbl_in_NET && p.lbl_info) {
                strtbl->get(file, p.lbl_info,IGarminStrTbl::net, p.labels);
            }
//             if(polygon.type == 0x4a && polygon.labels.size() > 1) {
//                 subfile.definitionAreas[polygon.labels[1]] = polygon;
//             }
        }
    }
}

