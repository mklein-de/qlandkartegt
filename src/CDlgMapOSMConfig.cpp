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
#include "CMapOSMType.h"
#include <QtGui>

CDlgMapOSMConfig::CDlgMapOSMConfig(CMapOSM * map)
    : map(map)
{
    QList<CMapOSMType*> list;
    list=map->getServerList();

    setupUi(this);

    QList<QString> header;
    header << tr("Name") << tr("Path") << tr("Key");

    mapsTreeWidget->setHeaderLabels(header);

    mapsTreeWidget->setColumnCount(3);
    mapsTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);


    topBuiltin = new QTreeWidgetItem(mapsTreeWidget);
    topBuiltin->setText(0, tr("Built-in maps"));

    topCustom = new QTreeWidgetItem(mapsTreeWidget);
    topCustom->setText(0, tr("Custom maps"));

    CMapOSMType* currentMap;
    for (int i = 0; i < list.size(); ++i)
    {
        currentMap=list.at(i);
        QTreeWidgetItem *cm = new QTreeWidgetItem(currentMap->isBuiltin() ? topBuiltin : topCustom);
        cm->setFlags(cm->flags()|Qt::ItemIsSelectable);
        cm->setText(0, currentMap->title);
        cm->setText(1, currentMap->path);
        if (currentMap->isBuiltin())
        {
            cm->setText(2, currentMap->key);
            cm->setFlags(cm->flags()|Qt::ItemIsEditable);
        }
    }

    topBuiltin->sortChildren(0,Qt::AscendingOrder);
    topCustom->sortChildren(0,Qt::AscendingOrder);

}

CDlgMapOSMConfig::~CDlgMapOSMConfig()
{
}

void CDlgMapOSMConfig::accept()
{
    QList<CMapOSMType*> list;
    QSettings cfg;

//    QTreeWidgetItemIterator itAll(mapsTreeWidget,QTreeWidgetItemIterator::NoChildren);
//    while (*itAll) {


//        list << qMakePair(QString((*itAll)->text(0)),QString((*itAll)->text(1)));
//        ++itAll;
//    }

//    QTreeWidgetItemIterator itCustom(topCustom,QTreeWidgetItemIterator::NoChildren);
//    cfg.beginWriteArray("osm/customMaps");
//    int counter=0;
//    while (*itCustom) {
//        cfg.setArrayIndex(counter++);
//        cfg.setValue("mapName", (*itCustom)->text(0));
//        cfg.setValue("mapString", (*itCustom)->text(1));
//        ++itCustom;
//    }
//    cfg.endArray();


//    map->setServerList(list);
    QDialog::accept();
}

