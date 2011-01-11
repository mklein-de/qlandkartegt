/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "CMapJnx.h"
#include "CResources.h"
#include <QtGui>

#define MAX_IDX_ZOOM 26
#define MIN_IDX_ZOOM 0


CMapJnx::scale_t CMapJnx::scales[] =
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

CMapJnx::CMapJnx(const QString& key, const QString& fn, CCanvas * parent)
: IMap(eRaster,key,parent)
, xscale(1.0)
, yscale(-1.0)
{
    hdr_t hdr;

    filename = fn;

    QFileInfo fi(fn);
    name = fi.fileName();
    name = name.left(name.size() - 4);

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> hdr.version;            // byte 00000000..00000003
    stream >> hdr.devid;              // byte 00000004..00000007
    stream >> hdr.top;              // byte 00000008..0000000B
    stream >> hdr.right;              // byte 0000000C..0000000F
    stream >> hdr.bottom;              // byte 00000010..00000013
    stream >> hdr.left;              // byte 00000014..00000017
    stream >> hdr.details;            // byte 00000018..0000001B
    stream >> hdr.expire;             // byte 0000001C..00000023
    stream >> hdr.crc;                // byte 00000024..00000027
    stream >> hdr.signature;          // byte 00000028..0000002B
    stream >> hdr.signature_offset;   // byte 0000002C..0000002F

    lat1 = hdr.top * 180.0 / 0x7FFFFFFF;
    lat2 = hdr.bottom * 180.0 / 0x7FFFFFFF;
    lon1 = hdr.left * 180.0 / 0x7FFFFFFF;
    lon2 = hdr.right * 180.0 / 0x7FFFFFFF;

    qDebug() << filename;
    qDebug() << hex << "Version:" << hdr.version << "DevId" <<  hdr.devid;
    qDebug() << lon1 << lat1 << lon2 << lat2;
    qDebug() << hex <<  hdr.top <<  hdr.right <<  hdr.bottom <<  hdr.left;
    qDebug() << hex << "Details:" <<  hdr.details << "Expire:" <<  hdr.expire << "CRC:" <<  hdr.crc ;
    qDebug() << hex << "Signature:" <<  hdr.signature << "Offset:" <<  hdr.signature_offset;

    qDebug() << "Levels:";
    levels.resize(hdr.details);
    for(quint32 i = 0; i < hdr.details; i++)
    {
        level_t& level = levels[i];
        stream >> level.nTiles >> level.offset >> level.scale;

        qDebug() << i << hex << level.nTiles << level.offset << level.scale;
    }

    for(quint32 i = 0; i < hdr.details; i++)
    {

        level_t& level = levels[i];
        const quint32 M = level.nTiles;
        file.seek(level.offset);

        level.tiles.resize(M);

        for(quint32 m = 0; m < M; m++)
        {

            qint32 top, right, bottom, left;
            tile_t& tile = level.tiles[m];

            stream >> top >> right >> bottom >> left;
            stream >> tile.width >> tile.height >> tile.size >> tile.offset;

            tile.area.setTop(top * 180.0 / 0x7FFFFFFF);
            tile.area.setRight(right * 180.0 / 0x7FFFFFFF);
            tile.area.setBottom(bottom * 180.0 / 0x7FFFFFFF);
            tile.area.setLeft(left * 180.0 / 0x7FFFFFFF);
        }
    }



    pjsrc   = pj_init_plus("+proj=merc +ellps=WGS84 +datum=WGS84 +no_defs");

    x = lon1 * DEG_TO_RAD;
    y = lat2 * DEG_TO_RAD;

    pj_transform(pjtar, pjsrc,1,0,&x,&y,0);

    QSettings cfg;
    cfg.beginGroup("birdseye/maps");
    cfg.beginGroup(name);
    zoomidx = cfg.value("zoomidx",23).toInt();
    x       = cfg.value("x",x).toDouble();
    y       = cfg.value("y",y).toDouble();
    cfg.endGroup();
    cfg.endGroup();

    zoom(zoomidx);

    setAngleNorth();
}

CMapJnx::~CMapJnx()
{
    if(pjsrc) pj_free(pjsrc);

    QSettings cfg;
    cfg.beginGroup("birdseye/maps");
    cfg.beginGroup(name);
    cfg.setValue("zoomidx",zoomidx);
    cfg.setValue("x",x);
    cfg.setValue("y",y);
    cfg.endGroup();
    cfg.endGroup();

}

void CMapJnx::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}

void CMapJnx::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}

void CMapJnx::move(const QPoint& old, const QPoint& next)
{
    XY p2;
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

void CMapJnx::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

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

    XY p2;
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

void CMapJnx::zoom(double lon1, double lat1, double lon2, double lat2)
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

void CMapJnx::zoom(qint32& level)
{
    needsRedraw = true;

    zoomidx = level;
    if(zoomidx < MIN_IDX_ZOOM) zoomidx = MIN_IDX_ZOOM;
    if(zoomidx > MAX_IDX_ZOOM) zoomidx = MAX_IDX_ZOOM;
    zoomFactor = scales[zoomidx].qlgtScale;

    emit sigChanged();
}

void CMapJnx::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1 * DEG_TO_RAD;
    lon2 = this->lon2 * DEG_TO_RAD;
    lat1 = this->lat2 * DEG_TO_RAD;
    lat2 = this->lat1 * DEG_TO_RAD;

}

void CMapJnx::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    if(needsRedraw)
    {
        draw();
    }


    p.drawImage(0,0,buffer);

    if(!ovlMap.isNull() && !doFastDraw)
    {
        ovlMap->draw(size, needsRedraw, p);
    }

    needsRedraw = false;
}

void CMapJnx::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
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


qint32 CMapJnx::zlevel2idx(quint32 l)
{
    quint32 index   = -1;
    quint32 scale   = levels[0].scale;

    quint32 s1 = scales[l].jnxScale;
    quint32 s2 = (l == 0) ? 0x7FFFFFFF : scales[l - 1].jnxScale;

    qDebug() << "-----------";

    for(int i = 0; i < levels.size(); i++)
    {
        scale = levels[i].scale;
        qDebug() << s1 << scale << s2;

        if(s1 <= scale && scale < s2)
        {
            return i;
        }
    }

    if(s1 < scale && s2 < scale)
    {
        return levels.size() - 1;
    }

    return index;
}

void CMapJnx::draw()
{
    if(pjsrc == 0) return IMap::draw();

    buffer.fill(Qt::white);
    QPainter p(&buffer);

    p.setBrush(Qt::NoBrush);


    double u1 = 0;
    double v1 = size.height();
    double u2 = size.width();
    double v2 = 0;

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

    qint32 level = zlevel2idx(zoomidx);

    qDebug() << "use level" << level;

    if(level < 0)
    {
        double u1 = lon1 * DEG_TO_RAD;
        double u2 = lon2 * DEG_TO_RAD;
        double v2 = lat2 * DEG_TO_RAD;
        double v1 = lat1 * DEG_TO_RAD;

        convertRad2Pt(u1,v1);
        convertRad2Pt(u2,v2);

        QRectF r;
        r.setLeft(u1);
        r.setRight(u2);
        r.setTop(v2);
        r.setBottom(v1);

        p.setPen(QPen(Qt::darkBlue,2));
        p.setBrush(QBrush(QColor(230,230,255,100) ));
        p.drawRect(r);
        return;
    }


    QByteArray data(1024*1024*4,0);
    data[0] = 0xFF;
    data[1] = 0xD8;
    char * pData = data.data() + 2;

    QImage image;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    quint32 cnt = 0;
    double m_px = 0.0;
    double d_px = 0.0;
    QVector<tile_t>& tiles = levels[level].tiles;
    const quint32 M = tiles.size();
    for(quint32 m = 0; m < M; m++)
    {
        tile_t& tile = tiles[m];

        if(viewport.intersects(tile.area))
        {
            {
                double u1 = tile.area.left() * DEG_TO_RAD;
                double u2 = tile.area.right() * DEG_TO_RAD;
                double v2 = tile.area.top() * DEG_TO_RAD;
                double v1 = tile.area.bottom() * DEG_TO_RAD;

                convertRad2M(u1,v1);
                convertRad2M(u2,v2);

                m_px += (u2 - u1) / tile.width;
                d_px += (tile.area.right() - tile.area.left()) / tile.width;
                cnt++;
            }

            double u1 = tile.area.left() * DEG_TO_RAD;
            double u2 = tile.area.right() * DEG_TO_RAD;
            double v2 = tile.area.top() * DEG_TO_RAD;
            double v1 = tile.area.bottom() * DEG_TO_RAD;

            convertRad2Pt(u1,v1);
            convertRad2Pt(u2,v2);

            QRectF r;
            r.setLeft(u1);
            r.setRight(u2);
            r.setTop(v2);
            r.setBottom(v1);

            file.seek(tile.offset);
            file.read(pData, tile.size);
            image.loadFromData(data);

            p.drawImage(r, image.scaled(r.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    qDebug() << m_px/cnt << "m/px";
    qDebug() << d_px/cnt << "\260/px";
}


