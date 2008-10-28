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

#include "Platform.h"
#include "IGarminStrTbl.h"

#include <QtCore>

IGarminStrTbl::IGarminStrTbl(const quint16 codepage, const quint8 mask, QObject * parent)
: QObject(parent)
, offsetLBL1(0)
, offsetLBL6(0)
, offsetNET1(0)
, addrshift1(0)
, addrshift2(0)
, codepage(codepage)
, codec(0)
, mask(mask)
{
    if(codepage != 0) {

        if(1250 <= codepage && codepage <= 1258) {
            char strcp[64];
            sprintf(strcp,"Windows-%i",codepage);
            codec = QTextCodec::codecForName(strcp);
        }
        else if(codepage == 950) {
            codec = QTextCodec::codecForName("Big5");
        }
        else if(codepage == 850) {
            codec = QTextCodec::codecForName("IBM 850");
        }
        else {
            qDebug() << "unknown codepage:" << codepage << "0x" << hex << codepage;
        }
    }
}


IGarminStrTbl::~IGarminStrTbl()
{

}


void IGarminStrTbl::readFile(QFile& file, quint32 offset, quint32 size, QByteArray& data)
{
    file.seek(offset);
    data = file.read(size);

    if((quint32)data.size() != size) {
        //         throw exce_t(eErrOpen, tr("Failed to read: ") + file.filename());
        return;
    }

    quint8 * p = (quint8*)data.data();
    for(quint32 i = 0; i < size; ++i) {
        *p++ ^= mask;
    }

}


quint32 IGarminStrTbl::calcOffset(QFile& file, const quint32 offset, type_e t)
{
    quint32 newOffset = offset;

    if(t == poi) {
        QByteArray buffer;
        readFile(file, offsetLBL6 + offset, sizeof(uint32_t), buffer);
        newOffset = gar_ptr_load(uint32_t, buffer.data());
        newOffset = (newOffset & 0x003FFFFF);
    }
    else if(t == net) {
        if(offsetNET1 == 0) {
            return 0xFFFFFFFF;
        }

        QByteArray data;
        readFile(file,  offsetNET1 + (offset << addrshift2), sizeof(uint32_t), data);
        newOffset = gar_ptr_load(uint32_t, data.data());
        if(newOffset & 0x00400000) {
            return 0xFFFFFFFF;
        }
        newOffset = (newOffset & 0x003FFFFF);
    }

    newOffset <<= addrshift1;
    //     qDebug() << hex << newOffset;
    return newOffset;
}
