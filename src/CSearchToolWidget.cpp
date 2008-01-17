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
#include <QtGui>

CSearchToolWidget::CSearchToolWidget(QToolBox * parent)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("Search");
    parent->addItem(this,QIcon(":/icons/iconSearch16x16"),tr("Search"));
}

CSearchToolWidget::~CSearchToolWidget()
{

}

