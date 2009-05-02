/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CWptToolWidget.h"
#include "WptIcons.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "CRoute.h"
#include "CRouteDB.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CDlgEditWpt.h"
#include "CDlgDelWpt.h"
#include "GeoMath.h"
#include "IUnit.h"

#include <QtGui>

CWptToolWidget::CWptToolWidget(QTabWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("Waypoints");
    parent->addTab(this,QIcon(":/icons/iconWaypoint16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Waypoints"));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(listWpts,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotItemClicked(QListWidgetItem*)));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Position"),this,SLOT(slotCopyPosition()),Qt::CTRL + Qt::Key_C);
    contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    contextMenu->addAction(QPixmap(":/icons/iconProximity16x16.png"),tr("Proximity ..."),this,SLOT(slotProximity()));
    contextMenu->addAction(QPixmap(":/icons/iconRoute16x16.png"),tr("Make Route ..."),this,SLOT(slotMakeRoute()));
    contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);
    contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete by ..."),this,SLOT(slotDeleteBy()));

    connect(listWpts,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

}


CWptToolWidget::~CWptToolWidget()
{

}


void CWptToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete) {
        slotDelete();
        e->accept();
    }
    else if(e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier) {
        slotCopyPosition();
    }
    else {
        QWidget::keyPressEvent(e);
    }
}


void CWptToolWidget::slotDBChanged()
{
    listWpts->clear();

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()) {
        QListWidgetItem * item = new QListWidgetItem(listWpts);
        if((*wpt)->sticky) {
            item->setText((*wpt)->name + tr(" (sticky)"));
        }
        else {
            item->setText((*wpt)->name);
        }

        item->setIcon(getWptIconByName((*wpt)->icon));
        item->setData(Qt::UserRole, (*wpt)->key());
        ++wpt;
    }
}


void CWptToolWidget::slotItemClicked(QListWidgetItem* item)
{
    CWpt * wpt = CWptDB::self().getWptByKey(item->data(Qt::UserRole).toString());
    if(wpt) {
        theMainWindow->getCanvas()->move(wpt->lon, wpt->lat);
    }
}


void CWptToolWidget::slotContextMenu(const QPoint& pos)
{
    if(listWpts->currentItem()) {
        QPoint p = listWpts->mapToGlobal(pos);
        contextMenu->exec(p);
    }
}


void CWptToolWidget::slotEdit()
{
    CWpt * wpt = CWptDB::self().getWptByKey(listWpts->currentItem()->data(Qt::UserRole).toString());
    if(wpt) {
        CDlgEditWpt dlg(*wpt,this);
        dlg.exec();
    }
}


void CWptToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listWpts->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CWptDB::self().delWpt(keys, false);
}


void CWptToolWidget::slotDeleteBy()
{
    CDlgDelWpt dlg(this);
    dlg.exec();
}


void CWptToolWidget::slotCopyPosition()
{
    QListWidgetItem * item = listWpts->currentItem();
    if(item == 0) return;
    CWpt * wpt = CWptDB::self().getWptByKey(item->data(Qt::UserRole).toString());
    if(wpt == 0) return;

    QString position;
    GPS_Math_Deg_To_Str(wpt->lon, wpt->lat, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}


void CWptToolWidget::selWptByKey(const QString& key)
{
    for(int i=0; i<listWpts->count(); ++i) {
        QListWidgetItem * item = listWpts->item(i);
        if(item && item->data(Qt::UserRole) == key) {
            listWpts->setCurrentItem(item);
        }
    }
}

void CWptToolWidget::slotProximity()
{
    bool ok         = false;
    QString str    = tr("Distance [%1]").arg(IUnit::self().baseunit);
    double dist     = QInputDialog::getDouble(0,tr("Proximity distance ..."), str, 0, 0, 2147483647, 0,&ok);
    if(ok) {
        str = QString("%1 %2").arg(dist).arg(IUnit::self().baseunit);
        dist = IUnit::self().str2distance(str);

        QStringList keys;
        QListWidgetItem * item;
        const QList<QListWidgetItem*>& items = listWpts->selectedItems();
        foreach(item,items) {
            keys << item->data(Qt::UserRole).toString();
        }
        CWptDB::self().setProxyDistance(keys,(dist == 0 ? WPT_NOFLOAT : dist));
    }
}

void CWptToolWidget::slotMakeRoute()
{
    const QList<QListWidgetItem*>& items = listWpts->selectedItems();
    if(items.count() < 2) return;

    CRoute * route = new CRoute(&CRouteDB::self());

    QListWidgetItem * item;
    foreach(item,items) {
        CWpt * wpt = CWptDB::self().getWptByKey(item->data(Qt::UserRole).toString());
        if(wpt){
            route->addPosition(wpt->lon, wpt->lat);
        }
    }

    CRouteDB::self().addRoute(route, false);
}
