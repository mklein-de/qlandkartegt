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
#include "COverlayToolWidget.h"
#include "COverlayDB.h"
#include "IOverlay.h"

#include <QtGui>

COverlayToolWidget::COverlayToolWidget(QTabWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("Overlay");

    parent->addTab(this,QIcon(":/icons/iconOverlay16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Draw"));

    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(listOverlays,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    connect(listOverlays,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
}


COverlayToolWidget::~COverlayToolWidget()
{

}


void COverlayToolWidget::slotDBChanged()
{
    listOverlays->clear();

    QMap<QString, IOverlay*>& overlays                  = COverlayDB::self().overlays;
    QMap<QString, IOverlay*>::const_iterator overlay    = overlays.begin();
    while(overlay != overlays.end()) {
        QListWidgetItem * item = new QListWidgetItem(listOverlays);
        item->setIcon((*overlay)->icon);
        item->setText((*overlay)->getInfo());
        item->setData(Qt::UserRole, overlay.key());
        ++overlay;
    }
}


void COverlayToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();
    QMap<QString,IOverlay*>& overlays = COverlayDB::self().overlays;
    if(!overlays.contains(key)) {
        return;
    }

    overlays[key]->makeVisible();
}


void COverlayToolWidget::slotContextMenu(const QPoint& pos)
{
    QListWidgetItem * item = listOverlays->currentItem();
    if(item) {
        QPoint p = listOverlays->mapToGlobal(pos);

        QMenu contextMenu;
        COverlayDB::self().customMenu(item->data(Qt::UserRole).toString(), contextMenu);
        if(contextMenu.isEmpty()) {
            contextMenu.addAction(QPixmap(),tr("<---->"));
        }
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);

        contextMenu.exec(p);
    }
}


void COverlayToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listOverlays->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    COverlayDB::self().delOverlays(keys);
}


void COverlayToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete) {
        slotDelete();
        e->accept();
    }
}
