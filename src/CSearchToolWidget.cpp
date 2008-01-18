/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CSearchToolWidget.cpp

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/

#include "CSearchToolWidget.h"
#include "CSearchDB.h"
#include "CMainWindow.h"
#include "CCanvas.h"

#include <QtGui>

CSearchToolWidget::CSearchToolWidget(QToolBox * parent)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("Search");

    connect(lineInput, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(&CSearchDB::self(), SIGNAL(sigStatus(const QString&)), labelStatus, SLOT(setText(const QString&)));
    connect(&CSearchDB::self(), SIGNAL(sigFinished()), this, SLOT(slotQueryFinished()));

    connect(listResults,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotItemClicked(QListWidgetItem*)));

    parent->addItem(this,QIcon(":/icons/iconSearch16x16"),tr("Search"));
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
