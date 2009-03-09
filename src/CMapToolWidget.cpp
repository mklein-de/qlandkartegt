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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMapToolWidget.h"
#include "CMapDB.h"
#include "CMainWindow.h"
#include "GeoMath.h"
#include "CMapQMAPExport.h"
#include "CMapSelectionRaster.h"
#include "CGarminExport.h"
#include "CMapSelectionGarmin.h"

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
    actAddDEM = contextMenuKnownMaps->addAction(QPixmap(":/icons/iconDEM16x16.png"),tr("Add DEM..."),this,SLOT(slotAddDEM()));
    actDelDEM = contextMenuKnownMaps->addAction(QPixmap(":/icons/iconNoDEM16x16.png"),tr("Del. DEM..."),this,SLOT(slotDelDEM()));
    actDelMap = contextMenuKnownMaps->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteKnownMap()));
    connect(treeKnownMaps,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenuKnownMaps(const QPoint&)));
    connect(treeKnownMaps,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(slotKnownMapClicked(QTreeWidgetItem*, int)));
    connect(treeKnownMaps,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(slotKnownMapDoubleClicked(QTreeWidgetItem*, int)));

    contextMenuSelectedMaps = new QMenu(this);
    contextMenuSelectedMaps->addAction(QPixmap(":/icons/iconFileSave16x16.png"),tr("Export"),this,SLOT(slotExportMap()));
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
    IMap& basemap               = CMapDB::self().getMap();
    QString key                 = basemap.getKey();
    QTreeWidgetItem * selected  = 0;

    treeKnownMaps->clear();
    const QMap<QString,CMapDB::map_t>& knownMaps = CMapDB::self().getKnownMaps();
    {
        QMap<QString,CMapDB::map_t>::const_iterator map = knownMaps.begin();
        while(map != knownMaps.end()) {
            QTreeWidgetItem * item = new QTreeWidgetItem(treeKnownMaps);

            item->setText(eName, map->description);
            item->setData(eName, Qt::UserRole, map.key());
            item->setIcon(eType, map->type == IMap::eRaster ? QIcon(":/icons/iconRaster16x16") : map->type == IMap::eGarmin ? QIcon(":/icons/iconVector16x16") : QIcon(":/icons/iconUnknown16x16"));
            item->setData(eType, Qt::UserRole, map->type);

            if(map.key() == key) {
                selected = item;
                item->setIcon(eMode, QIcon(QIcon(":/icons/iconOk16x16")));
                item->setData(eMode, Qt::UserRole, eSelected);
            }
            else if(basemap.hasOverlayMap(map.key())) {
                item->setIcon(eMode, QIcon(QIcon(":/icons/iconOvlOk16x16")));
                item->setData(eMode, Qt::UserRole, eOverlayActive);
            }
            else if(map->type == IMap::eGarmin) {
                item->setIcon(eMode, QIcon(QIcon(":/icons/iconOvl16x16")));
                item->setData(eMode, Qt::UserRole, eOverlay);
            }
            else {
                item->setData(eMode, Qt::UserRole, eNoMode);
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
    const QMap<QString,IMapSelection*>& selectedMaps = CMapDB::self().getSelectedMaps();
    {
        QListWidgetItem * selected = 0;
        QMap<QString,IMapSelection*>::const_iterator map = selectedMaps.begin();
        while(map != selectedMaps.end()) {
            QListWidgetItem * item = new QListWidgetItem(listSelectedMaps);

            item->setText((*map)->description);
            item->setData(Qt::UserRole, (*map)->key);

            if(IMapSelection::focusedMap == (*map)->key) selected = item;
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
    if(c == eMode) {
        QString key = item->data(eName, Qt::UserRole).toString();

        if(item->data(eMode, Qt::UserRole).toInt() == eOverlay) {
            CMapDB::self().getMap().addOverlayMap(key);
        }
        else if(item->data(eMode, Qt::UserRole).toInt() == eOverlayActive) {
            CMapDB::self().getMap().delOverlayMap(key);
        }

        emit sigChanged();
    }
}


void CMapToolWidget::slotSelectedMapClicked(QListWidgetItem* item)
{
    QString key = item->data(Qt::UserRole).toString();

    const QMap<QString,IMapSelection*>& selectedMaps = CMapDB::self().getSelectedMaps();
    if(selectedMaps.contains(key)) {
        const IMapSelection * ms = selectedMaps[key];

        CMapDB::self().getMap().zoom(ms->lon1, ms->lat1, ms->lon2, ms->lat2);
        CMapDB::self().selSelectedMap(key);
    }

}


void CMapToolWidget::slotContextMenuKnownMaps(const QPoint& pos)
{
    QTreeWidgetItem * item = treeKnownMaps->currentItem();
    if(item) {
        IMap& dem = CMapDB::self().getDEM();

        if(item->data(eMode, Qt::UserRole).toInt() == eSelected) {
            actAddDEM->setEnabled(true);
            actDelDEM->setEnabled(dem.maptype == IMap::eDEM);
            actDelMap->setEnabled(false);
        }
        else {
            actAddDEM->setEnabled(false);
            actDelDEM->setEnabled(false);
            actDelMap->setEnabled(true);
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
    const QMap<QString,IMapSelection*>& selectedMaps = CMapDB::self().getSelectedMaps();
    QString key = item->data(Qt::UserRole).toString();
    if(selectedMaps.contains(key)) {
        IMapSelection::focusedMap = key;
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

    QListWidgetItem * item  = listSelectedMaps->currentItem();
    if(item == 0) return;

    QString key             = item->data(Qt::UserRole).toString();
    if(!CMapDB::self().getSelectedMaps().contains(key)) return;

    const QMap<QString,IMapSelection*>& selectedMaps = CMapDB::self().getSelectedMaps();
    const IMapSelection * ms = selectedMaps[key];
    if(ms->type == IMapSelection::eRaster) {
        bool haveGDALWarp       = QProcess::execute("gdalwarp --version") == 0;
        bool haveGDALTranslate  = QProcess::execute("gdal_translate --version") == 0;
        bool haveGDAL = haveGDALWarp && haveGDALTranslate;
        if(!haveGDAL) {
            QMessageBox::critical(0,tr("Error export maps..."), tr("You need to have the GDAL toolchain installed in your path."), QMessageBox::Abort, QMessageBox::Abort);
            return;
        }

        CMapQMAPExport dlg((CMapSelectionRaster&)*ms,this);
        dlg.exec();
    }
    if(ms->type == IMapSelection::eGarmin) {
        CGarminExport dlg(this);
        dlg.exportToFile((CMapSelectionGarmin&)*ms);
    }
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
    if(dem.maptype == IMap::eDEM) {
        QSettings cfg;
        cfg.setValue(QString("map/dem/%1").arg(CMapDB::self().getMap().getKey()), "");
        dem.deleteLater();
    }
}
