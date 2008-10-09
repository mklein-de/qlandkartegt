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

#include "CMapSearchThread.h"
#include "CMapQMAP.h"
#include "CImage.h"

#include <QtGui>

CMapSearchThread::CMapSearchThread(QObject * parent)
: QThread(parent)
, threshold(0)
, zoomlevel(1)
{
    mask = new CImage(this);
}

CMapSearchThread::~CMapSearchThread()
{

}

void CMapSearchThread::start(const int th, const QImage& m, const CMapSelection& ms)
{
    if(isRunning()) return;

    threshold   = th;
    area        = ms;
    mask->setPixmap(m);

    qDebug() << area.focusedMap << area.mapkey << area.lon1 << area.lat1 << area.lon2 << area.lat2;

    QThread::start();
}

void CMapSearchThread::run()
{
    qDebug() << "thread start...";
    emit sigProgress(tr(""), 0);


    QList<QPoint> syms;
    QSize size = QSize(1024,1024) + mask->mask().size();

    int n,m;
    QImage buffer(size, QImage::Format_ARGB32);
    double x1 = area.lon1;
    double y1 = area.lat1;
    double x2 = area.lon2;
    double y2 = area.lat2;

    double u1 = area.lon1;
    double v1 = area.lat1;
    double u2 = area.lon2;
    double v2 = area.lat2;

    CMapQMAP map("", area.mapkey, 0);
    map.resize(size);
    map.zoom(zoomlevel);

    map.convertRad2Pt(x1, y1);
    map.convertRad2Pt(x2, y2);

    map.convertRad2M(u1, v1);
    map.convertRad2M(u2, v2);
    QRect rectArea(QPoint(u1,v1), QPoint(u2,v2));

    double w = x2 - x1;
    double h = y2 - y1;

    int maxN = ceil(h/1024);
    int maxM = ceil(w/1024);

    map.move(QPoint(x1,y1), QPoint(0, 0));

    symbols.clear();

    for(n = 0; n < maxN; ++n){

        for(m = 0; m < maxM; ++m){
            qDebug() << n << m;

            QPainter p(&buffer);
            map.draw(p);

            CImage img(map.getBuffer());
            img.binarize(threshold);
            img.findSymbol(syms, *mask);

            QPoint sym;
            foreach(sym, syms){
                double x = sym.x();
                double y = sym.y();
                map.convertPt2M(x,y);
                if(rectArea.contains(x,y)){
                    symbols << QPoint(x,y);
                }
            }

            emit sigProgress(tr("Parsing..."), (100 * (n * maxM + m + 1)) / (maxN * maxM));

            map.move(QPoint(1024, 0), QPoint(0,0));
        }
        map.move(QPoint(-(maxM * 1024), 1024), QPoint(0,0));
    }

    emit sigProgress(tr("Done!"), 100);
    qDebug() << "...thread stop";
}

