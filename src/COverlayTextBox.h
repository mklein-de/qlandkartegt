/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        COverlayTextBox.h

  Module:

  Description:

  Created:     06/09/2008

  (C) 2008


**********************************************************************************************/
#ifndef COVERLAYTEXTBOX_H
#define COVERLAYTEXTBOX_H

#include "IOverlay.h"
#include <QRect>
#include <QPolygon>
class QPointF;

class COverlayTextBox : public IOverlay
{
    Q_OBJECT;
    public:
        COverlayTextBox(const QPointF& anchor, const QRect& rect, QObject * parent);
        virtual ~COverlayTextBox();

        void draw(QPainter& p);

        static QPolygon polygon(int x, int y, const QRect& r);

    private:
        double u;
        double v;

        QRect rect;
};

#endif //COVERLAYTEXTBOX_H

