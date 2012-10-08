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
#include "CSearch.h"

CSearch::CSearch(QObject * parent)
: IItem(parent)
, lon(0.0)
, lat(0.0)
{
    iconPixmap = QPixmap(":/icons/iconBullseye16x16.png");
    iconString = "Bullseye";
}


CSearch::~CSearch()
{

}


QString CSearch::getInfo()
{
    QString str;

    if(!street.isEmpty())
    {
        str = street;
    }

    if(!municipal.isEmpty())
    {
        if(!str.isEmpty()) str += ", ";
        if(!postalCode.isEmpty()) str += postalCode + " ";

        str += municipal;
    }

    if(!country.isEmpty())
    {
        if(!str.isEmpty()) str += ", ";
        str += country;
    }

    if(!countryCode.isEmpty())
    {
        if(!str.isEmpty()) str += ", ";
        str += "(" + countryCode.toUpper() + ")";
    }

    return str.isEmpty() ? name : str;
}
