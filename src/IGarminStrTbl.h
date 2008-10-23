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

#ifndef IGARMINSTRTBL_H
#define IGARMINSTRTBL_H

#include <QObject>

class IGarminStrTbl : public QObject
{
    public:
        IGarminStrTbl(const quint16 codepage, QObject * parent);
        virtual ~IGarminStrTbl();

        enum type_e {norm,poi,net};

        void registerLBL1(const quint32 offset, const quint32 size, const quint8 shift){offsetLBL1 = offset; sizeLBL1 = size; addrshift1 = shift;}
        void registerLBL6(const quint32 offset, const quint32 size){offsetLBL6 = offset; sizeLBL6 = size;}
        void registerNET1(const quint32 offset, const quint32 size, const quint8 shift){offsetNET1 = offset; sizeNET1 = size; addrshift2 = shift;}

    private:
        quint32 calcOffset(const quint32 offset, type_e t);

        quint32 offsetLBL1;
        quint32 sizeLBL1;
        quint32 offsetLBL6;
        quint32 sizeLBL6;
        quint32 offsetNET1;
        quint32 sizeNET1;

        quint8 addrshift1;
        quint8 addrshift2;

        // conversion of strings
        quint16 codepage;
        QTextCodec * codec;
};

#endif //IGARMINSTRTBL_H


