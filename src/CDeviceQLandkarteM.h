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
#ifndef CDEVICEQLANDKARTEM_H
#define CDEVICEQLANDKARTEM_H

#include "IDevice.h"
#include <QtNetwork> 

class CDeviceQLandkarteM : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceQLandkarteM(const QString& ipaddr, quint16 port, QObject * parent);
        virtual ~CDeviceQLandkarteM();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);


        void uploadMap(const QList<IMapSelection*>& mss);

    private:
	QUdpSocket *udpSocket;
        QString ipaddr;
        quint16 port;
    private slots:
	void detectedDevice();

};

#endif //CDEVICEQLANDKARTEM_H

