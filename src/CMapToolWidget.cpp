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
#include "CMapQMAPExport.h"

#include <QtGui>

CMapToolWidget::CMapToolWidget(QTabWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("Maps");
    parent->addTab(this,QIcon(":/icons/iconMap16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Maps"));

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));


    contextMenuKnownMaps = new QMenu(this);
    contextMenuKnownMaps->addAction(QPixmap(),tr("<---->"));
    contextMenuKnownMaps->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteKnownMap()));
    connect(listKnownMaps,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenuKnownMaps(const QPoint&)));
    connect(listKnownMaps,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotKnownMapClicked(QListWidgetItem*)));

    contextMenuSelectedMaps = new QMenu(this);
    contextMenuSelectedMaps->addAction(QPixmap(),tr("<---->"));
    contextMenuSelectedMaps->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteSelectedMap()));
    connect(listSelectedMaps,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenuSelectedMaps(const QPoint&)));
    connect(listSelectedMaps,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotSelectedMapClicked(QListWidgetItem*)));
    connect(listSelectedMaps,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotSelectMap(QListWidgetItem*)));

    connect(pushExportMap, SIGNAL(clicked()), this, SLOT(slotExportMap()));
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
    listKnownMaps->sortItems();

    listSelectedMaps->clear();
    const QMap<QString,CMapSelection>& selectedMaps = CMapDB::self().getSelectedMaps();
    {
        QListWidgetItem * selected = 0;
        QMap<QString,CMapSelection>::const_iterator map = selectedMaps.begin();
        while(map != selectedMaps.end()) {
            QListWidgetItem * item = new QListWidgetItem(listSelectedMaps);
            QString pos1, pos2;

            GPS_Math_Deg_To_Str(map->lon1 * RAD_TO_DEG, map->lat1 * RAD_TO_DEG, pos1);
            GPS_Math_Deg_To_Str(map->lon2 * RAD_TO_DEG, map->lat2 * RAD_TO_DEG, pos2);

            item->setText(QString("%1\n%2\n%3").arg(map->description).arg(pos1).arg(pos2));
            item->setData(Qt::UserRole, map.key());

            if(CMapSelection::focusedMap == map.key()) selected = item;
            ++map;
        }

        if(selected) listSelectedMaps->setCurrentItem(selected);
        updateEportButton();
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

    const QMap<QString,CMapSelection>& selectedMaps = CMapDB::self().getSelectedMaps();
    if(selectedMaps.contains(key)){
        const CMapSelection& ms = selectedMaps[key];
        CMapDB::self().getMap().zoom(ms.lon1, ms.lat1, ms.lon2, ms.lat2);
        CMapDB::self().selSelectedMap(key);
    }

}

void CMapToolWidget::slotContextMenuKnownMaps(const QPoint& pos)
{
    if(listKnownMaps->currentItem()) {
        QPoint p = listKnownMaps->mapToGlobal(pos);
        contextMenuKnownMaps->exec(p);
    }
}

void CMapToolWidget::slotContextMenuSelectedMaps(const QPoint& pos)
{
    if(listSelectedMaps->currentItem()) {
        QPoint p = listSelectedMaps->mapToGlobal(pos);
        contextMenuSelectedMaps->exec(p);
    }
}

void CMapToolWidget::slotDeleteKnownMap()
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

void CMapToolWidget::slotDeleteSelectedMap()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listSelectedMaps->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CMapDB::self().delSelectedMap(keys);

    updateEportButton();
}

void CMapToolWidget::slotSelectMap(QListWidgetItem* item)
{
    const QMap<QString,CMapSelection>& selectedMaps = CMapDB::self().getSelectedMaps();
    QString key = item->data(Qt::UserRole).toString();
    if(selectedMaps.contains(key)){
        CMapSelection::focusedMap = key;
        theMainWindow->getCanvas()->update();
    }
    updateEportButton();
}

void CMapToolWidget::updateEportButton()
{
    pushExportMap->setEnabled(listSelectedMaps->currentItem() != 0);
}

void CMapToolWidget::slotExportMap()
{
    bool haveGDALWarp       = QProcess::execute("gdalwarp --version") == 0;
    bool haveGDALTranslate  = QProcess::execute("gdal_translate --version") == 0;
    bool haveGDAL = haveGDALWarp && haveGDALTranslate;
    if(!haveGDAL){
        QMessageBox::critical(0,tr("Error export maps..."), tr("You need to have the GDAL toolchain installed in your path."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    QListWidgetItem * item  = listSelectedMaps->currentItem();
    if(item == 0) return;

    QString key             = item->data(Qt::UserRole).toString();
    if(!CMapDB::self().getSelectedMaps().contains(key)) return;

    CMapQMAPExport dlg(CMapDB::self().getSelectedMaps()[key],this);
    dlg.exec();
}
