/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDeviceMikrokopter.h

  Module:

  Description:

  Created:     10/02/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDEVICEMIKROKOPTER_H
#define CDEVICEMIKROKOPTER_H

#include "IDevice.h"

class CDeviceMikrokopter : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceMikrokopter(const QString& port, QObject * parent);
        virtual ~CDeviceMikrokopter();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);

        void uploadMap(const QList<IMapSelection*>& mss);

        void downloadScreenshot(QImage& image);
};

#endif //CDEVICEMIKROKOPTER_H

