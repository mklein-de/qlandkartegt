/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTextBrowser.cpp

  Module:      

  Description:

  Created:     06/20/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CTextBrowser.h"

#include <QtGui>

CTextBrowser::CTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{

}

CTextBrowser::~CTextBrowser()
{

}

void CTextBrowser::paintEvent(QPaintEvent * e)
{
    QTextBrowser::paintEvent(e);
    QPainter p(viewport());
}
