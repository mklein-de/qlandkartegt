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

#ifndef CTRACK_H
#define CTRACK_H

#include <QObject>
#include <QVector>
#include "CWpt.h"

class CTrack : public QObject
{
    Q_OBJECT
    public:
        CTrack(QObject * parent);
        virtual ~CTrack();

        struct pt_t {
            pt_t() : lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT), timestamp(0){}
            float lon;
            float lat;
            float ele;
            quint32 timestamp;
        };

    private:
        QString _key_;
        QString name;
        QString comment;
        QVector<pt_t> track;
};

#endif //CTRACK_H

