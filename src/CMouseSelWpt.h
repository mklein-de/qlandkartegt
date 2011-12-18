/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CMouseSelWpt.h

  Module:

  Description:

  Created:     12/18/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CMOUSESELWPT_H
#define CMOUSESELWPT_H

#include "IMouse.h"

class CMouseSelWpt : public IMouse
{
    Q_OBJECT;
    public:
        CMouseSelWpt(CCanvas * canvas);
        virtual ~CMouseSelWpt();

        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void draw(QPainter& p);

    private:
        bool mousePressed;
        QPointF center;
        QPointF point1;

};

#endif //CMOUSESELWPT_H

