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

CWptToolWidget::sortmode_e CWptToolWidget::sortmode = CWptToolWidget::eSortByName;
QString CWptToolWidget::sortpos;

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


    toolSortAlpha->setIcon(QPixmap(":/icons/iconDec16x16.png"));
    toolSortComment->setIcon(QPixmap(":/icons/iconText16x16.png"));
    toolSortIcon->setIcon(QPixmap(":/icons/iconWaypoint16x16.png"));
    toolSortPosition->setIcon(QPixmap(":/icons/iconLiveLog16x16.png"));

    connect(toolSortAlpha, SIGNAL(clicked()), this, SLOT(slotDBChanged()));
    connect(toolSortComment, SIGNAL(clicked()), this, SLOT(slotDBChanged()));
    connect(toolSortIcon, SIGNAL(clicked()), this, SLOT(slotDBChanged()));
    connect(toolSortPosition, SIGNAL(clicked()), this, SLOT(slotDBChanged()));

    connect(linePosition, SIGNAL(textChanged(const QString&)), this, SLOT(slotPosTextChanged(const QString&)));

    QSettings cfg;
    toolSortAlpha->setChecked(cfg.value("waypoint/sortAlpha", true).toBool());
    toolSortComment->setChecked(cfg.value("waypoint/sortComment", true).toBool());
    toolSortIcon->setChecked(cfg.value("waypoint/sortIcon", false).toBool());
    toolSortPosition->setChecked(cfg.value("waypoint/sortPosition", false).toBool());
    linePosition->setText(cfg.value("waypoint/position",tr("enter valid position")).toString());

}


CWptToolWidget::~CWptToolWidget()
{
    QSettings cfg;
    cfg.setValue("waypoint/sortAlpha", toolSortAlpha->isChecked());
    cfg.setValue("waypoint/sortComment", toolSortComment->isChecked());
    cfg.setValue("waypoint/sortIcon", toolSortIcon->isChecked());
    cfg.setValue("waypoint/sortPosition", toolSortPosition->isChecked());
    cfg.setValue("waypoint/position", linePosition->text());
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




void CWptToolWidget::slotDBChanged()
{
    listWpts->clear();

    CWptDB::keys_t key;
    QList<CWptDB::keys_t> keys = CWptDB::self().keys();

    if(toolSortAlpha->isChecked())
    {
        qSort(keys.begin(), keys.end(), CWptDB::keyLessThanAlpha);
        sortmode = eSortByName;
    }
    else if(toolSortComment->isChecked())
    {
        qSort(keys.begin(), keys.end(), CWptDB::keyLessThanComment);
        sortmode = eSortByComment;
    }
    else if(toolSortIcon->isChecked())
    {
        qSort(keys.begin(), keys.end(), CWptDB::keyLessThanIcon);
        sortmode = eSortByIcon;
    }
    else if(toolSortPosition->isChecked())
    {
        XY p1, p2;
        float lon, lat;
        GPS_Math_Str_To_Deg(linePosition->text(), lon, lat, true);
        p1.u = lon * DEG_TO_RAD;
        p1.v = lat * DEG_TO_RAD;

        QList<CWptDB::keys_t>::iterator k = keys.begin();
        while(k != keys.end())
        {
            double a1 = 0, a2 = 0;

            p2.u = k->lon * DEG_TO_RAD;
            p2.v = k->lat * DEG_TO_RAD;

            k->d = distance(p1, p2, a1, a2);
            ++k;
        }
        qSort(keys.begin(), keys.end(), CWptDB::keyLessThanDistance);
        sortmode = eSortByDistance;
        sortpos  = linePosition->text();
    }
    else
    {
        qSort(keys.begin(), keys.end(), CWptDB::keyLessThanAlpha);
        sortmode = eSortByName;
    }

    foreach(key, keys)
    {
        QString name;
        CWpt * wpt = CWptDB::self().getWptByKey(key.key);

        QListWidgetItem * item = new QListWidgetItem(listWpts);
        item->setIcon(getWptIconByName(wpt->icon));
        item->setData(Qt::UserRole, wpt->key());

        if(toolSortComment->isChecked())
        {
            name = key.comment;

            if(name.isEmpty())
            {
                name = key.name;
            }
            else
            {
                item->setToolTip(key.name);
            }
        }
        else
        {
            name = key.name;
            item->setToolTip(key.comment);
        }

        if(wpt->sticky)
        {
            name += tr(" (sticky)");
        }

        if(toolSortPosition->isChecked())
        {
            QString val, unit;
            IUnit::self().meter2distance(key.d, val, unit);
            item->setText(QString("%1 (%2 %3)").arg(name, val, unit));
        }
        else
        {
            item->setText(name);
        }
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

void CWptToolWidget::slotPosTextChanged(const QString& text)
{
    float lon = 0, lat = 0;
    toolSortPosition->setEnabled(GPS_Math_Str_To_Deg(text, lon, lat, true));
}
