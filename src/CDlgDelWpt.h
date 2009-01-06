/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgDelWpt.h

  Module:

  Description:

  Created:     01/06/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGDELWPT_H
#define CDLGDELWPT_H

#include <QDialog>
#include "ui_IDlgDelWpt.h"

class CDlgDelWpt : public QDialog, private Ui::IDlgDelWpt
{
    Q_OBJECT;
    public:
        CDlgDelWpt(QWidget * parent);
        virtual ~CDlgDelWpt();

    public slots:
        void accept();
};

#endif //CDLGDELWPT_H

