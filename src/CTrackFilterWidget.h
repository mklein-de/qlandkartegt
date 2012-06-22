/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTrackFilterWidget.h

  Module:      

  Description:

  Created:     06/22/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CTRACKFILTERWIDGET_H
#define CTRACKFILTERWIDGET_H

#include <QWidget>

#include "ui_ITrackFilterWidget.h"

class CTrackFilterWidget : public QWidget, private Ui::ITrackFilterWidget
{
    Q_OBJECT;
    public:
        CTrackFilterWidget(QWidget * parent);
        virtual ~CTrackFilterWidget();
};

#endif //CTRACKFILTERWIDGET_H

