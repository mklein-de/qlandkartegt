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

#include "CDlgScreenshot.h"
#include "CResources.h"
#include "IDevice.h"

#include <QtGui>

CDlgScreenshot::CDlgScreenshot(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);

    connect(butAcquire, SIGNAL(clicked()), this, SLOT(slotAcquire()));
    connect(butSave, SIGNAL(clicked()), this, SLOT(slotSave()));
}

CDlgScreenshot::~CDlgScreenshot()
{

}

void CDlgScreenshot::slotAcquire()
{
    IDevice * dev = CResources::self().device();
    if(dev) {
        dev->downloadScreenshot(image);
        labelScreenshot->setPixmap(QPixmap::fromImage(image));

        butSave->setEnabled(true);
    }
}

void CDlgScreenshot::slotSave()
{
    QSettings cfg;
    QString pathData = cfg.value("path/data","./").toString();

    QString filter;
    QString filename = QFileDialog::getSaveFileName( 0, tr("Select output file")
        ,pathData
        ,"Bitmap (*.png *.jpg);;"
        ,&filter
        , QFileDialog::DontUseNativeDialog
        );

    if(filename.isEmpty()) return;

    image.save(filename);

}
