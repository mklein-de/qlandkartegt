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
    MapInfoDatumInfo * di   = asDatumInfoList;

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
}

CDlgProjWizzard::~CDlgProjWizzard()
{

}


void CDlgProjWizzard::accept()
{

    QDialog::accept();
}
