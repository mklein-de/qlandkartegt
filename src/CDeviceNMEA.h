/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDeviceNMEA.h

  Module:

  Description:

  Created:     01/03/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDEVICENMEA_H
#define CDEVICENMEA_H

#include "IDevice.h"
#include <ManageSerialPort.h>

class CDeviceNMEA : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceNMEA(const QString& serialport, QObject * parent);
        virtual ~CDeviceNMEA();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);

        void uploadMap(const QList<IMapSelection*>& mss);

        void downloadScreenshot(QImage& image);

        void setLiveLog(bool on);
        bool liveLog();

    private slots:
        void slotNewDataReceived(const QByteArray &dataReceived);
        void slotWatchdog();

    private:
        void decode();

        QString serialport;
        QString line;
        ManageSerialPort tty;

        bool haveSeenData;
        bool haveSeenGPVTG;

        QTimer * watchdog;

};

#endif //CDEVICENMEA_H

