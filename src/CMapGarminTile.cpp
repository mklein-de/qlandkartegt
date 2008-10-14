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
#include "CMapGarminTile.h"
#include "Platform.h"

#include <QtGui>

#undef DEBUG_SHOW_SECT_DESC
#undef DEBUG_SHOW_TRE_DATA
#undef DEBUG_SHOW_MAPLEVEL_DATA

CMapGarminTile::CMapGarminTile(QObject * parent)
: QObject(parent)
, transparent(false)
{

}

CMapGarminTile::~CMapGarminTile()
{

}

void CMapGarminTile::readFile(QFile& file, quint32 offset, quint32 size, QByteArray& data)
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

void CMapGarminTile::readBasics(const QString& fn)
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

void CMapGarminTile::readSubfileBasics(subfile_desc_t& subfile, QFile& file)
{
    quint32 i;
    // test for mandatory subfile parts
    if(!(subfile.parts.contains("TRE") && subfile.parts.contains("RGN"))) return;

    QByteArray trehdr;
    readFile(file, subfile.parts["TRE"].offset, sizeof(hdr_tre_t), trehdr);
    const hdr_tre_t * pTreHdr = (const hdr_tre_t * )trehdr.data();

    if(pTreHdr->flag & 0x80) {
        throw exce_t(errLock,tr("File contains locked / encypted data. Garmin does not "
                                "want you to use this file with any other software than "
                                "the one supplied by Garmin."));
    }

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

    subfile.area = QRectF(QPointF(subfile.west * RAD_TO_DEG, subfile.north * RAD_TO_DEG), QPointF(subfile.east * RAD_TO_DEG, subfile.south * RAD_TO_DEG));

#ifdef DEBUG_SHOW_TRE_DATA
    qDebug() << "bounding area (rad)" << subfile.north << subfile.east << subfile.south << subfile.west;
    qDebug() << "bounding area (\260)" << subfile.area;
#endif                       // DEBUG_SHOW_TRE_DATA

    QByteArray maplevel;
    readFile(file, subfile.parts["TRE"].offset + gar_load(uint32_t, pTreHdr->tre1_offset), gar_load(uint32_t, pTreHdr->tre1_size), maplevel);
    const tre_map_level_t * pMapLevel = (const tre_map_level_t * )maplevel.data();

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
}
