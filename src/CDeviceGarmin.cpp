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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CDeviceGarmin.h"
#include "CMainWindow.h"

#undef IDEVICE_H
#include <garmin/IDevice.h>

#include <QtGui>

#define XSTR(x) STR(x)
#define STR(x) #x

/**
  @param progress the progress as integer from 0..100, if -1 no progress bar needed.
  @param ok if this pointer is 0 no ok button needed, if non zero set to 1 if ok button pressed
  @param cancel if this pointer is 0 no cancel button needed, if non zero set to 1 if cancel button pressed
  @param title dialog title as C string
  @param msg dialog message C string to display
  @param self void pointer as provided while registering the callback
*/
void GUICallback(int progress, int * ok, int * cancel, const char * title, const char * msg, void * self)
{
    CDeviceGarmin * parent = static_cast<CDeviceGarmin *>(self);
    CDeviceGarmin::dlgdata_t& dd = parent->dlgData;

    if(progress != -1) {
        quint32 togo, hour, min, sec;
        QString message;

        if(dd.dlgProgress == 0) {
            dd.canceled     = false;
            dd.dlgProgress  = new QProgressDialog(QString(title),0,0,100,theMainWindow, Qt::WindowStaysOnTopHint);
            dd.timeProgress.start();
            if(cancel) {
                QPushButton * butCancel = new QPushButton(QObject::tr("Cancel"),dd.dlgProgress);
                parent->connect(butCancel, SIGNAL(clicked()), parent, SLOT(slotCancel()));
                dd.dlgProgress->setCancelButton(butCancel);
            }
        }

        if(title) dd.dlgProgress->setWindowTitle(QString(title));

        togo = (quint32)((100.0 * (double)dd.timeProgress.elapsed() / (double)progress) + 0.5);
        togo = (quint32)((double)(togo - dd.timeProgress.elapsed()) / 1000.0 + 0.5);

        hour = (togo / 3600);
        min  = (togo - hour * 3600) / 60;
        sec  = (togo - hour * 3600 - min * 60);

        message.sprintf(QObject::tr("\n\nEstimated finish: %02i:%02i:%02i [hh:mm:ss]").toUtf8(),hour,min,sec);

        dd.dlgProgress->setLabelText(QString(msg) + message);
        dd.dlgProgress->setValue(progress);

        if(progress == 100 && dd.dlgProgress) {
            delete dd.dlgProgress;
            dd.dlgProgress = 0;
        }

        if(cancel) {
            *cancel = dd.canceled;
        }

        qApp->processEvents();

    }
    else {
        if(ok && cancel) {
            QMessageBox::StandardButtons res = QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel);
            *ok     = res == QMessageBox::Ok;
            *cancel = res == QMessageBox::Cancel;
        }
        else if(ok && !cancel) {
            QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Ok,QMessageBox::Ok);
            *ok     = true;
        }
        else if(!ok && cancel) {
            QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Cancel,QMessageBox::Cancel);
            *cancel     = true;
        }
        else if(!ok && !cancel) {
            //kiozen - that doesn't work nicely
            //             QMessageBox * dlg = new QMessageBox(&parent->main);
            //             dlg->setWindowTitle(QString(title));
            //             dlg->setText(QString(msg));
            //             dlg->setStandardButtons(QMessageBox::NoButton);
            //             dlg->setIcon(QMessageBox::Information);
            //             dlg->show();
            //             qApp->processEvents(QEventLoop::AllEvents, 1000);
            //             sleep(3); // sleep for 3 seconds
            //             delete dlg;
        }
    }
}


CDeviceGarmin::CDeviceGarmin(const QString& devkey, const QString& port, QObject * parent)
    : IDevice(devkey, parent)
    , port(port)
{
    qDebug() << "CDeviceGarmin::CDeviceGarmin()";

}

CDeviceGarmin::~CDeviceGarmin()
{
    qDebug() << "~CDeviceGarmin::CDeviceGarmin()";
}

Garmin::IDevice * CDeviceGarmin::getDevice()
{
    Garmin::IDevice * (*func)(const char*) = 0;
    Garmin::IDevice * dev = 0;

    QString libname     = QString("%1/lib%2" XSTR(SOEXT)).arg(XSTR(QL_LIBDIR)).arg(devkey);
    QString funcname    = QString("init%1").arg(devkey);

    func = (Garmin::IDevice * (*)(const char*))QLibrary::resolve(libname,funcname.toAscii());

    if(func == 0) {
        QMessageBox::warning(0,tr("Error ..."),tr("Failed to load driver."),QMessageBox::Ok,QMessageBox::NoButton);
        return 0;
    }

    dev = func(INTERFACE_VERSION);
    if(dev == 0) {
        QMessageBox::warning(0,tr("Error ..."),tr("Driver version mismatch."),QMessageBox::Ok,QMessageBox::NoButton);
        func = 0;
    }

    if(dev){
        dev->setPort(port.toLatin1());
        dev->setGuiCallback(GUICallback,this);
    }


    return dev;
}

void CDeviceGarmin::uploadWpts(const QList<CWpt*>& wpts)
{
    qDebug() << "CDeviceGarmin::uploadWpts()";
    Garmin::IDevice * dev = getDevice();
    if(dev == 0) return;

}

void CDeviceGarmin::downloadWpts(QList<CWpt*>& wpts)
{
    qDebug() << "CDeviceGarmin::downloadWpts()";
    Garmin::IDevice * dev = getDevice();
    if(dev == 0) return;
}
