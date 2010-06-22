/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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


#include "CDlgExport.h"
#include "IDevice.h"
#include "CWptDB.h"
#include "CWpt.h"
#include "CWptToolWidget.h"
#include "WptIcons.h"
#include "GeoMath.h"


#include <QtGui>
#include <projects.h>

CDlgExport::CDlgExport(QWidget * parent, QStringList * wpt, QStringList * trk, QStringList * rte)
: QDialog(parent)
, keysWpt(wpt)
, keysTrk(trk)
, keysRte(rte)
{
    setupUi(this);
}

CDlgExport::~CDlgExport()
{

}

int CDlgExport::exec()
{

    itemWpt = new QTreeWidgetItem(treeWidget, QStringList(tr("Waypoints")));
    itemWpt->setIcon(0,QIcon(":/icons/iconWaypoint16x16"));
    itemWpt->setExpanded(true);

    itemTrk = new QTreeWidgetItem(treeWidget, QStringList(tr("Tracks")));
    itemTrk->setIcon(0,QIcon(":/icons/iconTrack16x16"));
    itemTrk->setExpanded(true);

    itemRte = new QTreeWidgetItem(treeWidget, QStringList(tr("Routes")));
    itemRte->setIcon(0,QIcon(":/icons/iconRoute16x16"));
    itemRte->setExpanded(true);

    if(IDevice::m_UploadAllWpt && keysWpt)
    {
        QString pos;
        CWptToolWidget::sortmode_e sortmode = CWptToolWidget::getSortMode(pos);

        CWptDB::keys_t key;
        QList<CWptDB::keys_t> keys = CWptDB::self().keys();

        switch(sortmode)
        {
            case CWptToolWidget::eSortByName:
                qSort(keys.begin(), keys.end(), CWptDB::keyLessThanAlpha);
                break;
            case CWptToolWidget::eSortByComment:
                qSort(keys.begin(), keys.end(), CWptDB::keyLessThanComment);
                break;
            case CWptToolWidget::eSortByIcon:
                qSort(keys.begin(), keys.end(), CWptDB::keyLessThanIcon);
                break;
            case CWptToolWidget::eSortByDistance:
                {
                    XY p1, p2;
                    float lon, lat;
                    GPS_Math_Str_To_Deg(pos, lon, lat, true);
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
                }
                break;
        }

        foreach(key, keys)
        {
            QStringList str;
            str << key.name << key.comment.left(32);
            QTreeWidgetItem * item = new QTreeWidgetItem(itemWpt, str);
            item->setCheckState(0, Qt::Checked);
            item->setIcon(0,getWptIconByName(key.icon));
            item->setData(0, Qt::UserRole, key.key);
        }

    }
    else
    {
        itemWpt->setDisabled(true);
    }

    if(IDevice::m_UploadAllTrk && keysTrk)
    {
    }
    else
    {
        itemTrk->setDisabled(true);
    }

    if(IDevice::m_UploadAllRte && keysRte)
    {
    }
    else
    {
        itemRte->setDisabled(true);
    }


    QDialog::exec();
}

void CDlgExport::accept()
{

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items;

    if(keysWpt){
        items = itemWpt->takeChildren();
        foreach(item, items)
        {
            if(item->checkState(0) == Qt::Checked)
            {
                *keysWpt << item->data(0, Qt::UserRole).toString();
            }

            delete item;
        }
    }

    if(keysTrk)
    {
        items = itemTrk->takeChildren();
        foreach(item, items)
        {
            if(item->checkState(0) == Qt::Checked)
            {
                *keysTrk << item->data(0, Qt::UserRole).toString();
            }

            delete item;
        }
    }

    if(keysRte)
    {
        items = itemRte->takeChildren();
        foreach(item, items)
        {
            if(item->checkState(0) == Qt::Checked)
            {
                *keysRte << item->data(0, Qt::UserRole).toString();
            }

            delete item;
        }
    }

    QDialog::accept();
}
