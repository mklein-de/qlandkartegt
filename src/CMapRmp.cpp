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
#include "CMapRmp.h"
#include "CSettings.h"
#include "CMainWindow.h"

#include <QtGui>

#define MAX_IDX_ZOOM 26
#define MIN_IDX_ZOOM 0

CMapRmp::scale_t CMapRmp::scales[] =
{
{80000, 2.2400e+02 }
,{5000,  1.4000e+01 }
,{3000,  8.4000e+00 }
,{2000,  5.6000e+00 }
,{1200,  3.3600e+00 }
,{800,   2.2400e+00 }
,{500,   1.4000e+00 }
,{300,   8.4000e-01 }
,{200,   5.6000e-01 }
,{120,   3.3600e-01 }
,{80.0,  2.2400e-01 }
,{50.0,  1.4000e-01 }
,{30.0,  8.4000e-02 }
,{20.0,  5.6000e-02 }
,{12.0,  3.3600e-02 }
,{8.00,  2.2400e-02 }
,{5.00,  1.4000e-02 }
,{3.00,  8.4000e-03 }
,{2.00,  5.6000e-03 }
,{1.20,  3.3600e-03 }
,{0.80,  2.2400e-03 }
,{0.50,  1.4000e-03 }
,{0.30,  8.4000e-04 }
,{0.20,  5.6000e-04 }
,{0.12,  3.3600e-04 }
,{0.08,  2.2400e-04 }
,{0.05,  1.4000e-04 }
};


CMapRmp::CMapRmp(const QString &key, const QString &fn, CCanvas *parent)
    : IMap(eRaster,key,parent)
    , xref1(180.0)
    , yref1(-90)
    , xref2(-180)
    , yref2(90)
    , xscale(1.0)
    , yscale(-1.0)

{
    int i;

    for(i = 0; i <= MAX_IDX_ZOOM; i++)
    {
        scales[i].tileYScale = scales[i].qlgtScale * 0.0014/1.2;
    }

    qint32 tmp32;
    quint64 offset;
    QByteArray buffer(30,0);
    filename = fn;

    QFileInfo fi(fn);
    name = fi.baseName();

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> tmp32;
    offset = tmp32 * 24 + 10;

    file.seek(offset);
    stream.readRawData(buffer.data(), 29);

    if("MAGELLAN" != QString(buffer))
    {
        QMessageBox::warning(0, tr("Error..."), tr("This is not a Magellan RMP file."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    // read the directory section
    file.seek(8);
    for(i = 0; i < tmp32; i++)
    {
        dir_entry_t entry;

        buffer.fill(0);
        stream.readRawData(buffer.data(), 9);
        entry.name = buffer;
        buffer.fill(0);
        stream.readRawData(buffer.data(), 7);
        entry.extension = buffer;
        stream >> entry.offset >> entry.length;
        directory << entry;

        if(entry.extension == "tlm")
        {
            level_t& level = levels[entry.name];
            level.tlm = entry;
        }

        if(entry.extension == "a00")
        {
            level_t& level = levels[entry.name];
            level.a00 = entry;
        }
    }

    foreach(const dir_entry_t& entry, directory)
    {
        qDebug() << entry.name << "." << entry.extension << hex << entry.offset << entry.length;
    }


    // read all information about the levels
    QStringList keys = levels.keys();
    qSort(keys);
    foreach(const QString& key, keys)
    {
        double tileLeft, tileTop, tileRight, tileBottom;
        quint32 firstBlockOffset;
        level_t& level = levels[key];
        file.seek(level.tlm.offset);
        stream >> tmp32 >> level.tlm.tileCount >> level.tlm.tileXSize >> level.tlm.tileYSize;
        stream >> tmp32 >> level.tlm.tileHeight >> level.tlm.tileWidth >> tileLeft >> tileTop >> tileRight >> tileBottom;

        tileTop     = -tileTop;
        tileBottom  = -tileBottom;

        if(tileLeft   < xref1) xref1  = tileLeft;
        if(tileTop    > yref1) yref1  = tileTop;
        if(tileRight  > xref2) xref2  = tileRight;
        if(tileBottom < yref2) yref2  = tileBottom;

        level.tlm.bbox = QRectF(QPointF(tileLeft, tileTop), QPointF(tileRight, tileBottom));

        qDebug() << "--------------";
        qDebug() << level.tlm.name;
        qDebug() << level.tlm.tileCount << level.tlm.tileXSize << level.tlm.tileYSize;
        qDebug() << level.tlm.tileHeight << level.tlm.tileWidth << level.tlm.bbox.topLeft() << level.tlm.bbox.bottomRight();


        //start 1st node
        file.seek(level.tlm.offset + 256);
        stream >> tmp32 >> tmp32 >> firstBlockOffset; //(tlm.offset + 256 + firstBlockOffset)
        file.seek(level.tlm.offset + 256 + firstBlockOffset);

        readTLMNode(stream, level.tlm);

        QList<quint32> otherNodes;

        stream >> tmp32;
        if(tmp32)
        {
//            qDebug() << "prev:" << hex << tmp32;
            otherNodes << (level.tlm.offset + 256 + tmp32);
        }

        for(i = 0; i<99; i++)
        {
            stream >> tmp32;
            if(tmp32)
            {
//                qDebug() << "next:" << hex << tmp32;
                otherNodes << (level.tlm.offset + 256 + tmp32);
            }
        }

        foreach(quint32 offset, otherNodes)
        {
            file.seek(offset);
            readTLMNode(stream, level.tlm);
        }
    }

    pjsrc = pj_init_plus("+proj=merc +ellps=WGS84 +datum=WGS84 +units=m +no_defs +towgs84=0,0,0");
    oSRS.importFromProj4(getProjection());

    xref1 *= DEG_TO_RAD;
    yref1 *= DEG_TO_RAD;
    xref2 *= DEG_TO_RAD;
    yref2 *= DEG_TO_RAD;

    convertRad2M(xref1, yref1);
    convertRad2M(xref2, yref2);

    qDebug() << "xref1:" << xref1 << "yref1:" << yref1;
    qDebug() << "xref2:" << xref2 << "yref2:" << yref2;

    SETTINGS;
    cfg.beginGroup("magellan/maps");
    cfg.beginGroup(name);
    zoomidx = cfg.value("zoomidx",23).toInt();
    x       = cfg.value("x",x).toDouble();
    y       = cfg.value("y",y).toDouble();
    cfg.endGroup();
    cfg.endGroup();

    zoom(zoomidx);

    setAngleNorth();

    theMainWindow->getCheckBoxQuadraticZoom()->hide();
}

CMapRmp::~CMapRmp()
{
    midU = rect.center().x();
    midV = rect.center().y();
    convertPt2Rad(midU, midV);

    if(pjsrc) pj_free(pjsrc);

    SETTINGS;
    cfg.beginGroup("magellan/maps");
    cfg.beginGroup(name);
    cfg.setValue("zoomidx",zoomidx);
    cfg.setValue("x",x);
    cfg.setValue("y",y);
    cfg.endGroup();
    cfg.endGroup();

    theMainWindow->getCheckBoxQuadraticZoom()->show();

}

void CMapRmp::readTLMNode(QDataStream& stream, tlm_t& tlm)
{
    int i;
    node_t node;
    float tileLeft, tileTop, tileRight, tileBottom;
    quint32 tmp32, tilesSubtree;
    quint16 lastNode;


//    qDebug() << "read tiles from:" << hex << quint32(stream.device()->pos());
    stream >> tilesSubtree >> node.nTiles >> lastNode;

//    qDebug() << "tiles sub:" << tilesSubtree << "tiles node:" << node.nTiles << "is last node" << lastNode;

    tileLeft    =  180.0;
    tileTop     = -90.0;
    tileRight   = -180.0;
    tileBottom  =  90.0;

    for(i=0; i < 99; i++)
    {
        tile_t& tile = node.tiles[i];
        float lon, lat;
        qint32 x,y;
        stream >> x >> y >> tmp32 >> tile.offset;

        lon =   x * tlm.tileWidth - 180.0;
        lat = -(y * tlm.tileHeight - 90.0);

        tile.bbox = QRectF(lon, lat, tlm.tileWidth, -tlm.tileHeight);

        if(i < node.nTiles)
        {
            if(tile.bbox.left()   < tileLeft)   tileLeft   = tile.bbox.left();
            if(tile.bbox.top()    > tileTop)    tileTop    = tile.bbox.top();
            if(tile.bbox.right()  > tileRight)  tileRight  = tile.bbox.right();
            if(tile.bbox.bottom() < tileBottom) tileBottom = tile.bbox.bottom();
        }
        //qDebug() << tile.bbox.topLeft() << tile.bbox.bottomRight() << tile.offset;
    }
    node.bbox = QRectF(QPointF(tileLeft, tileTop), QPointF(tileRight, tileBottom));
    tlm.nodes << node;
}


void CMapRmp::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}

void CMapRmp::convertM2Pt(double& u, double& v)
{
    u = floor((u - x) / (xscale * zoomFactor) + 0.5);
    v = floor((v - y) / (yscale * zoomFactor) + 0.5);
}

void CMapRmp::move(const QPoint& old, const QPoint& next)
{
    projXY p2;
    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2M(p2.u, p2.v);
    x = p2.u;
    y = p2.v;

    needsRedraw = true;
    emit sigChanged();

    setAngleNorth();
}

void CMapRmp::zoom(bool zoomIn, const QPoint& p0)
{
    projXY p1;

    needsRedraw = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? +1 : -1;
    // sigChanged will be sent at the end of this function
    blockSignals(true);
    zoom(zoomidx);

    // convert geo. coordinates back to point
    IMap::convertRad2Pt(p1.u, p1.v);

    projXY p2;
    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2M(p2.u, p2.v);
    x = p2.u;
    y = p2.v;

    blockSignals(false);
    emit sigChanged();
}

void CMapRmp::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;

    needsRedraw = true;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = u[1] - u[0];
    dV = v[2] - v[0];

    for(int i = MAX_IDX_ZOOM; i >= MIN_IDX_ZOOM; --i)
    {

        double z    = scales[i].qlgtScale;
        double pxU  = dU / (+1.0 * z);
        double pxV  = dV / (-1.0 * z);

        if(isLonLat())
        {
            pxU /= xscale;
            pxV /= yscale;
        }

        if((fabs(pxU) < size.width()) && (fabs(pxV) < size.height()))
        {
            zoomFactor  = z;
            zoomidx     = i;
            double u = lon1 + (lon2 - lon1)/2;
            double v = lat1 + (lat2 - lat1)/2;
            IMap::convertRad2Pt(u,v);
            move(QPoint(u,v), rect.center());
            return;
        }
    }
}

void CMapRmp::zoom(qint32& level)
{
    needsRedraw = true;

    zoomidx = level;
    if(zoomidx < MIN_IDX_ZOOM) zoomidx = MIN_IDX_ZOOM;
    if(zoomidx > MAX_IDX_ZOOM) zoomidx = MAX_IDX_ZOOM;
    zoomFactor = scales[zoomidx].qlgtScale;

    qDebug() << "zoom:" << zoomFactor << level;

    emit sigChanged();
}

void CMapRmp::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    if(pjsrc == 0) return;

    lon1 = xref1;
    lat1 = yref1;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);

    lon2 = xref2;
    lat2 = yref2;
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);


}

void CMapRmp::getArea_n_Scaling(projXY& p1, projXY& p2, float& my_xscale, float& my_yscale)
{
    p1.u = x;
    p1.v = y;
    convertM2Rad(p1.u, p1.v);

    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    p2.u += size.width();
    p2.v += size.height();

    convertPt2Rad(p2.u, p2.v);

    my_xscale   = xscale*zoomFactor;
    my_yscale   = yscale*zoomFactor;
}


const QString CMapRmp::zlevel2idx(quint32 zl)
{
    QString keyLevel;
    double actScale = scales[zl].tileYScale;

//    qDebug() << "-----------" << zl;
    QStringList keys = levels.keys();
    qSort(keys);
    foreach(const QString& key, keys)
    {
        level_t& level = levels[key];

//        qDebug() << level.tlm.tileHeight << actScale;
        if(actScale <= level.tlm.tileHeight)
        {
            keyLevel = key;
        }

    }

//    qDebug() << keyLevel;
    return keyLevel;
}

void CMapRmp::draw(QPainter& p)
{
    QImage img;
    QRectF viewport;
    double u1, v1, u2, v2;

    u1 = 0;
    v1 = size.height();
    u2 = size.width();
    v2 = 0;

    convertPt2Rad(u1,v1);
    convertPt2Rad(u2,v2);

    u1 *= RAD_TO_DEG;
    v1 *= RAD_TO_DEG;
    u2 *= RAD_TO_DEG;
    v2 *= RAD_TO_DEG;

    viewport.setTop(v2);
    viewport.setRight(u2);
    viewport.setBottom(v1);
    viewport.setLeft(u1);

    level_t& level = levels[zlevel2idx(zoomidx)];

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    foreach(const node_t& node, level.tlm.nodes)
    {
        foreach(const tile_t& tile, node.tiles)
        {
            if(!viewport.intersects(tile.bbox))
            {
                continue;
            }

            u1 = tile.bbox.left()   * DEG_TO_RAD;
            v1 = tile.bbox.top()    * DEG_TO_RAD;
            u2 = tile.bbox.right()  * DEG_TO_RAD;
            v2 = tile.bbox.bottom() * DEG_TO_RAD;

            convertRad2Pt(u1,v1);
            convertRad2Pt(u2,v2);

            file.seek(level.a00.offset + tile.offset + 4);

            img.load(&file,"JPG");
            if(img.isNull())
            {
                continue;
            }

            p.drawImage(u1 + 0.5,v1 + 0.5,img.scaled(u2 - u1  + 0.5, v2 - v1 + 0.5,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
        }
    }
    file.close();

}
