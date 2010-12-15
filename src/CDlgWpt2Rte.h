/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgWpt2Rte.h

  Module:      

  Description:

  Created:     12/15/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGWPT2RTE_H
#define CDLGWPT2RTE_H


#include <QDialog>
#include "ui_IDlgWpt2Rte.h"


class CDlgWpt2Rte  : public QDialog, private Ui::IDlgWpt2Rte
{
    Q_OBJECT;
    public:
        CDlgWpt2Rte();
        virtual ~CDlgWpt2Rte();

    public slots:
        void accept();

    private slots:
        void slotAdd();
        void slotDel();
        void slotUp();
        void slotDown();
        void slotItemSelectionChanged();

};

#endif //CDLGWPT2RTE_H

