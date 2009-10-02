/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDeviceMikrokopter.cpp

  Module:

  Description:

  Created:     10/02/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDeviceMikrokopter.h"

#include <QtGui>

CDeviceMikrokopter::CDeviceMikrokopter(const QString& port, QObject * parent)
: IDevice("Mikrokopter", parent)
{

}

CDeviceMikrokopter::~CDeviceMikrokopter()
{

}

void CDeviceMikrokopter::uploadWpts(const QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::downloadWpts(QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::uploadTracks(const QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::downloadTracks(QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::uploadRoutes(const QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::downloadRoutes(QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::uploadMap(const QList<IMapSelection*>& mss)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceMikrokopter::downloadScreenshot(QImage& image)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Download screenschots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

