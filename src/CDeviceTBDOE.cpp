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

#include "CDeviceTBDOE.h"
#include "CWpt.h"

#include <QtGui>
#include <QtNetwork/QTcpSocket>

CDeviceTBDOE::CDeviceTBDOE(QObject * parent)
    : IDevice(parent)
{

}

CDeviceTBDOE::~CDeviceTBDOE()
{

}

void CDeviceTBDOE::uploadWpts(QList<CWpt*>& wpts)
{
    qint32 ack = -1;
    QTcpSocket socket;
    socket.connectToHost("192.168.1.2",4242);
    if(!socket.waitForConnected()){
        QMessageBox::critical(0,tr("Error..."), tr("Failed to connect to device."),QMessageBox::Abort,QMessageBox::Abort);
        return;
    }

    QDataStream socketstream(&socket);

    QList<CWpt*>::iterator wpt = wpts.begin();
    while(wpt != wpts.end()){
        QByteArray buf;
        QDataStream s(&buf,QIODevice::WriteOnly);

        s << *(*wpt);
        socketstream << buf.size() << buf;
        socket.flush();

        while(socket.bytesAvailable() < (int)sizeof(ack)){
            if (!socket.waitForReadyRead()) {
                 QMessageBox::critical(0,tr("Error..."), tr("Response timeout."),QMessageBox::Abort,QMessageBox::Abort);
                 return;
            }
        }
        socketstream >> ack;
        ++wpt;
    }


    socket.disconnectFromHost();
}

void CDeviceTBDOE::downloadWpts(QList<CWpt*>& wpts)
{

}

