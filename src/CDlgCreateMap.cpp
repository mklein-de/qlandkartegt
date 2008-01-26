/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CDlgCreateMap.cpp

  Module:

  Description:

  Created:     01/26/2008

  (C) 2008


**********************************************************************************************/

#include "CDlgCreateMap.h"

CDlgCreateMap::CDlgCreateMap(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    comboSource->insertItem(eNone,tr(""));
    comboSource->insertItem(eOSM,QIcon(":/icons/iconOSM16x16.png"),tr("Open Street Map"));

    connect(comboSource, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));
}

CDlgCreateMap::~CDlgCreateMap()
{

}

