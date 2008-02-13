/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMapToolWidget.h

  Module:

  Description:

  Created:     02/13/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMAPTOOLWIDGET_H
#define CMAPTOOLWIDGET_H

#include <QWidget>

class QToolBox;

class CMapToolWidget : public QWidget//, private Ui::IWptToolWidget
{
    Q_OBJECT
    public:
        CMapToolWidget(QToolBox * parent);
        virtual ~CMapToolWidget();
};

#endif //CMAPTOOLWIDGET_H

