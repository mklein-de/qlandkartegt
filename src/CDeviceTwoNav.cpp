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


#include "CDeviceTwoNav.h"

#include <QtGui>

CDeviceTwoNav::CDeviceTwoNav(QObject *parent)
: IDevice("TwoNav", parent)
{

}

CDeviceTwoNav::~CDeviceTwoNav()
{

}

void CDeviceTwoNav::uploadWpts(const QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload wapoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadWpts(QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download wapoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::uploadTracks(const QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadTracks(QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::uploadRoutes(const QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadRoutes(QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::uploadMap(const QList<IMapSelection*>& mss)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadScreenshot(QImage& image)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download screenshots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}
