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

#include "CSearchToolWidget.h"
#include "CSearchDB.h"
#include "CMainWindow.h"
#include "CCanvas.h"

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

    parent->addTab(this,QIcon(":/icons/iconSearch16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Search"));
}

CSearchToolWidget::~CSearchToolWidget()
{

}

void CSearchToolWidget::slotReturnPressed()
{
    QString line = lineInput->text().trimmed();
    if(!line.isEmpty()){
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

    QMap<QString,CSearchDB::result_t>::const_iterator result = CSearchDB::self().begin();
    while(result != CSearchDB::self().end()){
        QListWidgetItem * item = new QListWidgetItem(listResults);
        item->setText(result->query);

        ++result;
    }

}

void CSearchToolWidget::slotItemClicked(QListWidgetItem* item)
{
    CSearchDB::result_t * result = CSearchDB::self().getResultByKey(item->text());
    if(result){
        theMainWindow->getCanvas()->move(result->lon, result->lat);
    }
}
