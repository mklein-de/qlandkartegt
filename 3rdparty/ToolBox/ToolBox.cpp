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

#include "ToolBox.h"

ToolBox::ToolBox()
{
}

void ToolBox::Wait(int Time)
{
#ifndef _WIN32_
    usleep(Time);
#else
//    sleep(Time);
#endif
}

QString ToolBox::get_Float(long Wert, int Count)
{
    QString Temp, s_Wert;

    s_Wert = QString("%1").arg(Wert);

    Temp = s_Wert.left(s_Wert.length() - Count) + QString(".") + s_Wert.right(Count);

/*
    if (Wert > 0)
        Temp = "";
    else
        Temp = "-";

    Temp = Temp + QString("%1").arg(Wert / Count) + "." + QString("%1").arg(Wert % Count);
*/
    return Temp;
}


// Base64 Decoder
bool ToolBox::Decode64(sRxData &RX, bool Long)
{
    unsigned char a,b,c,d;
    unsigned char ptr = 0;
    unsigned char x,y,z;
    int ptrOut[150];

    int ptrIn = 3;
    int max = RX.String.length();
    int len = RX.String.length();
    int DecLen = 0;

    if (RX.Input[ptrIn] == 0)
    {
//        qDebug("QString to Char ERROR...!!!!");
        return false;
    }

    while(len != 0)
    {
        a = RX.Input[ptrIn++] - '=';
        b = RX.Input[ptrIn++] - '=';
        c = RX.Input[ptrIn++] - '=';
        d = RX.Input[ptrIn++] - '=';

        if(ptrIn > max - 2) break; // nicht mehr Daten verarbeiten, als empfangen wurden

        x = (a << 2) | (b >> 4);
        y = ((b & 0x0f) << 4) | (c >> 2);
        z = ((c & 0x03) << 6) | d;

        if(len--) ptrOut[ptr++] = x; else break;
        if(len--) ptrOut[ptr++] = y; else break;
        if(len--) ptrOut[ptr++] = z; else break;
    }

    for (int a=0; a<ptr; a++)
    {
        if (Long == false)
        {
            int b1, b2, b3;

            b1 = ptrOut[a++];
            b2 = ptrOut[a];

            b3 = (b2 << 8) | b1;

            if (b3 > 32767)
                b3 = b3 - 65536;

            RX.Decode[DecLen] = b3;
            DecLen++;
        }
        else
        {
            RX.Decode[DecLen] = ptrOut[a];
            DecLen++;
        }

        RX.DecLen = DecLen;
    }
    return true;
}

// Base64 Encoder
QString ToolBox::Encode64(char Data[150],unsigned int Length)
{
    unsigned int pt = 0;
    unsigned char a,b,c;
    unsigned char ptr = 0;

    char TX_Buff[150];

    while(Length > 0)
    {
        if(Length) { a = Data[ptr++]; Length--;} else a = 0;
        if(Length) { b = Data[ptr++]; Length--;} else b = 0;
        if(Length) { c = Data[ptr++]; Length--;} else c = 0;

        TX_Buff[pt++] = '=' + (a >> 2);
        TX_Buff[pt++] = '=' + (((a & 0x03) << 4) | ((b & 0xf0) >> 4));
        TX_Buff[pt++] = '=' + (((b & 0x0f) << 2) | ((c & 0xc0) >> 6));
        TX_Buff[pt++] = '=' + ( c & 0x3f);
    }
    TX_Buff[pt] = 0;

    return QString(TX_Buff);
}

// Datensatz nach 8bit Integer
int ToolBox::Data2Char(int *Data , int Start, bool is_signed)
{
    int Out = (Data[Start]);

    if ((Out > 128) && (is_signed))
      Out = Out - 256;

    return Out;

}

// Datensatz nach 8bit Integer
int ToolBox::Char2Data(int Data)
{
    if (Data < 0)
    {
        return Data + 256;
    }
    return Data;
}


// Datensatz nach 16bit Integer
int ToolBox::Data2Int(int *Data , int Start, bool is_signed)
{
    int Out = (Data[Start+1]<<8) | (Data[Start+0]);

    if ((Out > 32767) && (is_signed))
      Out = Out - 65536;

    return Out;

}

// Datensatz nach 32bit Long
long ToolBox::Data2Long(int *Data , int Start, bool is_signed)
{
    long Out = (Data[Start+3]<<24) | (Data[Start+2]<<16) | (Data[Start+1]<<8) | (Data[Start+0]);

    if ((Out > 32767) && (is_signed))
      Out = Out;

    return Out;
}

// Datensatz nach QString
QString ToolBox::Data2QString(int Data[150], int Start, int End)
{
    char String[150];
    for (int a = Start; a < End; a++)
    {
        String[a - Start] = Data[a];
    }
    String[End - Start] = '\0';

    return QString(String);
}

// Datensatz-CRC prüfen
bool ToolBox::check_CRC(QString RXString)
{
    int CRC = 0;
    char *RX;

    int Length = RXString.length();

    RX = RXString.toLatin1().data();

    if (RX[1] == 127)
    {
        RX[1] = 0;
    }

    for(int i=0; i < Length-2; i++)
    {
            CRC+=RX[i];
    }

    CRC = CRC % 4096;

    if(RX[Length - 2] != ('=' + (CRC / 64)))
    {
        return false;
    }

    if(RX[Length - 1] != ('=' + CRC % 64))
    {
        return false;
    }

    return true;
}

// Datensatz-CRC hinzufügen
QString ToolBox::add_CRC(QString TXString)
{
    unsigned int tmpCRC = 0;

    char *TXBuff;
    char CRC[2];

    TXBuff = TXString.toLatin1().data();

    for(int i = 0; i < TXString.length(); i++)
    {
        tmpCRC += TXBuff[i];
    }

    tmpCRC %= 4096;

    CRC[0] = '=' + tmpCRC / 64;
    CRC[1] = '=' + tmpCRC % 64;
    CRC[2] = '\0';

    QString Return = TXString + QString(CRC);

    return Return;
}

// Alle Icons
QIcon ToolBox::Icon(int ID)
{
    QIcon Icons[5] ;
    Icons[0].addPixmap(QPixmap(QString::fromUtf8(":/LED/Images/16X16/ledred.png")), QIcon::Normal, QIcon::Off);
    Icons[1].addPixmap(QPixmap(QString::fromUtf8(":/LED/Images/16X16/ledyellow.png")), QIcon::Normal, QIcon::Off);
    Icons[3].addPixmap(QPixmap(QString::fromUtf8(":/LED/Images/16X16/ledyellow.png")), QIcon::Normal, QIcon::Off);
    Icons[4].addPixmap(QPixmap(QString::fromUtf8(":/LED/Images/16X16/ledoff.png")), QIcon::Normal, QIcon::Off);
    return Icons[ID];
}
