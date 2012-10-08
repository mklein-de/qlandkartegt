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

#ifndef CCREATEMAPWMS_H
#define CCREATEMAPWMS_H

#include <QWidget>
#include <QtNetwortk>
#include "ui_ICreateMapWMS.h"

class QHttp;

class CCreateMapWMS : public QWidget, private Ui::ICreateMapWMS
{
    Q_OBJECT;
    public:
        CCreateMapWMS(QWidget * parent);
        virtual ~CCreateMapWMS();

    private slots:
        void slotLoadCapabilities();
        void slotSetupLink();
        void slotRequestStarted(int );
        void slotRequestFinished(int , bool error);
        void slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);
        void slotSave();
        void slotSelectFile();

    private:
        QHttp * server;
        QString versionString;
        QString urlOnlineResource;
        QRectF  rectLatLonBoundingBox;
        QString mapPath;
};
#endif                           //CCREATEMAPWMS_H
