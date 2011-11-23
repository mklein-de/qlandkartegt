/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDeviceGarminBulk.h

  Module:

  Description:

  Created:     11/23/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDEVICEGARMINBULK_H
#define CDEVICEGARMINBULK_H


#include "IDevice.h"

class CDeviceGarminBulk : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceGarminBulk(QObject * parent);
        virtual ~CDeviceGarminBulk();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);

        void uploadMap(const QList<IMapSelection*>& mss);

        void setLiveLog(bool on);
        bool liveLog(){return false;}

        void downloadScreenshot(QImage& image);

};

#endif //CDEVICEGARMINBULK_H

