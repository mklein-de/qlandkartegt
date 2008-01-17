/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CSearchDB.cpp

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/

#include "CSearchDB.h"
#include "CSearchToolWidget.h"

CSearchDB * CSearchDB::m_self;

CSearchDB::CSearchDB(QToolBox * tb, QObject * parent)
    : IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CSearchToolWidget(tb);
    toolview->setObjectName("Search");
}

CSearchDB::~CSearchDB()
{

}

