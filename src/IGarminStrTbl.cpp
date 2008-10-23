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

#include "IGarminStrTbl.h"

#include <QtCore>

IGarminStrTbl::IGarminStrTbl(const quint16 codepage, QObject * parent)
: QObject(parent)
, offsetLBL1(0)
, offsetLBL6(0)
, offsetNET1(0)
, addrshift1(0)
, addrshift2(0)
, codepage(codepage)
, codec(0)
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

quint32 IGarminStrTbl::calcOffset(const quint32 offset, type_e t)
{
    quint32 newOffset = offset;

//     if(t == poi) {
//         newOffset = gar_ptr_load(uint32_t, pLbl6 + offset);
//         newOffset = (newOffset & 0x003FFFFF);
//     }
//     else if(t == net) {
//         if(hdrNet == 0) {
//             return 0xFFFFFFFF;
//         }
//
//         newOffset = gar_ptr_load(uint32_t, pNet1 + (offset << hdrNet->net1_addr_shift));
//         if(newOffset & 0x00400000) {
//             return 0xFFFFFFFF;
//         }
//         newOffset = (newOffset & 0x003FFFFF);
//     }

    newOffset <<= addrshift1;
    //     qDebug() << hex << newOffset;
    return newOffset;
}

