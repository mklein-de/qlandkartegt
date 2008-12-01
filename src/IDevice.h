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
#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QList>
#include <QPointer>

#include "CLiveLog.h"

class CWpt;
class CTrack;
class QProgressDialog;
class IMapSelection;

class IDevice : public QObject
{
    Q_OBJECT;
    public:
        IDevice(const QString& devkey, QObject * parent);
        virtual ~IDevice();

        virtual const QString getDevKey(){return devkey;}

        virtual void uploadWpts(const QList<CWpt*>& wpts) = 0;
        virtual void downloadWpts(QList<CWpt*>& wpts) = 0;

        virtual void uploadTracks(const QList<CTrack*>& trks) = 0;
        virtual void downloadTracks(QList<CTrack*>& trks) = 0;

        virtual void uploadMap(const QList<IMapSelection*>& mss) = 0;

        virtual void downloadAll();
        virtual void uploadAll();

        virtual void setLiveLog(bool on);
        virtual bool liveLog(){return false;}

        static bool m_UploadAllWpt;
        static bool m_DownloadAllWpt;
        static bool m_UploadAllTrk;
        static bool m_DownloadAllTrk;

        signals:
        void sigLiveLog(const CLiveLog& log);

    protected:
        void createProgress(const QString& title, const QString& text, int max);
        QString devkey;
        QPointer<QProgressDialog> progress;

        CLiveLog log;
};
#endif                           //IDEVICE_H
