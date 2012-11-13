/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgMapRMPConfig.h

  Module:      

  Description:

  Created:     11/13/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGMAPRMPCONFIG_H
#define CDLGMAPRMPCONFIG_H

#include <QDialog>
#include "ui_IDlgMapRMPConfig.h"

class CMapRmp;

class CDlgMapRMPConfig : public QDialog, private Ui::IDlgMapRMPConfig
{
    public:
        CDlgMapRMPConfig(CMapRmp * map);
        virtual ~CDlgMapRMPConfig();

    private:
        static const QString text;
};

#endif //CDLGMAPRMPCONFIG_H

