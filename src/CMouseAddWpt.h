/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseAddWpt.h

  Module:

  Description:

  Created:     01/19/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMOUSEADDWPT_H
#define CMOUSEADDWPT_H

#include "IMouse.h"

class CMouseAddWpt : public IMouse
{
    Q_OBJECT
    public:
        CMouseAddWpt(CCanvas * canvas);
        virtual ~CMouseAddWpt();

        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

};

#endif //CMOUSEADDWPT_H

