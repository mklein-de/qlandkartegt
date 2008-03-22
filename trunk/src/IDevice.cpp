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
#include <QtGui>

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
