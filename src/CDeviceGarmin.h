/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CDEVICEGARMIN_H
#define CDEVICEGARMIN_H

#include "IDevice.h"

#include <QTime>

class QTimer;

namespace Garmin
{
    class IDevice;
}


class CDeviceGarmin : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceGarmin(const QString& devkey, const QString& port, QObject * parent);
        virtual ~CDeviceGarmin();

        const QString getDevKey(){return QString("Garmin");}

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);

        void uploadMap(const QList<IMapSelection*>& mss);

        void setLiveLog(bool on);
        bool liveLog();

    private slots:
        void slotTimeout();
        void slotCancel();

    private:
        Garmin::IDevice * getDevice();

        QString port;

        friend void GUICallback(int progress, int * ok, int * cancel, const char * title, const char * msg, void * self);
        struct dlgdata_t
        {
            dlgdata_t() : dlgProgress(0), canceled(false)
                {}

            QProgressDialog * dlgProgress;
            QTime timeProgress;
            bool canceled;
        };

        dlgdata_t dlgData;

        QTimer * timer;

};
#endif                           //CDEVICEGARMIN_H
