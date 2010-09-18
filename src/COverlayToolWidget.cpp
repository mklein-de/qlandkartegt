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
#include "COverlayToolWidget.h"
#include "COverlayDB.h"
#include "COverlayDistanceEditWidget.h"
#include "IOverlay.h"
#include "CMapDB.h"

#include <QtGui>

COverlayToolWidget::COverlayToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Overlay");

    parent->addTab(this,QIcon(":/icons/iconOverlay16x16.png"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Draw"));

    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(&COverlayDB::self(), SIGNAL(sigModified()), this, SLOT(slotDBChanged()));
    connect(listOverlays,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect(listOverlays,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listOverlays,SIGNAL(itemSelectionChanged()),this,SLOT(slotSelectionChanged()));

    connect(listOverlays,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
}


COverlayToolWidget::~COverlayToolWidget()
{

}


void COverlayToolWidget::slotDBChanged()
{
    if(originator) return;

    listOverlays->clear();

    QMap<QString, IOverlay*>& overlays  = COverlayDB::self().overlays;
    QList<COverlayDB::keys_t> keys      = COverlayDB::self().keys();

    COverlayDB::keys_t key;

    foreach(key, keys)
    {
        IOverlay * ovl = overlays[key.key];

        QListWidgetItem * item = new QListWidgetItem(listOverlays);
        item->setIcon(ovl->getIcon());
        item->setText(ovl->getInfo());
        item->setToolTip(ovl->getComment());
        item->setData(Qt::UserRole, ovl->getKey());

    }

    listOverlays->sortItems();
}


void COverlayToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QStringList keys;
    keys << item->data(Qt::UserRole).toString();
    COverlayDB::self().makeVisible(keys);
}

void COverlayToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    COverlayDB::self().highlightOverlay(item->data(Qt::UserRole).toString());
    originator = false;
}

void COverlayToolWidget::slotSelectionChanged()
{
    if(listOverlays->hasFocus() && listOverlays->selectedItems().isEmpty())
    {
        COverlayDB::self().highlightOverlay("");
        if(overlayDistanceEditWidget)
        {
            delete overlayDistanceEditWidget;
        }
    }
}


void COverlayToolWidget::slotContextMenu(const QPoint& pos)
{
    QListWidgetItem * item = listOverlays->currentItem();
    if(item)
    {
        originator = true;
        COverlayDB::self().highlightOverlay(item->data(Qt::UserRole).toString());
        originator = false;

        QPoint p = listOverlays->mapToGlobal(pos);

        QMenu contextMenu;
        COverlayDB::self().customMenu(item->data(Qt::UserRole).toString(), contextMenu);
        contextMenu.addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFit()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::CTRL + Qt::Key_Delete);

        contextMenu.exec(p);
    }
}


void COverlayToolWidget::slotZoomToFit()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listOverlays->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    COverlayDB::self().makeVisible(keys);
}


void COverlayToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listOverlays->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    COverlayDB::self().delOverlays(keys);
}


void COverlayToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        slotDelete();
        e->accept();
    }
}
