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
#ifndef CWPT_H
#define CWPT_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QList>

/// waypoint object
class CWpt : public QObject
{
    Q_OBJECT
    public:
        CWpt(QObject * parent);
        virtual ~CWpt();

        const QString& key();
        const QString filename();
        enum id_e {eEnd,eBase,eImage};

    private:
        void genKey();

        // eBase: base information
    private:
        friend QDataStream& operator >>(QDataStream& s, CWpt& wpt);
        friend QDataStream& operator <<(QDataStream& s, CWpt& wpt);
        QString _key_;

    public:
        quint32 sticky;
        quint32 timestamp;
        QString icon;
        QString name;
        QString comment;
        float   lat;
        float   lon;
        float   altitude;
        float   proximity;

        // images
    private:
        struct image_t
        {
            quint32 offset;
            QString name;
            QPixmap preview;
        };
        QList<image_t> images;
};

QDataStream& operator >>(QDataStream& s, CWpt& wpt);
QDataStream& operator <<(QDataStream& s, CWpt& wpt);

#endif //CWPT_H

