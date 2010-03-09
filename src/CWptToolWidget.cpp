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
#include "CMapDB.h"

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

    listWpts->setSortingEnabled(false);

    contextMenu     = new QMenu(this);
    actCopyPos      = contextMenu->addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Position"),this,SLOT(slotCopyPosition()),Qt::CTRL + Qt::Key_C);
    actEdit         = contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    actProximity    = contextMenu->addAction(QPixmap(":/icons/iconProximity16x16.png"),tr("Proximity ..."),this,SLOT(slotProximity()));
    actMakeRte      = contextMenu->addAction(QPixmap(":/icons/iconRoute16x16.png"),tr("Make Route ..."),this,SLOT(slotMakeRoute()));
    actZoomToFit    = contextMenu->addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFit()));
    actDelete       = contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);
    actDeleteBy     = contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete by ..."),this,SLOT(slotDeleteBy()));

    connect(listWpts,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

}


CWptToolWidget::~CWptToolWidget()
{

}


void CWptToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        slotDelete();
        e->accept();
    }
    else if(e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
    {
        slotCopyPosition();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}

bool keyLessThan(CWptDB::keys_t&  s1, CWptDB::keys_t&  s2)
{
    return s1.name.toLower() < s2.name.toLower();
}

void CWptToolWidget::slotDBChanged()
{
    listWpts->clear();

    CWptDB::keys_t key;
    QList<CWptDB::keys_t> keys = CWptDB::self().keys();
    qSort(keys.begin(), keys.end(), keyLessThan);

    foreach(key, keys)
    {
        CWpt * wpt = CWptDB::self().getWptByKey(key.key);

        QListWidgetItem * item = new QListWidgetItem(listWpts);
        if(wpt->sticky)
        {
            item->setText(wpt->name + tr(" (sticky)"));
        }
        else
        {
            item->setText(wpt->name);
        }

        item->setIcon(getWptIconByName(wpt->icon));
        item->setData(Qt::UserRole, wpt->key());
    }

}


void CWptToolWidget::slotItemClicked(QListWidgetItem* item)
{
    CWpt * wpt = CWptDB::self().getWptByKey(item->data(Qt::UserRole).toString());
    if(wpt)
    {
        theMainWindow->getCanvas()->move(wpt->lon, wpt->lat);
    }
}


void CWptToolWidget::slotContextMenu(const QPoint& pos)
{
    int cnt = listWpts->selectedItems().count();

    if(cnt > 0)
    {
        if(cnt > 1)
        {
            actCopyPos->setEnabled(false);
            actEdit->setEnabled(false);
            actMakeRte->setEnabled(true);
        }
        else
        {
            actCopyPos->setEnabled(true);
            actEdit->setEnabled(true);
            actMakeRte->setEnabled(false);
        }

        QPoint p = listWpts->mapToGlobal(pos);
        contextMenu->exec(p);
    }
}


void CWptToolWidget::slotEdit()
{
    CWpt * wpt = CWptDB::self().getWptByKey(listWpts->currentItem()->data(Qt::UserRole).toString());
    if(wpt)
    {
        CDlgEditWpt dlg(*wpt,this);
        dlg.exec();
    }
}


void CWptToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listWpts->selectedItems();
    foreach(item,items)
    {
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

void CWptToolWidget::slotZoomToFit()
{

    const QList<QListWidgetItem*>& items = listWpts->selectedItems();
    QList<QListWidgetItem*>::const_iterator item = items.begin();

    CWpt * wpt = CWptDB::self().getWptByKey((*item)->data(Qt::UserRole).toString());

    QRectF r(wpt->lon, wpt->lat, 0.001, 0.001);

    while(item != items.end())
    {
        wpt = CWptDB::self().getWptByKey((*item)->data(Qt::UserRole).toString());
        r |= QRectF(wpt->lon, wpt->lat, 0.001, 0.001);
        ++item;
    }

    qDebug() << "ggggggggggg" << r;

    CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
}


void CWptToolWidget::selWptByKey(const QString& key)
{
    for(int i=0; i<listWpts->count(); ++i)
    {
        QListWidgetItem * item = listWpts->item(i);
        if(item && item->data(Qt::UserRole) == key)
        {
            listWpts->setCurrentItem(item);
        }
    }
}


void CWptToolWidget::slotProximity()
{
    bool ok         = false;
    QString str    = tr("Distance [%1]").arg(IUnit::self().baseunit);
    const QList<QListWidgetItem*>& items = listWpts->selectedItems();
    QListWidgetItem * item;
    double prx = 0.0;
    // If no item is selected, or multiple items, don't try to provide
    // an initial value to the dialog.
    if (items.count() == 1)
    {
        item = items.first();
	QString key = item->data(Qt::UserRole).toString();
	CWpt *pt    = CWptDB::self().getWptByKey(key);

	if (pt->prx != WPT_NOFLOAT)
	{
	    prx  = pt->prx * IUnit::self().basefactor;
	}
    }
    double dist     = QInputDialog::getDouble(0,tr("Proximity distance ..."), str, prx, 0, 2147483647, 0,&ok);
    if(ok)
    {
        str = QString("%1 %2").arg(dist).arg(IUnit::self().baseunit);
        dist = IUnit::self().str2distance(str);

        QStringList keys;
        foreach(item,items)
        {
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
    foreach(item,items)
    {
        CWpt * wpt = CWptDB::self().getWptByKey(item->data(Qt::UserRole).toString());
        if(wpt)
        {
            route->addPosition(wpt->lon, wpt->lat);
        }
    }

    CRouteDB::self().addRoute(route, false);
}
