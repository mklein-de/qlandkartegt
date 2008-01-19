/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CDlgEditWpt.h

  Module:

  Description:

  Created:     01/19/2008

  (C) 2008


**********************************************************************************************/
#ifndef CDLGEDITWPT_H
#define CDLGEDITWPT_H

#include <QDialog>

#include "ui_IDlgEditWpt.h"

class CDlgEditWpt : public QDialog, public Ui::IDlgEditWpt
{
    Q_OBJECT
    public:
        CDlgEditWpt(QWidget * parent);
        virtual ~CDlgEditWpt();
};

#endif //CDLGEDITWPT_H

