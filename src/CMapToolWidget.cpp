/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMapToolWidget.cpp

  Module:

  Description:

  Created:     02/13/2008

  (C) 2008


**********************************************************************************************/

#include "CMapToolWidget.h"

#include <QtGui>

CMapToolWidget::CMapToolWidget(QToolBox * parent)
    : QWidget(parent)
{
//     setupUi(this);
    setObjectName("Maps");
    parent->addItem(this,QIcon(":/icons/iconMap16x16"),tr("Maps"));


}

CMapToolWidget::~CMapToolWidget()
{

}

