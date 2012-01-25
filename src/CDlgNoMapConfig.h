/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgNoMapConfig.h

  Module:      

  Description:

  Created:     01/25/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGNOMAPCONFIG_H
#define CDLGNOMAPCONFIG_H

#include <QDialog>
#include "ui_IDlgNoMapConfig.h"

class CMapNoMap;

class CDlgNoMapConfig : public QDialog, private Ui::IDlgNoMapConfig
{
    Q_OBJECT
    public:
        CDlgNoMapConfig(CMapNoMap& map);
        virtual ~CDlgNoMapConfig();

    public slots:
        void accept();

    private slots:
        void slotRestoreDefault();
        void slotProjWizard();

    private:
        CMapNoMap& map;
};

#endif //CDLGNOMAPCONFIG_H

