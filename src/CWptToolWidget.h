/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CWptToolWidget.h

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/
#ifndef CWPTTOOLWIDGET_H
#define CWPTTOOLWIDGET_H

#include <QWidget>

class QToolBox;

class CWptToolWidget : public QWidget
{
    Q_OBJECT
    public:
        CWptToolWidget(QToolBox * parent);
        virtual ~CWptToolWidget();
};

#endif //CWPTTOOLWIDGET_H

