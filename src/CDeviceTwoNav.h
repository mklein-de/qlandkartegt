/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CDEVICETWONAV_H
#define CDEVICETWONAV_H

#include "IDevice.h"
#include "ui_IDlgDeviceTwoNavPath.h"

#include <QDialog>

class CWpt;

class CDlgDeviceTwoNavPath : public QDialog, private Ui::IDlgDeviceTwoNavPath
{
    Q_OBJECT;
    public:
        CDlgDeviceTwoNavPath(const QString &what, QDir &dir, QString &subdir, QWidget *parent);
        ~CDlgDeviceTwoNavPath();

    private slots:
        void slotItemClicked(QListWidgetItem*item);
        void slotReturnPressed();

    private:
        QString& subdir;
};


class CDeviceTwoNav : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceTwoNav(QObject * parent);
        virtual ~CDeviceTwoNav();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);

        void uploadMap(const QList<IMapSelection*>& mss);

        void downloadScreenshot(QImage& image);

    private:
        bool aquire(QDir& dir);
        void createDayPath(const QString &what);
        void readWptFile(QDir &dir, const QString &filename, QList<CWpt *> &wpts);

        QString makeUniqueName(const QString name, QDir& dir);
        void writeWaypointData(QTextStream& out, CWpt * wpt, QDir &dir);

        QString iconTwoNav2QlGt(const QString& sym);
        QString iconQlGt2TwoNav(const QString& sym);

        QString pathRoot;
        QString pathData;
        QString pathDay;

        CWpt * wpt;

};

#endif //CDEVICETWONAV_H

