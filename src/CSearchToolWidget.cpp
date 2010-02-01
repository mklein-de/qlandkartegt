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

#include "CSearchToolWidget.h"
#include "CSearchDB.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CMegaMenu.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CWptDB.h"

#include <QtGui>

CSearchToolWidget::CSearchToolWidget(QTabWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("Search");

    connect(lineInput, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(&CSearchDB::self(), SIGNAL(sigStatus(const QString&)), labelStatus, SLOT(setText(const QString&)));
    connect(&CSearchDB::self(), SIGNAL(sigFinished()), this, SLOT(slotQueryFinished()));
    connect(&CSearchDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listResults,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotItemClicked(QListWidgetItem*)));

    parent->insertTab(0,this,QIcon(":/icons/iconSearch16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Search"));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Position"),this,SLOT(slotCopyPosition()),Qt::CTRL + Qt::Key_C);
    contextMenu->addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("Add Waypoint ..."),this,SLOT(slotAdd()));
    contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);

    connect(listResults,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

}


CSearchToolWidget::~CSearchToolWidget()
{

}


void CSearchToolWidget::slotReturnPressed()
{
    QString line = lineInput->text().trimmed();
    if(!line.isEmpty())
    {
        CSearchDB::self().search(line);
        lineInput->setEnabled(false);
    }
}


void CSearchToolWidget::slotQueryFinished()
{
    lineInput->setEnabled(true);

}


void CSearchToolWidget::slotDBChanged()
{
    listResults->clear();

    QMap<QString,CSearch*>::const_iterator result = CSearchDB::self().begin();
    while(result != CSearchDB::self().end())
    {
        QListWidgetItem * item = new QListWidgetItem(listResults);
        item->setText((*result)->query);

        ++result;
    }

}


void CSearchToolWidget::slotContextMenu(const QPoint& pos)
{
    if(listResults->currentItem())
    {
        QPoint p = listResults->mapToGlobal(pos);
        contextMenu->exec(p);
    }
}


void CSearchToolWidget::slotItemClicked(QListWidgetItem* item)
{
    CSearch * result = CSearchDB::self().getResultByKey(item->text());
    if(result)
    {
        theMainWindow->getCanvas()->move(result->lon, result->lat);
    }
}


void CSearchToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listResults->selectedItems();
    foreach(item,items)
    {
        keys << item->text();
        delete item;
    }
    CSearchDB::self().delResults(keys);
}


void CSearchToolWidget::slotCopyPosition()
{
    QListWidgetItem * item = listResults->currentItem();
    if(item == 0) return;
    CSearch * result = CSearchDB::self().getResultByKey(item->text());
    if(result == 0) return;

    QString position;
    GPS_Math_Deg_To_Str(result->lon, result->lat, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}


void CSearchToolWidget::slotAdd()
{
    QListWidgetItem * item = listResults->currentItem();
    if(item == 0) return;
    CSearch * result = CSearchDB::self().getResultByKey(item->text());
    if(result == 0) return;

    float ele = CMapDB::self().getDEM().getElevation(result->lon * DEG_TO_RAD, result->lat * DEG_TO_RAD);
    CWptDB::self().newWpt(result->lon * DEG_TO_RAD, result->lat * DEG_TO_RAD, ele);
}


void CSearchToolWidget::keyPressEvent(QKeyEvent * e)
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
