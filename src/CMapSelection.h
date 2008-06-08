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
#ifndef CMAPSELECTION_H
#define CMAPSELECTION_H

#include <QString>

class CMapSelection
{
    public:
        CMapSelection() : lon1(0), lat1(0), lon2(0), lat2(0) {};

        static QString focusedMap;
        QString key;
        QString mapkey;
        QString description;
        double lon1; ///< top left longitude [rad]
        double lat1; ///< top left latitude [rad]
        double lon2; ///< bottom right longitude [rad]
        double lat2; ///< bottom right latitude [rad]


};

#endif //CMAPSELECTION_H

