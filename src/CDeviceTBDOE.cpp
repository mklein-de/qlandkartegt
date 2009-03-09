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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CDeviceTBDOE.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "CTrack.h"
#include "CTrackDB.h"

#include <QtGui>

CDeviceTBDOE::CDeviceTBDOE(const QString& ipaddr, quint16 port, QObject * parent)
: IDevice("QLandkarteM",parent)
, ipaddr(ipaddr)
, port(port)
, timeout(60000)
{

}


CDeviceTBDOE::~CDeviceTBDOE()
{

}


void CDeviceTBDOE::send(const packet_e type, const QByteArray& data)
{
    QByteArray packet;
    QDataStream out(&packet,QIODevice::WriteOnly);
    out << (qint32)type;
    out << (qint32)0;
    out << data;
    out.device()->seek(sizeof(type));
    out << (qint32)(packet.size() - 2 * sizeof(qint32));

    socket.write(packet);
}


bool CDeviceTBDOE::recv(packet_e& type, QByteArray& data)
{
    qint32 size;
    QDataStream in(&socket);

    while(socket.bytesAvailable() < (int)(2 * sizeof(qint32))) {
        if (!socket.waitForReadyRead(timeout)) {
            return false;
        }
    }

    in >> (qint32&)type;
    in >> size;

    while(socket.bytesAvailable() < size) {
        if (!socket.waitForReadyRead(timeout)) {
            return false;
        }
    }

    in >> data;
    return true;
}


bool CDeviceTBDOE::exchange(packet_e& type,QByteArray& data)
{
    send(type,data);
    data.clear();
    return recv(type,data);
}


bool CDeviceTBDOE::acquire(const QString& operation, int max)
{
    createProgress(operation, tr("Connect to device."), max);
    qApp->processEvents();

    socket.connectToHost(ipaddr,port);
    if(!socket.waitForConnected(timeout)) {
        QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to connect to device."),QMessageBox::Abort,QMessageBox::Abort);
        release();
        return false;
    }
    return true;
}


void CDeviceTBDOE::release()
{
    if(progress) progress->close();
    socket.disconnectFromHost();
}


void CDeviceTBDOE::uploadWpts(const QList<CWpt*>& wpts)
{
    packet_e type;

    if(!acquire(tr("Upload waypoints ..."), wpts.count())) return;

    int cnt = 0;
    QList<CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end() && !progress->wasCanceled()) {
        QByteArray data;
        QDataStream s(&data,QIODevice::WriteOnly);

        progress->setLabelText(tr("%1\n%2 of %3").arg((*wpt)->name).arg(++cnt).arg(wpts.count()));
        progress->setValue(cnt);
        qApp->processEvents();

        s << *(*wpt);

        if(!exchange(type = eC2HWpt,data)) {
            QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to transfer waypoints."),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        if(type == eError) {
            QMessageBox::critical(0,tr("Error..."), QString(data),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        ++wpt;
    }

    return release();
}


void CDeviceTBDOE::downloadWpts(QList<CWpt*>& wpts)
{
    packet_e    type;
    QByteArray  data1;
    quint32     nWpt = 0;
    quint32     n;
    QString     key, name;

    if(!acquire(tr("Download waypoints ..."),1)) return;

    progress->setLabelText(tr("Query list of waypoints from the device"));
    qApp->processEvents();

    if(!exchange(type = eH2CWptQuery,data1)) {
        QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to query waypoints from device."),QMessageBox::Abort,QMessageBox::Abort);
        return release();
    }

    if(type == eError) {
        QMessageBox::critical(0,tr("Error..."), QString(data1),QMessageBox::Abort,QMessageBox::Abort);
        return release();
    }

    QDataStream wptlist(&data1, QIODevice::ReadOnly);

    wptlist >> nWpt;
    progress->setMaximum(nWpt);
    for(n = 0; n < nWpt; ++n) {
        QByteArray data;
        QDataStream stream(&data,QIODevice::ReadWrite);
        wptlist >> key >> name;

        stream << key;

        if(!exchange(type = eH2CWpt,data)) {
            QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to transfer waypoints."),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        if(type == eError) {
            QMessageBox::critical(0,tr("Error..."), QString(data),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        stream.device()->seek(0);

        CWpt * wpt = new CWpt(&CWptDB::self());
        stream >> *wpt;

        wpts.push_back(wpt);

        progress->setValue(n + 1);
    }

    return release();
}


void CDeviceTBDOE::uploadTracks(const QList<CTrack*>& trks)
{
    packet_e type;

    if(!acquire(tr("Upload tracks ..."), trks.count())) return;

    int cnt = 0;
    QList<CTrack*>::const_iterator trk = trks.begin();
    while(trk != trks.end() && !progress->wasCanceled()) {
        QByteArray data;
        QDataStream s(&data,QIODevice::WriteOnly);

        progress->setLabelText(tr("%1\n%2 of %3").arg((*trk)->name).arg(++cnt).arg(trks.count()));
        progress->setValue(cnt);
        qApp->processEvents();

        s << *(*trk);

        if(!exchange(type = eC2HTrk,data)) {
            QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to transfer tracks."),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        if(type == eError) {
            QMessageBox::critical(0,tr("Error..."), QString(data),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        ++trk;
    }

    return release();
}


void CDeviceTBDOE::downloadTracks(QList<CTrack*>& trks)
{
    packet_e    type;
    QByteArray  data1;
    quint32     nTrk = 0;
    quint32     n;
    QString     key, name;

    if(!acquire(tr("Download tracks ..."),1)) return;

    progress->setLabelText(tr("Query list of tracks from the device"));
    qApp->processEvents();

    if(!exchange(type = eH2CTrkQuery,data1)) {
        QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to query tracks from device."),QMessageBox::Abort,QMessageBox::Abort);
        return release();
    }

    if(type == eError) {
        QMessageBox::critical(0,tr("Error..."), QString(data1),QMessageBox::Abort,QMessageBox::Abort);
        return release();
    }

    QDataStream trklist(&data1, QIODevice::ReadOnly);

    trklist >> nTrk;
    progress->setMaximum(nTrk);
    for(n = 0; n < nTrk; ++n) {
        QByteArray data;
        QDataStream stream(&data,QIODevice::ReadWrite);
        trklist >> key >> name;

        stream << key;

        if(!exchange(type = eH2CTrk,data)) {
            QMessageBox::critical(0,tr("Error..."), tr("QLandkarteM: Failed to transfer tracks."),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        if(type == eError) {
            QMessageBox::critical(0,tr("Error..."), QString(data),QMessageBox::Abort,QMessageBox::Abort);
            return release();
        }

        stream.device()->seek(0);

        CTrack * trk = new CTrack(&CTrackDB::self());
        stream >> *trk;

        trks.push_back(trk);

        progress->setValue(n + 1);
    }

    return release();
}


void CDeviceTBDOE::uploadMap(const QList<IMapSelection*>& mss)
{
    QMessageBox::information(0,tr("Error..."), tr("QLandkarteM: Upload map is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}
