/***************************************************************************
 *   Copyright (C) 2008 by Manuel Schrape                                  *
 *   manuel.schrape@gmx.de                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QString>
#include <QIcon>
#include "../global.h"

class ToolBox
{
    public :
        ToolBox();
        static bool Decode64(sRxData &RX, bool Long = true);
        static QString Encode64(char Data[150],unsigned int Length);
        static bool check_CRC(QString RXString);
        static QString add_CRC(QString TXString);
        static int Data2Char(int *Data , int Start, bool is_signed = true);
        static int Data2Int(int *Data , int Start, bool is_signed = true);
        static long Data2Long(int *Data , int Start, bool is_signed = true);

        static int Char2Data(int Data);

        static QString Data2QString(int Data[150], int Start = 0, int End = 150);
        static QIcon Icon(int ID);
        static QString get_Float(long Wert, int Count);
        static void Wait(int Time);
};

#endif // TOOLBOX_H
