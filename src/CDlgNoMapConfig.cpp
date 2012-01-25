/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgNoMapConfig.cpp

  Module:      

  Description:

  Created:     01/25/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgNoMapConfig.h"
#include "CMapNoMap.h"
#include "CDlgProjWizzard.h"

#include <QtGui>

CDlgNoMapConfig::CDlgNoMapConfig(CMapNoMap &map)
: map(map)
{
    setupUi(this);

    connect(toolProjWizzard, SIGNAL(clicked()), this, SLOT(slotProjWizard()));
    connect(toolRestoreDefault, SIGNAL(clicked()), this, SLOT(slotRestoreDefault()));

    lineProjection->setText(map.getProjection());
    lineProjection->setCursorPosition(0);

    lineXScale->setText(QString::number(  map.xscale, 'f'));
    lineYScale->setText(QString::number(- map.yscale, 'f'));
}

CDlgNoMapConfig::~CDlgNoMapConfig()
{

}

void CDlgNoMapConfig::accept()
{
    if (CDlgProjWizzard::validProjStr(lineProjection->text()))
    {
        map.setup(lineProjection->text(), lineXScale->text().toDouble(), -lineYScale->text().toDouble());


        QDialog::accept();
    }
}


void CDlgNoMapConfig::slotRestoreDefault()
{
    lineProjection->setText("+proj=merc +a=6378137.0000 +b=6356752.3142 +towgs84=0,0,0,0,0,0,0,0 +units=m  +no_defs");
    lineProjection->setCursorPosition(0);
}

void CDlgNoMapConfig::slotProjWizard()
{
    CDlgProjWizzard dlg(*lineProjection, this);
    dlg.exec();
}
