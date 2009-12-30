/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgSetupGarminIcons.h

  Module:

  Description:

  Created:     12/30/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGSETUPGARMINICONS_H
#define CDLGSETUPGARMINICONS_H

#include <QDialog>
#include "ui_IDlgSetupGarminIcons.h"

class CDlgSetupGarminIcons : public QDialog, private Ui::IDlgSetupGarminIcons
{
    Q_OBJECT;
    public:
        CDlgSetupGarminIcons();
        virtual ~CDlgSetupGarminIcons();

    public slots:
        void exec();
        void accept();

    private slots:
        void slotChangeIconSource();
        void slotResetIconSource();
        void slotSendToDevice();

};

#endif //CDLGSETUPGARMINICONS_H

