/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseMoveWpt.h

  Module:

  Description:

  Created:     03/03/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMOUSEMOVEWPT_H
#define CMOUSEMOVEWPT_H

#include "IMouse.h"
#include <QPoint>

class CMouseMoveWpt : public IMouse
{
    Q_OBJECT
    public:
        CMouseMoveWpt(CCanvas * canvas);
        virtual ~CMouseMoveWpt();

        void draw(QPainter& p);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

    private:
        bool moveWpt;
        QPoint newPos;
};

#endif //CMOUSEMOVEWPT_H

