/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgSetupGrid.h

  Module:

  Description:

  Created:     12/06/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGSETUPGRID_H
#define CDLGSETUPGRID_H

#include <QDialog>

#include "ui_IDlgSetupGrid.h"

class CDlgSetupGrid : public QDialog, private Ui::IDlgSetupGrid
{
    Q_OBJECT;
    public:
        CDlgSetupGrid(QWidget * parent);
        virtual ~CDlgSetupGrid();
    public slots:
        void accept();

    private slots:
        void slotProjWizard();
        void slotSelectGridColor();
};

#endif //CDLGSETUPGRID_H

