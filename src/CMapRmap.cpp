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
#include "CMapRmap.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "CCanvas.h"


#include <QtGui>

CMapRmap::CMapRmap(const QString &key, const QString &fn, CCanvas *parent)
: IMap(eRaster,key,parent)
, zoomFactor(0.0)
, needsRedrawOvl(true)
{
    filename = fn;

    quadraticZoom = theMainWindow->getCheckBoxQuadraticZoom();

    QFileInfo fi(fn);
    name = fi.fileName();
    name = name.left(name.size() - 5);

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    QByteArray charbuf(20,0);
    stream.readRawData(charbuf.data(), 19);

    if("CompeGPSRasterImage" != QString(charbuf))
    {
        qDebug() << QString(charbuf);
        QMessageBox::warning(0, tr("Error..."), tr("This is not a TwoNav RMAP file."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    quint32 tmp32;
    stream >> tmp32 >> tmp32 >> tmp32;
    stream >> xsize_px >> ysize_px;
    stream >> tmp32 >> tmp32;
    stream >> blockSizeX >> blockSizeY;

    ysize_px = -ysize_px;

    quint64 mapDataOffset;
    stream >> mapDataOffset;
    qDebug() << mapDataOffset << hex << (quint32)mapDataOffset;
    stream >> tmp32;


    qint32 nZoomLevels;
    stream >> nZoomLevels;
    qDebug() << nZoomLevels << hex << nZoomLevels;


    for(int i=0; i < nZoomLevels; i++)
    {
        level_t level;
        stream >> level.offsetLevel;
        qDebug() << level.offsetLevel << hex << (quint32)level.offsetLevel;
        levels << level;
    }

    for(int i=0; i<levels.size(); i++)
    {
        level_t& level = levels[i];
        file.seek(level.offsetLevel);

        stream >> level.width;
        stream >> level.height;
        stream >> level.xTiles;
        stream >> level.yTiles;

        qDebug() << level.width << level.height << level.xTiles << level.yTiles;

        for(int j=0; j<(level.xTiles * level.yTiles); j++)
        {
            quint64 offset;
            stream >> offset;
            level.offsetJpegs << offset;

            qDebug() << hex << (quint32) offset;
        }
    }


    file.seek(mapDataOffset);
    stream >> tmp32 >> tmp32;

    charbuf.resize(tmp32 + 1);
    charbuf.fill(0);
    stream.readRawData(charbuf.data(), tmp32);

    QPoint p0;
    QPoint p1;
    QPoint p2;
    QPoint p3;
    XY c0;
    XY c1;
    XY c2;
    XY c3;

    QString projection;
    QString datum;
    QStringList lines = QString(charbuf).split("\r\n");
    foreach(const QString& line, lines)
    {
        if(line.startsWith("Version="))
        {
            if(line.split("=")[1] != "2")
            {
                QMessageBox::warning(0, tr("Error..."), tr("Unknown version."), QMessageBox::Abort, QMessageBox::Abort);
                return;
            }
        }
        else if(line.startsWith("Projection="))
        {
            projection = line.split("=")[1];
        }
        else if(line.startsWith("Datum="))
        {
            datum = line.split("=")[1];
        }
        else if(line.startsWith("P0="))
        {
            QStringList vals = line.split("=")[1].split(",");
            if(vals.size() < 5)
            {
                QMessageBox::warning(0, tr("Error..."), tr("Failed to read reference point."), QMessageBox::Abort, QMessageBox::Abort);
                return;
            }

            p0 = QPoint(vals[0].toInt(), vals[1].toInt());
            c0.u = vals[3].toDouble() * DEG_TO_RAD;
            c0.v = vals[4].toDouble() * DEG_TO_RAD;

        }
        else if(line.startsWith("P1="))
        {
            QStringList vals = line.split("=")[1].split(",");
            if(vals.size() < 5)
            {
                QMessageBox::warning(0, tr("Error..."), tr("Failed to read reference point."), QMessageBox::Abort, QMessageBox::Abort);
                return;
            }

            p1 = QPoint(vals[0].toInt(), vals[1].toInt());
            c1.u = vals[3].toDouble() * DEG_TO_RAD;
            c1.v = vals[4].toDouble() * DEG_TO_RAD;
        }
        else if(line.startsWith("P2="))
        {
            QStringList vals = line.split("=")[1].split(",");
            if(vals.size() < 5)
            {
                QMessageBox::warning(0, tr("Error..."), tr("Failed to read reference point."), QMessageBox::Abort, QMessageBox::Abort);
                return;
            }

            p2 = QPoint(vals[0].toInt(), vals[1].toInt());
            c2.u = vals[3].toDouble() * DEG_TO_RAD;
            c2.v = vals[4].toDouble() * DEG_TO_RAD;
        }
        else if(line.startsWith("P3="))
        {
            QStringList vals = line.split("=")[1].split(",");
            if(vals.size() < 5)
            {
                QMessageBox::warning(0, tr("Error..."), tr("Failed to read reference point."), QMessageBox::Abort, QMessageBox::Abort);
                return;
            }

            p3 = QPoint(vals[0].toInt(), vals[1].toInt());
            c3.u = vals[3].toDouble() * DEG_TO_RAD;
            c3.v = vals[4].toDouble() * DEG_TO_RAD;
        }
        else
        {
            qDebug() << line;
        }
    }

    if(!projection.isEmpty() && !datum.isEmpty())
    {
        if(!setProjection(projection, datum))
        {
            QMessageBox::warning(0, tr("Error..."), tr("Unknown projection and datum (%1%2).").arg(projection).arg(datum), QMessageBox::Abort, QMessageBox::Abort);
            return;
        }
    }

    if(!pj_is_latlong(pjsrc))
    {
        pj_transform(pjtar, pjsrc, 1, 0, &c0.u, &c0.v, 0);
        pj_transform(pjtar, pjsrc, 1, 0, &c1.u, &c1.v, 0);
        pj_transform(pjtar, pjsrc, 1, 0, &c2.u, &c2.v, 0);
        pj_transform(pjtar, pjsrc, 1, 0, &c3.u, &c3.v, 0);
    }

    xref1  = c0.u;
    yref1  = c0.v;
    xref2  = c2.u;
    yref2  = c2.v;

    xscale = (c2.u - c0.u) / (p2.x() - p0.x());
    yscale = (c2.v - c0.v) / (p2.y() - p0.y());

    qDebug() << p0 << c0.u << c0.v;
    qDebug() << p2 << c2.u << c2.v;

    qDebug() << "map  width:" << xsize_px << "height:" << ysize_px;
    qDebug() << "tile width:" << blockSizeX << "height:" << blockSizeY;
    qDebug() << "scale x:  " << xscale << "y:" << yscale;

    x = xref1 + (xref2 - xref1);
    y = yref1 + (yref2 - yref1);

    QSettings cfg;
    cfg.beginGroup("rmap/maps");
    cfg.beginGroup(getKey());

    x = cfg.value("lon", x).toDouble();
    y = cfg.value("lat", y).toDouble();

    cfg.endGroup();
    cfg.endGroup();


    zoom(zoomidx);
}

CMapRmap::~CMapRmap()
{
    if(pjsrc) pj_free(pjsrc);

    QSettings cfg;
    cfg.beginGroup("rmap/maps");
    cfg.beginGroup(getKey());

    cfg.setValue("lon", x);
    cfg.setValue("lat", y);

    cfg.endGroup();
    cfg.endGroup();

}

bool CMapRmap::setProjection(const QString& projection, const QString& datum)
{
    QString projstr;
    if(projection.startsWith("2,"))
    {
        projstr += "+proj=merc";
    }

    if(datum == "WGS 84")
    {
        projstr += " +datum=WGS84";
    }

    pjsrc = pj_init_plus(projstr.toAscii().data());
    if(pjsrc == 0)
    {
        return false;
    }
    oSRS.importFromProj4(getProjection());
    char * ptr = pj_get_def(pjsrc,0);
    qDebug() << "rmap:" << ptr;


    return true;
}


void CMapRmap::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;

}

void CMapRmap::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}


void CMapRmap::move(const QPoint& old, const QPoint& next)
{
    if(pjsrc == 0) return;
    needsRedraw     = true;
    needsRedrawOvl  = true;

    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    emit sigChanged();

    setAngleNorth();
}

void CMapRmap::zoom(bool zoomIn, const QPoint& p0)
{
    qDebug() << "zoom" << zoomIn;

    XY p1;
    if(pjsrc == 0) return;

    needsRedraw     = true;
    needsRedrawOvl  = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    if(quadraticZoom->isChecked())
    {

        if(zoomidx > 1)
        {
            zoomidx = pow(2.0, ceil(log(zoomidx*1.0)/log(2.0)));
            zoomidx = zoomIn ? (zoomidx>>1) : (zoomidx<<1);
        }
        else
        {
            zoomidx += zoomIn ? -1 : 1;
        }
    }
    else
    {
        zoomidx += zoomIn ? -1 : 1;
    }
    // sigChanged will be sent at the end of this function
    blockSignals(true);
    zoom(zoomidx);

    // convert geo. coordinates back to point
    convertRad2Pt(p1.u, p1.v);

    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference point befor and after zoom
    xx += p1.u - p0.x();
    yy += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2M(xx, yy);
    x = xx;
    y = yy;
    blockSignals(false);
    emit sigChanged();
}

void CMapRmap::zoom(double lon1, double lat1, double lon2, double lat2)
{
    if(pjsrc == 0) return;

    needsRedraw     = true;
    needsRedrawOvl  = true;

    double u[3];
    double v[3];
    double dU, dV;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = (u[1] - u[0]) / xscale;
    dV = (v[0] - v[2]) / yscale;

    int z1 = fabs(dU / size.width());
    int z2 = fabs(dV / size.height());

    zoomFactor = (z1 > z2 ? z1 : z2)  + 1;
    if(quadraticZoom->isChecked())
    {
        zoomFactor = zoomidx = pow(2.0, ceil(log(zoomFactor)/log(2.0)));
    }
    else
    {
        zoomidx = zoomFactor;
    }

    double u_ = lon1 + (lon2 - lon1)/2;
    double v_ = lat1 + (lat2 - lat1)/2;
    convertRad2Pt(u_,v_);
    move(QPoint(u_,v_), rect.center());

    emit sigChanged();

    qDebug() << "zoom:" << zoomFactor;
}

void CMapRmap::zoom(qint32& level)
{
    if(pjsrc == 0) return;
    needsRedraw     = true;
    needsRedrawOvl  = true;

    // no level less than 1
    if(level < 1)
    {
        zoomFactor  = 1.0 / - (level - 2);
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = level;
    emit sigChanged();
    qDebug() << "zoom:" << zoomFactor;
}

void CMapRmap::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    if(pjsrc == 0) return;

    lon1 = xref1;
    lat1 = yref1;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);

    lon2 = xref2;
    lat2 = yref2;
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

}

void CMapRmap::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    if(pjsrc == 0) return;

    p1.u        = 0;
    p1.v        = 0;
    p2.u        = rect.width();
    p2.v        = rect.height();

    convertPt2Rad(p1.u, p1.v);
    convertPt2Rad(p2.u, p2.v);

    my_xscale   = xscale * zoomFactor;
    my_yscale   = yscale * zoomFactor;
}

void CMapRmap::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    // render map if necessary
    if(needsRedraw)
    {
        draw();
    }

    p.drawPixmap(0,0,pixBuffer);

    // render overlay
    if(!ovlMap.isNull() && !doFastDraw)
    {
        ovlMap->draw(size, needsRedrawOvl, p);
        needsRedrawOvl = false;
    }

    needsRedraw = false;

    if(CResources::self().showZoomLevel())
    {

        QString str;
        if(zoomFactor < 1.0)
        {
            str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
        }
        else
        {
            str = tr("Zoom level x%1").arg(zoomFactor);
        }

        p.setPen(Qt::white);
        p.setFont(QFont("Sans Serif",14,QFont::Black));

        p.drawText(9  ,23, str);
        p.drawText(10 ,23, str);
        p.drawText(11 ,23, str);
        p.drawText(9  ,24, str);
        p.drawText(11 ,24, str);
        p.drawText(9  ,25, str);
        p.drawText(10 ,25, str);
        p.drawText(11 ,25, str);

        p.setPen(Qt::darkBlue);
        p.drawText(10,24,str);
    }
}

void CMapRmap::draw()
{
    pixBuffer.fill(Qt::white);
}
