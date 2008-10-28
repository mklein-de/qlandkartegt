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

#ifndef GARMIN_H
#define GARMIN_H

#include <projects.h>

#define GARMIN_DEG(x) ((x) < 0x800000 ? (double)(x) * 360.0 / 16777216.0 : (double)((x) - 0x1000000) * 360.0 / 16777216.0)
#define GARMIN_RAD(x) ((x) < 0x800000 ? (double)(x) * TWOPI / 16777216.0 : (double)((x) - 0x1000000) * TWOPI / 16777216.0)
typedef quint8 quint24[3];
#endif                           //GARMIN_H
