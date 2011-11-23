/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDeviceGarminBulk.cpp

  Module:

  Description:

  Created:     11/23/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDeviceGarminBulk.h"

#include <QtGui>

CDeviceGarminBulk::CDeviceGarminBulk(QObject * parent)
: IDevice("Garmin Mass Storage", parent)
{

}

CDeviceGarminBulk::~CDeviceGarminBulk()
{

}

void CDeviceGarminBulk::uploadWpts(const QList<CWpt*>& /*wpts*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadWpts(QList<CWpt*>& /*wpts*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::uploadTracks(const QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadTracks(QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::uploadRoutes(const QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadRoutes(QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::uploadMap(const QList<IMapSelection*>& /*mss*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadScreenshot(QImage& /*image*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download screenshots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceGarminBulk::setLiveLog(bool on)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Live log is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}
