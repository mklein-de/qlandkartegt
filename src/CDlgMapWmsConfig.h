/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgMapWmsConfig.h

  Module:      

  Description:

  Created:     01/12/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGMAPWMSCONFIG_H
#define CDLGMAPWMSCONFIG_H

#include <QDialog>

#include "ui_IDlgMapWmsConfig.h"

class CMapWms;

class CDlgMapWmsConfig : public QDialog, private Ui::IDlgMapWmsConfig
{
    Q_OBJECT;
    public:
        CDlgMapWmsConfig(CMapWms& map);
        virtual ~CDlgMapWmsConfig();

    private:
        CMapWms& map;

        enum col_e{eColProperty, eColValue};
};

#endif //CDLGMAPWMSCONFIG_H

