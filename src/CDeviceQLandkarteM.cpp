/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#include "CDeviceQLandkarteM.h"

#include <QtGui>

CDeviceQLandkarteM::CDeviceQLandkarteM(const QString& ipaddr, quint16 port, QObject * parent)
: IDevice("QLandkarteM",parent)
, ipaddr(ipaddr)
, port(port)
{

}

CDeviceQLandkarteM::~CDeviceQLandkarteM()
{

}

void CDeviceQLandkarteM::uploadWpts(const QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::downloadWpts(QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadTracks(const QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::downloadTracks(QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadMap(const QList<IMapSelection*>& mss)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload map is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadRoutes(const QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::downloadRoutes(QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

