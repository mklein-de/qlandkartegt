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
#ifndef CMAPSELECTIONGARMIN_H
#define CMAPSELECTIONGARMIN_H

#include "IMapSelection.h"

#include <QVector>
#include <QMap>

class CMapSelectionGarmin : public IMapSelection
{
    public:
        CMapSelectionGarmin(QObject * parent);
        virtual ~CMapSelectionGarmin();

        void draw(QPainter& p, const QRect& rect);

        struct tile_t
        {
            QString filename;
            QVector<double> u;
            QVector<double> v;
            quint32 memSize;
        };

        struct map_t
        {
            QMap<QString, tile_t> tiles;
        };

        QMap<QString, map_t> maps;

};

#endif //CMAPSELECTIONGARMIN_H

