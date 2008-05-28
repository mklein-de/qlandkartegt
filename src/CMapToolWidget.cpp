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

#include "CMapToolWidget.h"
#include "CMapDB.h"
#include "CMainWindow.h"
#include "GeoMath.h"

#include <QtGui>

CMapToolWidget::CMapToolWidget(QTabWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("Maps");
    parent->addTab(this,QIcon(":/icons/iconMap16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Maps"));

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(listKnownMaps,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotKnownMapClicked(QListWidgetItem*)));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QPixmap(),tr(""));
    contextMenu->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Delete"),this,SLOT(slotDelete()));

    connect(listKnownMaps,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    connect(listSelectedMaps,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotSelectedMapClicked(QListWidgetItem*)));
}


CMapToolWidget::~CMapToolWidget()
{

}


void CMapToolWidget::slotDBChanged()
{
    listKnownMaps->clear();
    const QMap<QString,CMapDB::map_t>& knownMaps = CMapDB::self().getKnownMaps();
    {
        QMap<QString,CMapDB::map_t>::const_iterator map = knownMaps.begin();
        while(map != knownMaps.end()) {
            QListWidgetItem * item = new QListWidgetItem(listKnownMaps);
            item->setText(map->description);
            item->setData(Qt::UserRole, map.key());
            ++map;
        }
    }

    listSelectedMaps->clear();
    const QMap<QString,CMapDB::mapsel_t>& selectedMaps = CMapDB::self().getSelectedMaps();
    {
        QMap<QString,CMapDB::mapsel_t>::const_iterator map = selectedMaps.begin();
        while(map != selectedMaps.end()) {
            QListWidgetItem * item = new QListWidgetItem(listSelectedMaps);
            QString pos1, pos2;

            GPS_Math_Deg_To_Str(map->lon1 * RAD_TO_DEG, map->lat1 * RAD_TO_DEG, pos1);
            GPS_Math_Deg_To_Str(map->lon2 * RAD_TO_DEG, map->lat2 * RAD_TO_DEG, pos2);

            item->setText(QString("%1\n%2\n%3").arg(map->description).arg(pos1).arg(pos2));
            item->setData(Qt::UserRole, map.key());
            ++map;
        }

    }
}


void CMapToolWidget::slotKnownMapClicked(QListWidgetItem* item)
{
    QString key = item->data(Qt::UserRole).toString();
    CMapDB::self().openMap(key);
}

void CMapToolWidget::slotSelectedMapClicked(QListWidgetItem* item)
{
    QString key = item->data(Qt::UserRole).toString();

    const QMap<QString,CMapDB::mapsel_t>& selectedMaps = CMapDB::self().getSelectedMaps();
    if(selectedMaps.contains(key)){
        const CMapDB::mapsel_t& ms = selectedMaps[key];
        CMapDB::self().getMap().zoom(ms.lon1, ms.lat1, ms.lon2, ms.lat2);
    }
}

void CMapToolWidget::slotContextMenu(const QPoint& pos)
{
    if(listKnownMaps->currentItem()) {
        QPoint p = listKnownMaps->mapToGlobal(pos);
        contextMenu->exec(p);
    }
}


void CMapToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listKnownMaps->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CMapDB::self().delKnownMap(keys);
}
