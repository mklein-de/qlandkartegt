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
#ifndef CMAPSEARCHTHREAD_H
#define CMAPSEARCHTHREAD_H

#include <QThread>
#include <QPixmap>
#include <QList>
#include <QPoint>
#include "CMapSelection.h"

class CImage;

class CMapSearchThread : public QThread
{
    Q_OBJECT;
    public:
        CMapSearchThread(QObject * parent);
        virtual ~CMapSearchThread();

        void start(const int threshold, const QImage& mask, const CMapSelection& mapsel);

        const QList<QPoint>& getLastResult(){return symbols;}

        signals:
        void sigProgress(const QString& status, const int progress);

    protected:
        void run();

    private:
        QString mapfilename;

        int threshold;
        CMapSelection area;
        CImage * mask;
        qint32 zoomlevel;
        QList<QPoint> symbols;

};
#endif                           //CMAPSEARCHTHREAD_H
