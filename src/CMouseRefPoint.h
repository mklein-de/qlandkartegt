/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseRefPoint.h

  Module:

  Description:

  Created:     03/07/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMOUSEREFPOINT_H
#define CMOUSEREFPOINT_H

#include "IMouse.h"
#include "CCreateMapGeoTiff.h"

class CMouseRefPoint :  public IMouse
{
    Q_OBJECT;
    public:
        CMouseRefPoint(CCanvas * canvas);
        virtual ~CMouseRefPoint();

        void draw(QPainter& p);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

    private:
        /// true if left mouse button is pressed
        bool moveMap;
        /// the initial starting point of the transformation
        QPoint oldPoint;

        CCreateMapGeoTiff::refpt_t * selRefPt;
};

#endif //CMOUSEREFPOINT_H

