/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CCreateMapOSM.cpp

  Module:

  Description:

  Created:     01/26/2008

  (C) 2008


**********************************************************************************************/

#include "CCreateMapOSM.h"
#include "CResources.h"
#include "GeoMath.h"

#include <QtGui>
#include <math.h>

CCreateMapOSM::CCreateMapOSM(QWidget * parent)
    : QWidget(parent)

{
    setupUi(this);
    labelPath->setText(CResources::self().pathMaps);
    lineName->setText("testosm");

    connect(pushCreate, SIGNAL(clicked()), this, SLOT(slotCreate()));

}

CCreateMapOSM::~CCreateMapOSM()
{

}

void CCreateMapOSM::slotCreate()
{
    QString filename = QDir(labelPath->text()).filePath(lineName->text());
    float lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;
    GPS_Math_Str_To_Deg(lineTopLeft->text(), lon1, lat1);
    GPS_Math_Str_To_Deg(lineBottomRight->text(), lon2, lat2);

    /// for 17th zoom level
    int zoomlevel = 17;
    int offsetLon   = (lon1 + 180) * (1<<zoomlevel) / 360;
    int limitLon    = (lon2 + 180) * (1<<zoomlevel) / 360;

    int offsetLat   = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoomlevel);
    int limitLat    = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoomlevel);

    QString file = QString("http://tah.openstreetmap.org/Tiles/tile.php/%1/%2/%3.png").arg(zoomlevel).arg(offsetLon).arg(offsetLat);

    qDebug() << offsetLon << offsetLat << limitLon << limitLat;
    qDebug() << file;

}


