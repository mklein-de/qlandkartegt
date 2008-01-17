/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CSearchToolWidget.h

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/
#ifndef CSEARCHTOOLWIDGET_H
#define CSEARCHTOOLWIDGET_H

#include <QWidget>

class QToolBox;

class CSearchToolWidget : public QWidget
{
    Q_OBJECT
    public:
        CSearchToolWidget(QToolBox * parent);
        virtual ~CSearchToolWidget();
};

#endif //CSEARCHTOOLWIDGET_H

