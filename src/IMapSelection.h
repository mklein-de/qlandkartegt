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
#ifndef IMAPSELECTION_H
#define IMAPSELECTION_H

#include <QString>
#include <QObject>
class QPainter;
class QRect;

class IMapSelection : public QObject
{
    Q_OBJECT;
    public:
        enum type_e {eNone, eRaster, eGarmin};

        IMapSelection(type_e type, QObject * parent) : QObject(parent), type(type), lon1(0), lat1(0), lon2(0), lat2(0){}
        virtual ~IMapSelection(){}

        void operator=(const IMapSelection& ms)
        {
            key         = ms.key;
            mapkey      = ms.mapkey;
            description = ms.description;

            lon1        = ms.lon1;
            lat1        = ms.lat1;
            lon2        = ms.lon2;
            lat2        = ms.lat2;
        }

        virtual void draw(QPainter& p, const QRect& rect){}

        virtual bool isEmpty(){return false;}

        static QString focusedMap;

        type_e type;
        QString key;
        QString mapkey;                

        double lon1;             ///< top left longitude [rad]
        double lat1;             ///< top left latitude [rad]
        double lon2;             ///< bottom right longitude [rad]
        double lat2;             ///< bottom right latitude [rad]

        virtual QString getDescription(){return description;}
        virtual void setDescription(const QString& desc){description = desc;}

    protected:
        QString description;
};
#endif                           //IMAPSELECTION_H
