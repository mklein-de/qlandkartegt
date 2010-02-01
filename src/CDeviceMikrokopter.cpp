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

#include "CDeviceMikrokopter.h"

#include <QtGui>
#include <ToolBox.h>

CDeviceMikrokopter::CDeviceMikrokopter(const QString& port, QObject * parent)
: IDevice("Mikrokopter", parent)
{

    tty.setBaudRate(BAUD57600);  //BaudRate
    tty.setDataBits(DATA_8);     //DataBits
    tty.setParity(PAR_NONE);     //Parity
    tty.setStopBits(STOP_1);     //StopBits
    tty.setFlowControl(FLOW_OFF);//FlowControl

    tty.setTimeout(0, 10);
    tty.enableSending();
    tty.enableReceiving();
    tty.setPort(port);

    connect(&tty, SIGNAL(newDataReceived(const QByteArray &)), this, SLOT(slotNewData(const QByteArray &)));
}


CDeviceMikrokopter::~CDeviceMikrokopter()
{

}


bool CDeviceMikrokopter::aquire()
{
    if(!tty.isOpen()) {
        tty.open();
        tty.receiveData();
    }

    if(!tty.isOpen()) {
        QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Failed to open serial port."),QMessageBox::Abort,QMessageBox::Abort);
        return false;
    }

    return true;
}


void CDeviceMikrokopter::release()
{
    tty.close();

}


bool CDeviceMikrokopter::sendCmd(char cmd, int addr, char data[150], unsigned int len, bool resend)
{
    if(!tty.isOpen()) return false;

    QString encodedData;
    encodedData = ToolBox::Encode64(data, len);

    encodedData = QString("#") + (QString('a' + addr)) + QString(cmd) + encodedData;
    encodedData = ToolBox::add_CRC(encodedData) + '\r';

    QByteArray bytebuffer = QByteArray(encodedData.toUtf8());

    return tty.sendData(bytebuffer) == 1;
}


void CDeviceMikrokopter::slotNewData(const QByteArray & data)
{
    qDebug() << "slotNewData()";

    const char *ptr;
    ptr = data.data();
    int idx = 0;

    while (ptr[idx] != '\0') {
        if (ptr[idx] == '\r') {
            while ((rxData.length() > 1) && (rxData.at(1) == '#')) {
                rxData.remove(0,1);
            }

            if (ToolBox::check_CRC(rxData)) {
                // do decoding?
                qDebug() << rxData << rxData.size();
            }
            else {
                qDebug() << "error";
            }

            rxData.clear();
        }
        else {
            rxData +=  QString(ptr[idx]);
        }
        ++idx;
    }
}


void CDeviceMikrokopter::uploadWpts(const QList<CWpt*>& wpts)
{
    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceMikrokopter::downloadWpts(QList<CWpt*>& wpts)
{
    //    QMessageBox::information(0,tr("Error..."), tr("Mikrokopter: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);

    if(!aquire()) return;

    qDebug() << sendCmd('v', 2, 0, 0, 0);

    //release();

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
