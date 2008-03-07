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
#ifndef CCREATEMAPGEOTIFF_H
#define CCREATEMAPGEOTIFF_H

#include <QWidget>
#include "ui_ICreateMapGeoTiff.h"

class CCreateMapGeoTiff : public QWidget, private Ui::ICreateMapGeoTiff
{
    Q_OBJECT;
    public:
        CCreateMapGeoTiff(QWidget * parent);
        virtual ~CCreateMapGeoTiff();

        static CCreateMapGeoTiff * self(){return m_self;}

        struct refpt_t
        {
            refpt_t() : x(0), y(0), item(0){}

            double x;
            double y;

            QTreeWidgetItem * item;
        };

        enum columns_e
        {
              eNum = 0
            , eLabel = 1
            , eLon = 2
            , eLat = 3
            , eX = 4
            , eY = 5
        };

        QMap<quint32,refpt_t>& getRefPoints(){return refpts;}

    private slots:
        void slotOpenFile();
        void slotAddRef();
        void slotDelRef();

    private:
        static CCreateMapGeoTiff * m_self;

        void enableStep2();
        void enableStep3();

        QMap<quint32,refpt_t> refpts;
        quint32 refcnt;
};
#endif                           //CCREATEMAPGEOTIFF_H
