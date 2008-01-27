/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CCREATEMAPOSM_H
#define CCREATEMAPOSM_H

#include <QWidget>

#include "ui_ICreateMapOSM.h"

class QHttp;
class GDALDataset;
class GDALRasterBand;

class CCreateMapOSM : public QWidget, private Ui::ICreateMapOSM
{
    Q_OBJECT
    public:
        CCreateMapOSM(QWidget * parent);
        virtual ~CCreateMapOSM();

    private slots:
        void slotCreate();
        void slotRequestFinished(int , bool error);

    private:
        void getNextTile();

        QHttp * link;

        int zoomlevel;
        int x1;
        int x2;
        int y1;
        int y2;
        int x;
        int y;

        GDALDataset * dataset;
        GDALRasterBand * band;
};

#endif //CCREATEMAPOSM_H

