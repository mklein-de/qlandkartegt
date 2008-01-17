/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        IDB.cpp

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/

#include "IDB.h"

#include <QtGui>

IDB::IDB(QToolBox * tb, QObject * parent)
    : QObject(parent)
    , toolbox(tb)
{

}

IDB::~IDB()
{

}

void IDB::gainFocus()
{
    if(toolview && toolbox->currentWidget() != toolview){
        toolbox->setCurrentWidget(toolview);
    }
}
