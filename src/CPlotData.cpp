/**********************************************************************************************
    Copyright (C) 2006, 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CPlotData.h"
#include "CPlotAxis.h"

CPlotData::CPlotData(QObject * parent)
: QObject(parent)
, grid(true)
{
    xaxis = new CPlotAxis(this);
    xaxis->setAutoscale(false);
    yaxis = new CPlotAxis(this);
}


CPlotData::~CPlotData()
{

}


void CPlotData::setLimits()
{

    QList<line_t>::const_iterator line  = lines.begin();
    if(line == lines.end()) return;
    QPolygonF::const_iterator p         = line->points.begin();

    double xmin = p->x();
    double xmax = p->x();
    double ymin = p->y();
    double ymax = p->y();

    while(line != lines.end()) {
        QPolygonF::const_iterator p = line->points.begin();
        while(p != line->points.end()) {
            if(p->x() > xmax) xmax = p->x();
            if(p->x() < xmin) xmin = p->x();
            if(p->y() > ymax) ymax = p->y();
            if(p->y() < ymin) ymin = p->y();
            ++p;
        }

        ++line;
    }

    xaxis->setLimits(xmin,xmax);
    yaxis->setLimits(ymin,ymax);
}
