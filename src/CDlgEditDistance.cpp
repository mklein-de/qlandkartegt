/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgEditDistance.cpp

  Module:

  Description:

  Created:     08/30/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgEditDistance.h"
#include "COverlayDistance.h"

CDlgEditDistance::CDlgEditDistance(COverlayDistance &ovl, QWidget * parent)
: QDialog(parent)
, ovl(ovl)
{
    setupUi(this);
    lineName->setText(ovl.name);
    textComment->setText(ovl.comment);
}

CDlgEditDistance::~CDlgEditDistance()
{

}

void CDlgEditDistance::accept()
{
    ovl.name = lineName->text();
    ovl.comment = textComment->toPlainText();
    QDialog::accept();
}
