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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QList>
#include <QPointer>

class CWpt;
class QProgressDialog;

class IDevice : public QObject
{
    Q_OBJECT
    public:
        IDevice(const QString& devkey, QObject * parent);
        virtual ~IDevice();

        const QString& getDevKey(){return devkey;}

        virtual void uploadWpts(const QList<CWpt*>& wpts) = 0;
        virtual void downloadWpts(QList<CWpt*>& wpts) = 0;

    protected:
        void createProgress(const QString& title, const QString& text, int max);
        QString devkey;
        QPointer<QProgressDialog> progress;
};

#endif //IDEVICE_H

