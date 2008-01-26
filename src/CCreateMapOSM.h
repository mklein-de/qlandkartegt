/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CCreateMapOSM.h

  Module:

  Description:

  Created:     01/26/2008

  (C) 2008


**********************************************************************************************/
#ifndef CCREATEMAPOSM_H
#define CCREATEMAPOSM_H

#include <QWidget>

#include "ui_ICreateMapOSM.h"

class CCreateMapOSM : public QWidget, private Ui::ICreateMapOSM
{
    Q_OBJECT
    public:
        CCreateMapOSM(QWidget * parent);
        virtual ~CCreateMapOSM();
};

#endif //CCREATEMAPOSM_H

