/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMapDB.cpp

  Module:

  Description:

  Created:     02/13/2008

  (C) 2008


**********************************************************************************************/

#include "CMapDB.h"
#include "CMapToolWidget.h"

CMapDB * CMapDB::m_self = 0;

CMapDB::CMapDB(QToolBox * tb, QObject * parent)
    : IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CMapToolWidget(tb);

}

CMapDB::~CMapDB()
{

}

void CMapDB::loadGPX(CGpx& gpx)
{
}

void CMapDB::saveGPX(CGpx& gpx)
{
}

void CMapDB::loadQLB(CQlb& qlb)
{
}

void CMapDB::saveQLB(CQlb& qlb)
{
}

void CMapDB::upload()
{
}

void CMapDB::download()
{
}


