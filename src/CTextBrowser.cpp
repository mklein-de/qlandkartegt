/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTextBrowser.cpp

  Module:

  Description:

  Created:     06/20/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CTextBrowser.h"
#include "CCanvas.h"

#include <QtGui>

CTextBrowser::CTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{

}

CTextBrowser::~CTextBrowser()
{

}

void CTextBrowser::addArea(const QString& key, const QRect& rect)
{
    int offset = verticalScrollBar()->value();

    QRect r = rect;
    r.moveTop(r.top() + offset);
    areas[key] = r;
}

void CTextBrowser::highlightArea(const QString& key)
{
    if(areaKey == key)
    {
        return;
    }

    if(!key.isEmpty() && areas.contains(key))
    {
        areaKey = key;

//        QRect r = areas[key];

//        if(r.top() < verticalScrollBar()->value())
//        {
//            int pos = r.top() - r.height() * 3;
//            if(pos < 0) pos = 0;
//            verticalScrollBar()->setValue(pos);
//        }

//        if(r.bottom() > (verticalScrollBar()->value() + verticalScrollBar()->pageStep()))
//        {
//            int pos = r.bottom() - verticalScrollBar()->pageStep() + r.height();
//            if(pos < 0) pos = 0;
//            verticalScrollBar()->setValue(pos);
//        }

    }
    else
    {
        areaKey.clear();
    }
    viewport()->update();
}

void CTextBrowser::paintEvent(QPaintEvent * e)
{
    QTextBrowser::paintEvent(e);
    QPainter p(viewport());

    int offset = verticalScrollBar()->value();

    USE_ANTI_ALIASING(p, true);
    p.setPen(QPen(CCanvas::penBorderBlue));

    QRect r = areas[areaKey];
    r.moveTop(r.top() - offset);

    PAINT_ROUNDED_RECT(p, r);

    qDebug() << verticalScrollBar()->value();
}
