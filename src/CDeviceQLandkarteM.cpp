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

#define REMOTE_PORT 45454

CDeviceQLandkarteM::CDeviceQLandkarteM(const QString& ipaddr, quint16 port, QObject * parent)
: IDevice("QLandkarteM",parent)
, ipaddr(ipaddr)
, port(port)
, timeout(60000)
{
    udpSocket.bind(45453);
    connect(&udpSocket, SIGNAL(readyRead()),this, SLOT(detectedDevice()));

}

CDeviceQLandkarteM::~CDeviceQLandkarteM()
{

}

bool CDeviceQLandkarteM::acquire(const QString& operation, int max)
{
    createProgress(operation, tr("Connect to device."), max);
    qApp->processEvents();

    tcpSocket.connectToHost(ipaddr,port);
    if(!tcpSocket.waitForConnected(timeout)) {
        QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to connect to device."),QMessageBox::Abort,QMessageBox::Abort);
        release();
        return false;
    }
    return true;
}

void CDeviceQLandkarteM::send(const packet_e type, const QByteArray& data)
{
    QByteArray packet;
    QDataStream out(&packet,QIODevice::WriteOnly);
    out << (qint32)type;
    out << (qint32)0;
    out << data;
    out.device()->seek(sizeof(type));
    out << (qint32)(packet.size() - 2 * sizeof(qint32));

    tcpSocket.write(packet);
}


bool CDeviceQLandkarteM::recv(packet_e& type, QByteArray& data)
{
    qint32 size;
    QDataStream in(&tcpSocket);

    while(tcpSocket.bytesAvailable() < (int)(2 * sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead(timeout)) {
            return false;
        }
    }

    in >> (qint32&)type;
    in >> size;

    while(tcpSocket.bytesAvailable() < size) {
        if (!tcpSocket.waitForReadyRead(timeout)) {
            return false;
        }
    }

    in >> data;
    return true;
}


bool CDeviceQLandkarteM::exchange(packet_e& type,QByteArray& data)
{
    send(type,data);
    data.clear();
    return recv(type,data);
}


void CDeviceQLandkarteM::release()
{
    if(progress) progress->close();
    tcpSocket.disconnectFromHost();
}


void CDeviceQLandkarteM::uploadWpts(const QList<CWpt*>& wpts)
{
    if(!startDeviceDetection()) return;
    if(!acquire(tr("Upload waypoints ..."), wpts.count())) return;


    return release();
}

void CDeviceQLandkarteM::downloadWpts(QList<CWpt*>& wpts)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadTracks(const QList<CTrack*>& trks)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::downloadTracks(QList<CTrack*>& trks)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadMap(const QList<IMapSelection*>& mss)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload map is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::uploadRoutes(const QList<CRoute*>& rtes)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceQLandkarteM::downloadRoutes(QList<CRoute*>& rtes)
{
    if(!startDeviceDetection()) return;
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

bool CDeviceQLandkarteM::startDeviceDetection()
{

    qDebug() << ipaddr << port;
    if(ipaddr.isEmpty() || port == 0){

        QByteArray datagram = "GETADRESS";
        udpSocket.writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, REMOTE_PORT);

        QTime time;
        time.start();
        while(time.elapsed() < 5000){
            QApplication::processEvents();
        }
    }

    if(ipaddr.isEmpty() || port == 0){
        QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: No device found. Is it connected to the network?"),QMessageBox::Abort,QMessageBox::Abort);
        return false;
    }

    return true;
}

void CDeviceQLandkarteM::detectedDevice()
{
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress qlmAddress;
        quint16 qlmPort;

        datagram.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagram.data(), datagram.size(), &qlmAddress, &qlmPort);

        ipaddr  = qlmAddress.toString();
        port    = qlmPort;
        qDebug() << "Device detected is " << datagram << " with address " << ipaddr << " and port " << port << "\r\n";

        datagram = "START";
        udpSocket.writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, REMOTE_PORT);

    }
}
