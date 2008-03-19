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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CWptToolWidget.h"
#include "WptIcons.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CDlgEditWpt.h"
#include "GeoMath.h"

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
    contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    contextMenu->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);
    contextMenu->addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Position"),this,SLOT(slotCopyPosition()),Qt::CTRL + Qt::Key_C);

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
    else if(e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier){
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
        item->setText((*wpt)->name);
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
    CWptDB::self().delWpt(keys);
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
