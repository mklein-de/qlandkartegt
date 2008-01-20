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

#include <QtGui>

CWptToolWidget::CWptToolWidget(QToolBox * parent)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("Waypoints");
    parent->addItem(this,QIcon(":/icons/iconWaypoint16x16"),tr("Waypoints"));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(listWpts,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotItemClicked(QListWidgetItem*)));
}

CWptToolWidget::~CWptToolWidget()
{

}

void CWptToolWidget::slotDBChanged()
{
    listWpts->clear();

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()){
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
    if(wpt){
        theMainWindow->getCanvas()->move(wpt->lon, wpt->lat);
    }

}
