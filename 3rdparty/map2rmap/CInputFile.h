/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CINPUTFILE_H
#define CINPUTFILE_H

#include <QString>
#include <gdal_priv.h>
#include <projects.h>


class CInputFile
{
    public:
        CInputFile(const QString& filename);
        virtual ~CInputFile();

        double getXScale(){return xscale;}

        void setLevels(quint32 l);
        quint32 getLevels(){return levels;}
    private:
        QString filename;

        PJ * pj;
        GDALDataset * dataset;
        QString compeProj;
        QString compeDatum;

        quint32 width;
        quint32 height;

        double xscale;
        double yscale;
        double xref1;
        double yref1;

        int levels;
};

#endif //CINPUTFILE_H

