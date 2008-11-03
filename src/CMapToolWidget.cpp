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
, path("./")
{
    setupUi(this);
    setObjectName("Maps");
    parent->addTab(this,QIcon(":/icons/iconMap16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Maps"));

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    contextMenuKnownMaps = new QMenu(this);
    contextMenuKnownMaps->addAction(QPixmap(":/icons/iconDEM16x16.png"),tr("Add DEM..."),this,SLOT(slotAddDEM()));
    actDelDEM = contextMenuKnownMaps->addAction(QPixmap(":/icons/iconNoDEM16x16.png"),tr("Del. DEM..."),this,SLOT(slotDelDEM()));
    contextMenuKnownMaps->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteKnownMap()));
    connect(treeKnownMaps,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenuKnownMaps(const QPoint&)));
    connect(treeKnownMaps,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(slotKnownMapClicked(QTreeWidgetItem*, int)));
    connect(treeKnownMaps,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(slotKnownMapDoubleClicked(QTreeWidgetItem*, int)));

    actDelDEM->setEnabled(false);

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
    QString key                 = CMapDB::self().getMap().getKey();
    QTreeWidgetItem * selected  = 0;

    treeKnownMaps->clear();
    const QMap<QString,CMapDB::map_t>& knownMaps = CMapDB::self().getKnownMaps();
    {
        QMap<QString,CMapDB::map_t>::const_iterator map = knownMaps.begin();
        while(map != knownMaps.end()) {
            QTreeWidgetItem * item = new QTreeWidgetItem(treeKnownMaps);
            item->setText(eName, map->description);
            item->setData(eName, Qt::UserRole, map.key());
            item->setIcon(eType, map->type == IMap::eRaster ? QIcon(":/icons/iconRaster16x16") : map->type == IMap::eVector ? QIcon(":/icons/iconVector16x16") : QIcon(":/icons/iconUnknown16x16"));
            item->setData(eType, Qt::UserRole, map->type);
            if(map.key() == key){
                selected = item;
                item->setIcon(eMode, QIcon(QIcon(":/icons/iconOk16x16")));
                item->setData(eMode, Qt::UserRole, true);
            }
            else{
                item->setData(eMode, Qt::UserRole, false);
            }
            ++map;
        }
    }
    treeKnownMaps->sortItems(eName, Qt::AscendingOrder);
    if(selected)treeKnownMaps->setCurrentItem(selected);

    // adjust column sizes to fit
    treeKnownMaps->header()->setResizeMode(0,QHeaderView::Interactive);
    for(int i=0; i < eMaxColumn - 1; ++i) {
        treeKnownMaps->resizeColumnToContents(i);
    }

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
        updateExportButton();
    }


}


void CMapToolWidget::slotKnownMapDoubleClicked(QTreeWidgetItem* item, int)
{
    QString key = item->data(eName, Qt::UserRole).toString();
    CMapDB::self().openMap(key);
}

void CMapToolWidget::slotKnownMapClicked(QTreeWidgetItem* item, int c)
{
    if(c == eMode){
        QString key = item->data(eName, Qt::UserRole).toString();
        CMapDB::self().getMap().addOverlayMap(key);
    }
}


void CMapToolWidget::slotSelectedMapClicked(QListWidgetItem* item)
{
    QString key = item->data(Qt::UserRole).toString();

    const QMap<QString,CMapSelection>& selectedMaps = CMapDB::self().getSelectedMaps();
    if(selectedMaps.contains(key)) {
        const CMapSelection& ms = selectedMaps[key];
        CMapDB::self().getMap().zoom(ms.lon1, ms.lat1, ms.lon2, ms.lat2);
        CMapDB::self().selSelectedMap(key);
    }

}


void CMapToolWidget::slotContextMenuKnownMaps(const QPoint& pos)
{
    QTreeWidgetItem * item = treeKnownMaps->currentItem();
    if(item) {
        IMap& dem = CMapDB::self().getDEM();

        if(dem.maptype == IMap::eDEM && item->data(eMode, Qt::UserRole).toBool()){
            actDelDEM->setEnabled(true);
        }
        else{
            actDelDEM->setEnabled(false);
        }

        QPoint p = treeKnownMaps->mapToGlobal(pos);
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
    QTreeWidgetItem * item;
    const QList<QTreeWidgetItem*>& items = treeKnownMaps->selectedItems();
    foreach(item,items) {
        keys << item->data(eName, Qt::UserRole).toString();
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

    updateExportButton();
}


void CMapToolWidget::slotSelectMap(QListWidgetItem* item)
{
    const QMap<QString,CMapSelection>& selectedMaps = CMapDB::self().getSelectedMaps();
    QString key = item->data(Qt::UserRole).toString();
    if(selectedMaps.contains(key)) {
        CMapSelection::focusedMap = key;
        theMainWindow->getCanvas()->update();
    }
    updateExportButton();
}


void CMapToolWidget::updateExportButton()
{
    pushExportMap->setEnabled(listSelectedMaps->currentItem() != 0);
}


void CMapToolWidget::slotExportMap()
{
    bool haveGDALWarp       = QProcess::execute("gdalwarp --version") == 0;
    bool haveGDALTranslate  = QProcess::execute("gdal_translate --version") == 0;
    bool haveGDAL = haveGDALWarp && haveGDALTranslate;
    if(!haveGDAL) {
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

void CMapToolWidget::slotAddDEM()
{
    QSettings cfg;
    path = QDir(cfg.value("path/DEM",path.path()).toString());

    QString filename = QFileDialog::getOpenFileName(0, tr("Select DEM file..."),path.path(), tr("16bit GeoTiff (*.tif)"));
    if(filename.isEmpty()) return;

    QFileInfo fi(filename);
    path = QDir(fi.absolutePath());
    cfg.setValue("path/DEM",path.path());

    CMapDB::self().openDEM(filename);
}

void CMapToolWidget::slotDelDEM()
{
    IMap& dem = CMapDB::self().getDEM();
    if(dem.maptype == IMap::eDEM){
        QSettings cfg;
        cfg.setValue(QString("map/dem/%1").arg(CMapDB::self().getMap().getKey()), "");
        dem.deleteLater();
    }
}
