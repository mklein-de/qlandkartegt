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

#include "CDlgProjWizzard.h"
#include "mitab.h"

#include <QtGui>

struct mitab_entry_t {QString name; int idx;};

static bool mitabLessThan(const mitab_entry_t &s1, const mitab_entry_t &s2)
{
    return s1.name < s2.name;
}

CDlgProjWizzard::CDlgProjWizzard(QLineEdit& line, QWidget * parent)
: QDialog(parent)
, line(line)
{
    setupUi(this);

    mitab_entry_t           entry;
    QList<mitab_entry_t>    list;
    int idx                 = 0;
    const MapInfoDatumInfo * di   = asDatumInfoList;

    while(di->nMapInfoDatumID != -1){
        entry.name  = di->pszOGCDatumName;
        entry.idx   = idx;
        list << entry;
        ++di;++idx;
    }
    qSort(list.begin(), list.end(), mitabLessThan);

    foreach(entry, list){
        comboDatum->addItem(entry.name, entry.idx);
    }

    connect(radioMercator, SIGNAL(clicked()), this, SLOT(slotChange()));
    connect(radioUTM, SIGNAL(clicked()), this, SLOT(slotChange()));
    connect(radioUserDef, SIGNAL(clicked()), this, SLOT(slotChange()));
    connect(comboDatum, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChange()));
    connect(lineUserDef, SIGNAL(textChanged(const QString&)), this, SLOT(slotChange()));
    connect(spinUTMZone, SIGNAL(valueChanged(int)), this, SLOT(slotChange()));
}

CDlgProjWizzard::~CDlgProjWizzard()
{

}

void CDlgProjWizzard::slotChange()
{
    QString str;
    if(radioMercator->isChecked()){
        str += "+proj=merc ";
    }
    else if(radioUTM->isChecked()){
        str += QString("+proj=utm +zone=%1 ").arg(spinUTMZone->value());
    }
    else if(radioUserDef->isChecked()){
        str += lineUserDef->text() + " ";
    }

    int idx = comboDatum->itemData(comboDatum->currentIndex()).toInt();
    const MapInfoDatumInfo    di = asDatumInfoList[idx];
    const MapInfoSpheroidInfo si = asSpheroidInfoList[di.nEllipsoid];

    str += QString("+a=%1 +b=%2 ").arg(si.dfA,0,'f',4).arg(si.dfA * (1.0 - (1.0/si.dfInvFlattening)),0,'f',4);
    str += QString("+towgs84=%1,%2,%3,%4,%5,%6,%7,%8 ").arg(di.dfShiftX).arg(di.dfShiftY).arg(di.dfShiftZ).arg(di.dfDatumParm0).arg(di.dfDatumParm1).arg(di.dfDatumParm2).arg(di.dfDatumParm3).arg(di.dfDatumParm4);
    str += "+units=m  +no_defs";

    labelResult->setText(str);
}

void CDlgProjWizzard::accept()
{
    line.setText(labelResult->text());
    QDialog::accept();
}
