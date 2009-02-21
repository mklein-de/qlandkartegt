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

#include "CUnitNautic.h"

CUnitNautic::CUnitNautic(QObject * parent)
: IUnit("nautic", "nm", 0.00053989f, "nm/h", 1.94361780f, parent)
{

}


CUnitNautic::~CUnitNautic()
{

}


void CUnitNautic::meter2elevation(float meter, QString& val, QString& unit)
{
    val.sprintf("%1.0f", meter);
    unit = "m";
}


void CUnitNautic::meter2distance(float meter, QString& val, QString& unit)
{
    val.sprintf("%1.2f", meter * basefactor);
    unit = baseunit;
}


void CUnitNautic::meter2speed(float meter, QString& val, QString& unit)
{
    val.sprintf("%1.2f",meter * speedfactor);
    unit = speedunit;
}


float CUnitNautic::elevation2meter(const QString& val)
{
    return val.toDouble();
}

float CUnitNautic::str2speed(QString& str)
{
    return (str.remove(" nm/h").toDouble() / 0.53989f);
}

float CUnitNautic::str2distance(QString& str)
{
    return (str.remove(" nm").toDouble() / 0.00053989f);
}

