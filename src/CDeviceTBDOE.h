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
#ifndef CDEVICETBDOE_H
#define CDEVICETBDOE_H

#include "IDevice.h"
#include <QtNetwork/QTcpSocket>

class QProgressDialog;

/// device layer for QLandkarte M devices
/**

    Protocol specification:

    QLandkarte M := host; QLandkarte GT := client

    Each transaction is initiated by the client. A packet sent to the host is achnowledeged by
    a packet from the host. The client has to wait for each packet to get achnowledeged. The host
    and client will operate the socket as a QDataStream. Thus all objects sent will get serialized
    according to Trolltech's spec.

    The format of a packet is:

    qint32 type, qint32 size, QByteArray data

    The type will be an enumeration of type packet_e. The size value is needed by the receiving
    socket to wait for the reception of all packet data.


*/
class CDeviceTBDOE : public IDevice
{
    Q_OBJECT
    public:
        CDeviceTBDOE(const QString& ipaddr, quint16 port, QObject * parent);
        virtual ~CDeviceTBDOE();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        enum packet_e {
              eNone
            , eError        ///< error occured
            , eAck          ///<
            , eC2HAlive
            , eH2CAlive
            , eC2HWpt       ///< send waypoint data from client to host
            , eH2CWptQuery  ///< request waypoint keys from host
            , eH2CWpt       ///< request waypoint data from host
        };

    private:
        bool acquire(const QString& operation, int max);
        void send(const packet_e type, const QByteArray& data);
        bool recv(packet_e& type, QByteArray& data);
        bool exchange(packet_e& type,QByteArray& data);
        void release();

        QString ipaddr;
        quint16 port;
        const int timeout;

        QTcpSocket socket;


};

#endif //CDEVICETBDOE_H

