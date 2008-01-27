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

#include "CCreateMapOSM.h"
#include "CResources.h"
#include "GeoMath.h"

#include <QtGui>
#include <QtNetwork/QHttp>
#include <math.h>

#include <gdal_priv.h>

static char * osm_image_args[] = {
     "BLOCKXSIZE=256"
    ,"BLOCKYSIZE=256"
    ,"TILED=YES"
    ,"COMPRESS=DEFLATE"
    ,NULL
};


CCreateMapOSM::CCreateMapOSM(QWidget * parent)
    : QWidget(parent)
    , dataset(0)
    , band(0)
{
    setupUi(this);
    labelPath->setText(CResources::self().pathMaps);
    lineName->setText("testosm");

    connect(pushCreate, SIGNAL(clicked()), this, SLOT(slotCreate()));

    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    link = new QHttp(this);
    if(enableProxy){
        link->setProxy(url,port);
    }

    connect(link,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));


}

CCreateMapOSM::~CCreateMapOSM()
{
    if(dataset) delete dataset;
}

void CCreateMapOSM::slotCreate()
{
    QString filename = QDir(labelPath->text()).filePath(lineName->text());
    float lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;
    GPS_Math_Str_To_Deg(lineTopLeft->text(), lon1, lat1);
    GPS_Math_Str_To_Deg(lineBottomRight->text(), lon2, lat2);

    /// for 17th zoom level
    zoomlevel = 14;
    x1 = (lon1 + 180) * (1<<zoomlevel) / 360;
    x2 = (lon2 + 180) * (1<<zoomlevel) / 360;

    y1 = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoomlevel);
    y2 = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoomlevel);

    QString file = QString("http://tah.openstreetmap.org/Tiles/tile.php/%1/%2/%3.png").arg(zoomlevel).arg(x1).arg(y1);

    qDebug() << x1 << y1 << x2 << y2;

    qDebug() << (x2 - x1 + 1) * 256 << (y2 - y1 + 1) * 256;

    link->setHost("tah.openstreetmap.org");

    GDALDriverManager * drvman = GetGDALDriverManager();
    GDALDriver *        driver = drvman->GetDriverByName("GTiff");

    if(dataset) delete dataset;
    dataset = driver->Create("test.gtif",(x2 - x1 + 1) * 256,(y2 - y1 + 1) * 256,1,GDT_Byte,osm_image_args);
    band    = dataset->GetRasterBand(1);

    x = x1;
    y = y1;

    getNextTile();
}

void CCreateMapOSM::getNextTile()
{

    QUrl url;
    url.setPath(QString("/Tiles/tile.php/%1/%2/%3.png").arg(zoomlevel).arg(x).arg(y));
    qDebug() << url;
    link->get(url.toEncoded( ));
}

void CCreateMapOSM::slotRequestFinished(int id, bool error)
{
    qDebug() << "slotRequestFinished(" <<  id << "," << error << ")";

    QImage img1,img2;
    img1.loadFromData(link->readAll());
    img2 = img1.convertToFormat(QImage::Format_Indexed8);
//     img2.save("test.png");
    if(img1.format() == QImage::Format_Invalid){
        return;
    }

    qDebug() << img2.format();
    band->WriteBlock(x - x1, y - y1, img2.bits());

    if(++x > x2){
        x = x1;
        if(++y > y2){
            qDebug() << "done";
            dataset->FlushCache();
            delete dataset;
            dataset = 0;
            band    = 0;
            return;
        }
    }

    getNextTile();
}
