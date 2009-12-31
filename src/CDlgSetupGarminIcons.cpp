/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgSetupGarminIcons.cpp

  Module:

  Description:

  Created:     12/30/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/
#include "config.h"

#include "CDlgSetupGarminIcons.h"
#include "WptIcons.h"
#include "CResources.h"

#include <QtGui>

#undef IDEVICE_H
#include <garmin/IDevice.h>

#include <QtGui>

#include <limits>
#include <math.h>

#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif


CDlgSetupGarminIcons::CDlgSetupGarminIcons()
{
    setupUi(this);
    connect(butSend,SIGNAL(pressed()),this,SLOT(slotSendToDevice()));
}

CDlgSetupGarminIcons::~CDlgSetupGarminIcons()
{

}

void CDlgSetupGarminIcons::exec()
{
    QPushButton * but;
    QPixmap icon;
    QString name;
    QString src;

    // setup custom icons tab
    for(int i=0; i < N_CUSTOM_ICONS; ++i) {
        QTreeWidgetItem * entry = new QTreeWidgetItem(listCustomIcons);

        name = QString("Custom %1").arg(i + 1);

        entry->setIcon(0,QIcon(getWptIconByName(name,&src)));

        entry->setText(1,name);

        but = new QPushButton("x",listCustomIcons);
        but->setObjectName(QString::number(i));
        but->setToolTip(tr("reset icon"));
        connect(but,SIGNAL(pressed()), this, SLOT(slotResetIconSource()));
        listCustomIcons->setItemWidget(entry,2,but);

        but = new QPushButton("...",listCustomIcons);
        but->setObjectName(QString::number(i));
        but->setToolTip(tr("select icon"));
        connect(but,SIGNAL(pressed()), this, SLOT(slotChangeIconSource()));
        listCustomIcons->setItemWidget(entry,3,but);

        entry->setText(4,src);

    }

    listCustomIcons->header()->setResizeMode(0,QHeaderView::Interactive);
    listCustomIcons->header()->setResizeMode(1,QHeaderView::Interactive);
    listCustomIcons->header()->setResizeMode(2,QHeaderView::Interactive);
    listCustomIcons->header()->setResizeMode(3,QHeaderView::Interactive);
    listCustomIcons->resizeColumnToContents(0);
    listCustomIcons->resizeColumnToContents(1);
    listCustomIcons->resizeColumnToContents(2);
    listCustomIcons->resizeColumnToContents(3);

    QDialog::exec();
}

void CDlgSetupGarminIcons::accept()
{
    QSettings cfg;
    QString name;
    for(int i=0; i < N_CUSTOM_ICONS; ++i) {
        name = QString("Custom %1").arg(i + 1);

        QTreeWidgetItem * entry = listCustomIcons->topLevelItem(i);
        setWptIconByName(name, entry->text(4));

        name = QString("garmin/icons/custom%1").arg(i + 1);
        cfg.setValue(name, entry->text(4));
    }

    QDialog::accept();
}

void CDlgSetupGarminIcons::slotChangeIconSource()
{
    QSettings cfg;
    QDir dir(cfg.value("path/icons",QDir::homePath()).toString());

    QTreeWidgetItem * entry = listCustomIcons->topLevelItem(sender()->objectName().toInt());
    if(entry) {
        QString filename = QFileDialog::getOpenFileName(0,tr("Select icon ..."),dir.path());
        if(!filename.isEmpty()) {
            entry->setIcon(0,QIcon(loadIcon(filename)));

            entry->setText(4,filename);

            cfg.setValue("path/icons",QFileInfo(filename).absolutePath());
        }
    }
}

void CDlgSetupGarminIcons::slotResetIconSource()
{
    QTreeWidgetItem * entry = listCustomIcons->topLevelItem(sender()->objectName().toInt());
    if(entry) {
        entry->setIcon(0,QIcon(loadIcon(":/icons/wpt/custom15x15.bmp")));
        entry->setText(4,":/icons/wpt/custom15x15.bmp");
    }

}

#pragma pack(1)
// Header of Garmin 256-color BMP bitmap files
struct garmin_bmp_t
{
    quint16 bfType;
    quint32 bfSize;
    quint32 bfReserved;
    quint32 bfOffBits;

    quint32 biSize;
    qint32 biWidth;
    qint32 biHeight;
    quint16 biPlanes;
    quint16 biBitCount;
    quint32 biCompression;
    quint32 biSizeImage;
    quint32 biXPelsPerMeter;
    quint32 biYPelsPerMeter;
    quint32 biClrUsed;
    quint32 biClrImportant;

    quint32 clrtbl[0x100];

    quint8  data[];
};

#pragma pack(0)


void CDlgSetupGarminIcons::slotSendToDevice()
{

    std::list<Garmin::Icon_t> icons;

    for(int i=0; i < N_CUSTOM_ICONS; ++i) {
        QTreeWidgetItem * entry = listCustomIcons->topLevelItem(i);
        QFile file(entry->text(4));
        if(!file.open(QIODevice::ReadOnly)) continue;
        QByteArray buffer = file.readAll();
        file.close();
        const garmin_bmp_t * pBmp = (garmin_bmp_t*)buffer.data();

        qDebug() << "-----------" << entry->text(4);
        qDebug() << "bfType              " << hex << pBmp->bfType;
        qDebug() << "bfSize              " << hex << pBmp->bfSize;
        qDebug() << "bfReserved          " << hex << pBmp->bfReserved;
        qDebug() << "bfOffBits           " << hex << pBmp->bfOffBits;

        qDebug() << "biSize              " << hex << pBmp->biSize;
        qDebug() << "biWidth             " << hex << pBmp->biWidth;
        qDebug() << "biHeight            " << hex << pBmp->biHeight;
        qDebug() << "biPlanes            " << hex << pBmp->biPlanes;
        qDebug() << "biBitCount          " << hex << pBmp->biBitCount;
        qDebug() << "biCompression       " << hex << pBmp->biCompression;
        qDebug() << "biSizeImage         " << hex << pBmp->biSizeImage;
        qDebug() << "biXPelsPerMeter     " << hex << pBmp->biXPelsPerMeter;
        qDebug() << "biYPelsPerMeter     " << hex << pBmp->biYPelsPerMeter;
        qDebug() << "biClrUsed           " << hex << pBmp->biClrUsed;
        qDebug() << "biClrImportant      " << hex << pBmp->biClrImportant;
        qDebug() << "clrtbl[0]           " << hex << pBmp->clrtbl[0];
        qDebug() << "clrtbl[1]           " << hex << pBmp->clrtbl[1];
        qDebug() << "clrtbl[0xfe]        " << hex << pBmp->clrtbl[0xfe];
        qDebug() << "clrtbl[0xff]        " << hex << pBmp->clrtbl[0xff];

        if(    pBmp->biBitCount != 8
            || pBmp->biCompression != 0
            || (pBmp->biClrUsed != 0 && pBmp->biClrUsed != 0x100)
            || (pBmp->biClrImportant != 0 && pBmp->biClrImportant != 0x100)
            || pBmp->biWidth != 16
            || pBmp->biHeight != 16) {
            QMessageBox::warning(0,tr("Format Error"),entry->text(4) + tr(": Bad icon format"),QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }

        Garmin::Icon_t icon;
        icons.push_back(icon);
        icons.back().idx = i;
        memcpy(icons.back().clrtbl,pBmp->clrtbl,sizeof(icon.clrtbl));

        for(int r = 0; r < pBmp->biHeight; ++r) {
            for(int c = 0; c < pBmp->biWidth; ++c) {
                icons.back().data[r * pBmp->biWidth + c] = pBmp->data[(pBmp->biHeight - 1 - r)*pBmp->biWidth + c];
            }
        }

        //memcpy(icons.back().data,pBmp->data,pBmp->biSizeImage);
    }


    IDevice * dev = CResources::self().device();
    if(dev) {


    }
/*    Garmin::IDevice * dev = 0;
    try
    {
        dev = gpResources->device();
        if(dev) {
            dev->uploadCustomIcons(icons);
        }
    }
    catch(int e) {
        if(dev == 0) return;
        QMessageBox::warning(0,tr("Device Link Error"),dev->getLastError().c_str(),QMessageBox::Ok,QMessageBox::NoButton);
        if(e == Garmin::errSync) {
            gpResources->resetDevice();
        }
    }*/

}

