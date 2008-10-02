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
#ifndef CLIVELOG_H
#define CLIVELOG_H

#include <QtGlobal>
#include "CWpt.h"

class CLiveLog
{
    public:
        CLiveLog() : fix(eOff), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT)
            , timestamp(0xFFFFFFFF), error_horz(WPT_NOFLOAT), error_vert(WPT_NOFLOAT)
            , heading(WPT_NOFLOAT), velocity(WPT_NOFLOAT){};
        virtual ~CLiveLog();

        enum fix_e {eNoFix, e2DFix, e3DFix, eOff};

        fix_e fix;
        float lon;
        float lat;
        float ele;
        quint32 timestamp;
        float error_horz;
        float error_vert;
        float heading;
        float velocity;
};

extern void operator <<(QDataStream& s, const CLiveLog& log);
extern void operator <<(QFile& f, const CLiveLog& log);
extern void operator >>(QDataStream& s, CLiveLog& log);
#endif                           //CLIVELOG_H
