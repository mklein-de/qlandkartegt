/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgEditDistance.h

  Module:

  Description:

  Created:     08/30/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGEDITDISTANCE_H
#define CDLGEDITDISTANCE_H

#include <QDialog>
#include "ui_IDlgEditDistance.h"

class COverlayDistance;

class CDlgEditDistance : public QDialog, private Ui::IDlgEditDistance
{
    Q_OBJECT;
    public:
        CDlgEditDistance(COverlayDistance &ovl, QWidget * parent);
        virtual ~CDlgEditDistance();

    public slots:
        void accept();

    private:
        COverlayDistance& ovl;
};

#endif //CDLGEDITDISTANCE_H

