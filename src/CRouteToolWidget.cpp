/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#include "CRouteToolWidget.h"
#include "CRoute.h"
#include "CRouteDB.h"
#include "IUnit.h"
#include "CMapDB.h"
#include "CDlgEditRoute.h"

#include <QtGui>

CRouteToolWidget::CRouteToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Routes");
    parent->addTab(this,QIcon(":/icons/iconRoute16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Routes"));

    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listRoutes,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listRoutes,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    connect(listRoutes,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
}


CRouteToolWidget::~CRouteToolWidget()
{

}


void CRouteToolWidget::slotDBChanged()
{
    if(originator) return;

    listRoutes->clear();

    QListWidgetItem * highlighted = 0;

    const QMap<QString,CRoute*>& routes         = CRouteDB::self().getRoutes();
    QMap<QString,CRoute*>::const_iterator route = routes.begin();
    while(route != routes.end())
    {
        QListWidgetItem * item = new QListWidgetItem(listRoutes);

        QString val1, unit1, val2, unit2;

        QString str     = (*route)->getName();
        double distance = (*route)->getDistance();

        IUnit::self().meter2distance(distance, val1, unit1);
        str += tr("\nlength: %1 %2").arg(val1).arg(unit1);

        item->setText(str);
        item->setData(Qt::UserRole, (*route)->getKey());
        item->setIcon((*route)->getIcon());

        if((*route)->isHighlighted())
        {
            highlighted = item;
        }

        ++route;
    }

    listRoutes->sortItems();

    if(highlighted)
    {
        listRoutes->setCurrentItem(highlighted);
    }
}


void CRouteToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    CRouteDB::self().highlightRoute(item->data(Qt::UserRole).toString());
    originator = false;
}


void CRouteToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();

    QRectF r = CRouteDB::self().getBoundingRectF(key);
    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}


void CRouteToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        slotDelete();
        e->accept();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}


void CRouteToolWidget::slotContextMenu(const QPoint& pos)
{
    QListWidgetItem * item = listRoutes->currentItem();
    if(item)
    {
        QPoint p = listRoutes->mapToGlobal(pos);

        QMenu contextMenu;
        contextMenu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit"),this,SLOT(slotEdit()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::CTRL + Qt::Key_Delete);
        contextMenu.exec(p);
    }
}


void CRouteToolWidget::slotEdit()
{
    QListWidgetItem * item = listRoutes->currentItem();
    if(item == 0) return;

    QString key     = item->data(Qt::UserRole).toString();
    CRoute* route   = CRouteDB::self().getRoute(key);
    if(route == 0) return;

    CDlgEditRoute dlg(*route, this);
    dlg.exec();
}


void CRouteToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listRoutes->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
        delete listRoutes->takeItem(listRoutes->row(item));
    }
    originator = true;
    CRouteDB::self().delRoutes(keys);
    originator = false;
}
