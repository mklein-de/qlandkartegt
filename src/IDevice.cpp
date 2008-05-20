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

#include "IDevice.h"
#include "CWptDB.h"
#include "CTrackDB.h"
#include <QtGui>

bool IDevice::m_UploadAllWpt    = true;
bool IDevice::m_DownloadAllWpt  = true;
bool IDevice::m_DownloadAllTrk  = true;


IDevice::IDevice(const QString& devkey, QObject * parent)
: QObject(parent)
, devkey(devkey)
{

}


IDevice::~IDevice()
{

}


void IDevice::createProgress(const QString& title, const QString& text, int max)
{
    if(!progress.isNull()) delete progress;
    progress = new QProgressDialog(title,tr("Abort"),0,max);
    progress->setAttribute(Qt::WA_DeleteOnClose,true);
    progress->setAutoReset(false);
    progress->setAutoClose(false);
    progress->setLabelText(text);
    progress->show();
}


void IDevice::downloadAll()
{
    if(m_DownloadAllWpt) CWptDB::self().download();
    if(m_DownloadAllTrk) CTrackDB::self().download();
}

void IDevice::uploadAll()
{
    if(m_UploadAllWpt) CWptDB::self().upload();
}

void IDevice::setLiveLog(bool on)
{
    QMessageBox::information(0,tr("Sorry... "), tr("Your device does not support live log."), QMessageBox::Ok, QMessageBox::Ok);
}
