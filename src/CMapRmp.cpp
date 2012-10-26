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
    {80000,  2083334 }
    ,{5000,   1302084 }
    ,{3000,   781250  }
    ,{2000,   520834  }
    ,{1200,   312500  }
    ,{800,    208334  }
    ,{500,    130209  }
    ,{300,    78125   }
    ,{200,    52084   }
    ,{120,    31250   }
    ,{80.0,   20834   }
    ,{50.0,   13021   }
    ,{30.0,   7813    }
    ,{20.0,   5209    }
    ,{12.0,   3125    }
    ,{8.00,   2084    }
    ,{5.00,   1303    }
    ,{3.00,   782     }
    ,{2.00,   521     }
    ,{1.20,   313     }
    ,{0.80,   209     }
    ,{0.50,   131     }
    ,{0.30,   79      }
    ,{0.20,   52      }
    ,{0.12,   32      }
    ,{0.08,   21      }
    ,{0.05,   14      }
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
            qDebug() << "prev:" << hex << tmp32;
            otherNodes << (level.tlm.offset + 256 + tmp32);
        }

        for(i = 0; i<99; i++)
        {
            stream >> tmp32;
            if(tmp32)
            {
                qDebug() << "next:" << hex << tmp32;
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


    qDebug() << "read tiles from:" << hex << quint32(stream.device()->pos());
    stream >> tilesSubtree >> node.nTiles >> lastNode;

    qDebug() << "tiles sub:" << tilesSubtree << "tiles node:" << node.nTiles << "is last node" << lastNode;

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

void CMapRmp::draw(QPainter& p)
{

    double u1 = xref1 * DEG_TO_RAD;
    double v1 = yref1 * DEG_TO_RAD;
    double u2 = xref2 * DEG_TO_RAD;
    double v2 = yref2 * DEG_TO_RAD;

    convertRad2Pt(u1,v1);
    convertRad2Pt(u2,v2);

    p.setPen(Qt::black);
    p.drawRect(u1,v1, u2-u1, v2-v1);

    p.setPen(Qt::red);
    foreach(const node_t& node, levels["magella0"].tlm.nodes)
    {
        u1 = node.bbox.left() * DEG_TO_RAD;
        v1 = node.bbox.top() * DEG_TO_RAD;
        u2 = node.bbox.right() * DEG_TO_RAD;
        v2 = node.bbox.bottom() * DEG_TO_RAD;

        convertRad2Pt(u1,v1);
        convertRad2Pt(u2,v2);

        p.drawRect(u1,v1, u2-u1, v2-v1);

    }

    p.setPen(Qt::green);
    foreach(const node_t& node, levels["magella1"].tlm.nodes)
    {
        u1 = node.bbox.left() * DEG_TO_RAD;
        v1 = node.bbox.top() * DEG_TO_RAD;
        u2 = node.bbox.right() * DEG_TO_RAD;
        v2 = node.bbox.bottom() * DEG_TO_RAD;

        convertRad2Pt(u1,v1);
        convertRad2Pt(u2,v2);

        p.drawRect(u1,v1, u2-u1, v2-v1);

    }

    p.setPen(Qt::blue);
    foreach(const node_t& node, levels["magella2"].tlm.nodes)
    {
        u1 = node.bbox.left() * DEG_TO_RAD;
        v1 = node.bbox.top() * DEG_TO_RAD;
        u2 = node.bbox.right() * DEG_TO_RAD;
        v2 = node.bbox.bottom() * DEG_TO_RAD;

        convertRad2Pt(u1,v1);
        convertRad2Pt(u2,v2);

        p.drawRect(u1,v1, u2-u1, v2-v1);

    }

}
