/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CMouseColorPicker.h

  Module:

  Description:

  Created:     12/01/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CMOUSECOLORPICKER_H
#define CMOUSECOLORPICKER_H

#include "IMouse.h"
#include <QColor>

class CMouseColorPicker : public IMouse
{
    Q_OBJECT;
    public:
        CMouseColorPicker(CCanvas * canvas);
        virtual ~CMouseColorPicker();

        QColor getSelectedColor(){return selected;}

        void draw(QPainter& p);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

    private:
        QColor color;
        QColor selected;
};

#endif //CMOUSECOLORPICKER_H

