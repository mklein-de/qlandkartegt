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
#include "CDeviceNMEA.h"

#include <QtGui>
#include <projects.h>

CDeviceNMEA::CDeviceNMEA(const QString& serialport, QObject * parent)
: IDevice("NMEA",parent)
, serialport(serialport)
, haveSeenData(false)
, haveSeenGPVTG(false)
{
    tty.setBaudRate(BAUD4800);   //BaudRate
    tty.setDataBits(DATA_8);     //DataBits
    tty.setParity(PAR_NONE);     //Parity
    tty.setStopBits(STOP_1);     //StopBits
    tty.setFlowControl(FLOW_OFF);//FlowControl

    tty.setTimeout(0, 10);
    tty.enableReceiving();
    tty.setPort(serialport);

    watchdog = new QTimer(this);
    connect(watchdog, SIGNAL(timeout()), this, SLOT(slotWatchdog()));

    connect(&tty, SIGNAL(newDataReceived(const QByteArray &)), this, SLOT(slotNewDataReceived(const QByteArray &)));
}


CDeviceNMEA::~CDeviceNMEA()
{
    tty.close();
}


void CDeviceNMEA::setLiveLog(bool on)
{
    qDebug() << "void CDeviceNMEA::setLiveLog()" << on;
    if(on) {
        log.fix = CLiveLog::eNoFix;
        emit sigLiveLog(log);

        if(tty.open()) {
            tty.receiveData();
        }
        watchdog->start(10000);
        haveSeenData    = false;
        haveSeenGPVTG   = false;
    }
    else {
        tty.close();
        log.fix = CLiveLog::eOff;
        emit sigLiveLog(log);
    }
}


bool CDeviceNMEA::liveLog()
{
    return tty.isOpen();
}


void CDeviceNMEA::slotNewDataReceived(const QByteArray &dataReceived)
{
    int i;

    for(i = 0; i < dataReceived.size(); ++i) {

        if(dataReceived[i] == '\n') {
            line = line.trimmed();
            decode();
        }
        else {
            line += dataReceived[i];
        }
    }
}


void CDeviceNMEA::decode()
{
    QString tok;
    QStringList tokens = line.split(QRegExp("[,*]"));
    //     qDebug() << line;
    //     qDebug() << tokens.count() << tokens;
    if((tokens[0] == "$GPGGA")) {
        //             0      1                  2       3         4        5    6     7     8       9     10    11     12   13    14
        //     15 ("$GPGGA", "130108.000", "4901.7451", "N", "01205.8656", "E", "1", "06", "1.8", "331.6", "M", "47.3", "M", "", "0000*5F")
        //         qDebug() << tokens.count() << tokens;
        log.ele = tokens[9].toDouble();

    }
    else if((tokens[0] == "$GPGSA")) {
        //             0      1    2     3    4      5     6     7    8   9  10  11  12  13  14    15    16     17
        //     18 ("$GPGSA", "A", "3", "11", "23", "13", "04", "17", "", "", "", "", "", "", "", "3.5", "2.2", "2.6*31")
        //         qDebug() << tokens.count() << tokens;
        int tok = tokens[2].toInt();
        log.fix = tok == 3 ? CLiveLog::e3DFix : tok == 2 ? CLiveLog::e2DFix : CLiveLog::eNoFix;
        log.error_horz = tokens[16].toDouble();
        log.error_vert = tokens[17].toDouble();
        //         qDebug() << (hdop * 3);
    }
    else if((tokens[0] == "$GPRMC")) {
        //         13 ("$GPRMC", "122450.539", "V", "4901.6288", "N", "01205.5946", "E", "", "", "030408", "", "", "N*76")
        //            ("$GPRMC", "183956.648", "A", "4341.0506", "N", "00407.7897", "E", "3.81", "186.84", "060408", "", "", "A", "64")
        //         qDebug() << tokens.count() << tokens[1] << tokens[9];
        QDateTime datetime = QDateTime::fromString(tokens[1] + tokens[9],"hhmmss.zzzddMMyy").addYears(100);
        datetime.setTimeSpec(Qt::UTC);
        log.timestamp = datetime.toTime_t();

        tok = tokens[3];
        log.lat = tok.left(2).toInt() + tok.mid(2).toDouble() / 60.0;
        tok = tokens[5];
        log.lon = tok.left(3).toInt() + tok.mid(3).toDouble() / 60.0;

        log.lon = (tokens[4] == "N" ? log.lon : -log.lon);
        log.lat = (tokens[6] == "E" ? log.lat : -log.lat);

        log.velocity = 0;
        log.heading  = 0;

        //         calcSecondaryData();
        if(!haveSeenGPVTG) {
            emit sigLiveLog(log);
        }
    }
    else if((tokens[0] == "$GPVTG")) {
        //                  0        1       2    3   4      5     6      7     8    9
        //          11 ("$GPVTG", "357.22", "T", "", "M", "0.09", "N", "0.16", "K", "A", "32")

        log.velocity    = tokens[5].toFloat() * 0.514444;
        log.heading     = tokens[1].toFloat();

        emit sigLiveLog(log);

        haveSeenGPVTG = true;
    }

    haveSeenData = true;
    line.clear();
}


void CDeviceNMEA::slotWatchdog()
{
    if(tty.isOpen() && haveSeenData) {
        haveSeenData = false;
        return;
    }
    watchdog->stop();

    setLiveLog(false);

}


void CDeviceNMEA::uploadWpts(const QList<CWpt*>& /*wpts*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::downloadWpts(QList<CWpt*>& /*wpts*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::uploadTracks(const QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::downloadTracks(QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::uploadRoutes(const QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::downloadRoutes(QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::uploadMap(const QList<IMapSelection*>& /*mss*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceNMEA::downloadScreenshot(QImage& /*image*/)
{
    QMessageBox::information(0,tr("Error..."), tr("NMEA: Download screenshots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}
