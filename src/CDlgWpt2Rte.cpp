/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgWpt2Rte.cpp

  Module:      

  Description:

  Created:     12/15/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgWpt2Rte.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "CRoute.h"
#include "CRouteDB.h"
#include "CMegaMenu.h"
#include <QtGui>

CDlgWpt2Rte::CDlgWpt2Rte()
{
    setupUi(this);

    toolAdd->setIcon(QPixmap(":/icons/iconRight16x16.png"));
    connect(toolAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));
    toolDel->setIcon(QPixmap(":/icons/iconLeft16x16.png"));
    connect(toolDel, SIGNAL(clicked()), this, SLOT(slotDel()));

    toolUp->setIcon(QPixmap(":/icons/iconUpload16x16.png"));
    connect(toolUp, SIGNAL(clicked()), this, SLOT(slotUp()));
    toolDown->setIcon(QPixmap(":/icons/iconDownload16x16.png"));
    connect(toolDown, SIGNAL(clicked()), this, SLOT(slotDown()));

    connect(listSelWaypoints, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));

    QMap<QString, CWpt*>::iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end())
    {

        QListWidgetItem * item = new QListWidgetItem(listWaypoints);
        item->setText((*wpt)->getName());
        item->setData(Qt::UserRole, (*wpt)->getKey());

        wpt++;
    }

}

CDlgWpt2Rte::~CDlgWpt2Rte()
{

}


void CDlgWpt2Rte::accept()
{
    QList<QListWidgetItem*> items = listSelWaypoints->findItems("*",Qt::MatchWildcard);
    if(items.count() < 2 || lineRouteName->text().isEmpty()) return;

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

    route->setName(lineRouteName->text());

    CRouteDB::self().addRoute(route, false);
    CMegaMenu::self().switchByKeyWord("Routes");

    QDialog::accept();
}

void CDlgWpt2Rte::slotAdd()
{
    QListWidgetItem * item;
    QList<QListWidgetItem*> items = listWaypoints->selectedItems();

    foreach(item, items)
    {
        listSelWaypoints->addItem(listWaypoints->takeItem(listWaypoints->row(item)));
    }
}

void CDlgWpt2Rte::slotDel()
{
    QListWidgetItem * item;
    QList<QListWidgetItem*> items = listSelWaypoints->selectedItems();

    foreach(item, items)
    {
        listWaypoints->addItem(listSelWaypoints->takeItem(listSelWaypoints->row(item)));
    }
}

void CDlgWpt2Rte::slotUp()
{
    QListWidgetItem * item = listSelWaypoints->currentItem();
    if(item)
    {
        int row = listSelWaypoints->row(item);
        if(row == 0) return;
        listSelWaypoints->takeItem(row);
        row = row - 1;
        listSelWaypoints->insertItem(row,item);
        listSelWaypoints->setCurrentItem(item);
    }
}

void CDlgWpt2Rte::slotDown()
{
    QListWidgetItem * item = listSelWaypoints->currentItem();
    if(item)
    {
        int row = listSelWaypoints->row(item);
        if(row == (listSelWaypoints->count() - 1)) return;
        listSelWaypoints->takeItem(row);
        row = row + 1;
        listSelWaypoints->insertItem(row,item);
        listSelWaypoints->setCurrentItem(item);
    }
}

void CDlgWpt2Rte::slotItemSelectionChanged()
{
    if(listSelWaypoints->currentItem() == 0)
    {
        toolUp->setEnabled(false);
        toolDown->setEnabled(false);
    }
    else
    {
        toolUp->setEnabled(true);
        toolDown->setEnabled(true);
    }
}

