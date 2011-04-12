/**********************************************************************************************
    Copyright (C) 2011 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgMapOSMConfig.h"
#include "CMapOSM.h"
#include "CMapOSMType.h"
#include <QtGui>

CDlgMapOSMConfig::CDlgMapOSMConfig(CMapOSM * map)
: map(map)
{
    setupUi(this);

    connect(this, SIGNAL(accepted()), SLOT(deleteLater()));
    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));

    QList<CMapOSMType> list;
    list = map->getServerList();

    QList<QString> header;
    header << tr("Name") << tr("Path") << tr("Key");

    mapsTreeWidget->setHeaderLabels(header);

    mapsTreeWidget->setColumnCount(3);
    mapsTreeWidget->setColumnHidden(2,true);
    mapsTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

    topBuiltin = new QTreeWidgetItem(mapsTreeWidget);
    topBuiltin->setText(0, tr("Built-in maps"));

    topCustom = new QTreeWidgetItem(mapsTreeWidget);
    topCustom->setText(0, tr("Custom maps"));

    for (int i = 0; i < list.size(); ++i)
    {
        CMapOSMType currentMap=list.at(i);
        QTreeWidgetItem *cm = new QTreeWidgetItem(currentMap.isBuiltin() ? topBuiltin : topCustom);
        cm->setFlags(cm->flags()|Qt::ItemIsSelectable);
        cm->setText(0, currentMap.title);
        cm->setText(1, currentMap.path);
        if (currentMap.isBuiltin())
        {
            cm->setText(2, currentMap.key);
            cm->setFlags(cm->flags()|Qt::ItemIsUserCheckable);
            cm->setCheckState(0,currentMap.isEnabled() ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            cm->setFlags(cm->flags()|Qt::ItemIsEditable);
        }
    }

    topBuiltin->sortChildren(0,Qt::AscendingOrder);
    topBuiltin->setExpanded(true);
    topCustom->sortChildren(0,Qt::AscendingOrder);
    topCustom->setExpanded(true);

}


CDlgMapOSMConfig::~CDlgMapOSMConfig()
{
    qDebug() << "CDlgMapOSMConfig::~CDlgMapOSMConfig()";
}


void CDlgMapOSMConfig::onItemDelete()
{
    if (!(mapsTreeWidget->selectedItems().isEmpty()))
    {

        QTreeWidgetItem * item = mapsTreeWidget->selectedItems().first();
        int i = mapsTreeWidget->indexOfTopLevelItem(item);
        mapsTreeWidget->takeTopLevelItem(i);
        delete item;
    }
}


void CDlgMapOSMConfig::onItemSelectionChanged()
{
    if (mapsTreeWidget->selectedItems().isEmpty())
    {
        pbDelete->setEnabled(false);
    }
    else
    {
        if (mapsTreeWidget->selectedItems().first()->flags().testFlag(Qt::ItemIsEditable))
        {
            pbDelete->setEnabled(true);
        }
        else
        {
            pbDelete->setEnabled(false);
        }
    }
}


void CDlgMapOSMConfig::accept()
{
    QSettings cfg;

    QTreeWidgetItemIterator itBuiltin(topBuiltin,QTreeWidgetItemIterator::NoChildren);
    while (*itBuiltin)
    {
        bool mapEnabled=((*itBuiltin)->checkState(0)==Qt::Checked);
        cfg.setValue(QString("osm/builtinMaps/").append(QString((*itBuiltin)->text(2))), mapEnabled);

        ++itBuiltin;
    }

    cfg.remove("osm/customMaps");
    if (topCustom->childCount()>0)
    {
        QTreeWidgetItemIterator itCustom(topCustom,QTreeWidgetItemIterator::NoChildren);
        cfg.beginWriteArray("osm/customMaps");
        int counter=0;
        while (*itCustom)
        {
            cfg.setArrayIndex(counter++);
            cfg.setValue("mapName", (*itCustom)->text(0));
            cfg.setValue("mapString", (*itCustom)->text(1));

            ++itCustom;
        }
        cfg.endArray();
    }

    map->rebuildServerList();
    QDialog::accept();
}


void CDlgMapOSMConfig::on_pbAdd_clicked()
{
    QTreeWidgetItem *cm = new QTreeWidgetItem(topCustom);
    cm->setText(0, "New Map");
    cm->setText(1, "Path");
    cm->setFlags(cm->flags()|Qt::ItemIsSelectable|Qt::ItemIsEditable);

    topCustom->setExpanded(true);
}
