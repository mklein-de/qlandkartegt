/**********************************************************************************************
    Copyright (C) 2011 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgMapOSMConfig.h"
#include "CMapOSM.h"

CDlgMapOSMConfig::CDlgMapOSMConfig(CMapOSM * map)
    : map(map)
{
    QList<QPair<QString, QString> > list;
    list=map->getServerList();

    setupUi(this);

    QList<QString> header;
    header << tr("Name") << tr("Path");

    mapsTableWidget->setRowCount(list.size());
    mapsTableWidget->setColumnCount(2);
    mapsTableWidget->setHorizontalHeaderLabels(header);

    for (int i = 0; i < list.size(); ++i)
    {
        mapsTableWidget->setItem(i,0,new QTableWidgetItem(list.at(i).first));
        mapsTableWidget->setItem(i,1,new QTableWidgetItem(list.at(i).second));
    }

}

CDlgMapOSMConfig::~CDlgMapOSMConfig()
{
}

void CDlgMapOSMConfig::accept()
{
    QList<QPair<QString, QString> > list;
    for (int i=0; i<mapsTableWidget->rowCount(); ++i)
    {
        list << qMakePair(QString(mapsTableWidget->item(i, 0)->text()),QString(mapsTableWidget->item(i, 1)->text()));
    }
    map->setServerList(list);
    QDialog::accept();
}

