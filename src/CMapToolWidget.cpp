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

#include "CMapToolWidget.h"
#include "CMapDB.h"

#include <QtGui>

CMapToolWidget::CMapToolWidget(QToolBox * parent)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("Maps");
    parent->addItem(this,QIcon(":/icons/iconMap16x16"),tr("Maps"));

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));
    connect(listKnownMaps,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotItemClicked(QListWidgetItem*)));
}

CMapToolWidget::~CMapToolWidget()
{

}

void CMapToolWidget::slotDBChanged()
{
    listKnownMaps->clear();
    const QMap<QString,CMapDB::map_t>& knownMaps = CMapDB::self().getKnownMaps();

    QMap<QString,CMapDB::map_t>::const_iterator map = knownMaps.begin();
    while(map != knownMaps.end()){
        QListWidgetItem * item = new QListWidgetItem(listKnownMaps);
        item->setText(map->description);
        item->setData(Qt::UserRole, map.key());
        ++map;
    }
}

void CMapToolWidget::slotItemClicked(QListWidgetItem* item)
{
    QString key = item->data(Qt::UserRole).toString();
    CMapDB::self().openMap(key);
}
