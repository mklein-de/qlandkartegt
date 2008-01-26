/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CDlgCreateMap.h

  Module:

  Description:

  Created:     01/26/2008

  (C) 2008


**********************************************************************************************/
#ifndef CDLGCREATEMAP_H
#define CDLGCREATEMAP_H

#include <QDialog>

#include "ui_IDlgCreateMap.h"

class CDlgCreateMap : public QDialog, private Ui::IDlgCreateMap
{
    Q_OBJECT
    public:
        CDlgCreateMap(QWidget * parent);
        virtual ~CDlgCreateMap();


    private:
        enum widget_e {eNone, eOSM};

};

#endif //CDLGCREATEMAP_H

