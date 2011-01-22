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

#include "CDlgConvertToTrack.h"

#include <QtGui>

CDlgConvertToTrack::CDlgConvertToTrack(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);
    comboDelta->addItem(tr("none"), -1);
    comboDelta->addItem(tr("10 m"), 10);
    comboDelta->addItem(tr("20 m"), 20);
    comboDelta->addItem(tr("30 m"), 30);
    comboDelta->addItem(tr("50 m"), 50);
    comboDelta->addItem(tr("100 m"), 100);
    comboDelta->addItem(tr("200 m"), 200);
    comboDelta->addItem(tr("300 m"), 300);
    comboDelta->addItem(tr("500 m"), 500);
    comboDelta->addItem(tr("1 km"), 1000);

    QSettings cfg;
    comboDelta->setCurrentIndex(cfg.value("overlay/convert/intervall", 0).toInt());
    radioNoEle->setChecked(cfg.value("overlay/convert/noele", true).toBool());
    radioEleFromLocal->setChecked(cfg.value("overlay/convert/local", false).toBool());
    radioEleFromRemote->setChecked(cfg.value("overlay/convert/remote", false).toBool());

}


CDlgConvertToTrack::~CDlgConvertToTrack()
{
    QSettings cfg;
    cfg.setValue("overlay/convert/intervall", comboDelta->currentIndex());
    cfg.setValue("overlay/convert/noele", radioNoEle->isChecked());
    cfg.setValue("overlay/convert/local", radioEleFromLocal->isChecked());
    cfg.setValue("overlay/convert/remote", radioEleFromRemote->isChecked());
}


int CDlgConvertToTrack::getDelta()
{

    return comboDelta->itemData(comboDelta->currentIndex()).toInt();
}

CDlgConvertToTrack::EleMode_e CDlgConvertToTrack::getEleMode()
{
    if(radioNoEle->isChecked())
    {
        return eNone;
    }
    else if(radioEleFromLocal->isChecked())
    {
        return eLocal;
    }
    else if(radioEleFromRemote->isChecked())
    {
        return eRemote;
    }

    return eNone;
}
