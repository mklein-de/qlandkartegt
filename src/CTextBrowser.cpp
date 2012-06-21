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
    connect(this, SIGNAL(sigHighlightArea(QString)), this, SLOT(slotHighlightArea(QString)));
}

CTextBrowser::~CTextBrowser()
{

}

void CTextBrowser::resetAreas()
{
    areas.clear();
}

void CTextBrowser::addArea(const QString& key, const QRect& rect)
{
    int offset = verticalScrollBar()->value();

    QRect r = rect;
    r.moveTop(r.top() + offset);
    areas[key] = r;
}

void CTextBrowser::slotHighlightArea(const QString& key)
{
    if(areaKey == key)
    {
        return;
    }

    if(!key.isEmpty() && areas.contains(key))
    {
        areaKey     = key;

        QRect r     = areas[key];
        int top     = verticalScrollBar()->value();
        int bottom  = verticalScrollBar()->value() + verticalScrollBar()->pageStep();

        if(r.top() < (top + r.height()))
        {
            int val = r.top() - r.height();
            if(val < 0) val = 0;
            verticalScrollBar()->setValue(val);
        }

        if(r.bottom() > (bottom - r.height()))
        {
            int val = r.bottom() + r.height() - verticalScrollBar()->pageStep();
            if(val < 0) val = 0;
            verticalScrollBar()->setValue(val);
        }
    }
    else
    {
        verticalScrollBar()->setValue(0);
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
}

void CTextBrowser::mouseMoveEvent(QMouseEvent * e)
{
    QTextBrowser::mouseMoveEvent(e);

    QPoint p = e->pos();
    p.setX(p.x() - verticalScrollBar()->value());

    foreach(const QString& key, areas.keys())
    {
        if(areas[key].contains(p))
        {
            emit sigHighlightArea(key);
            return;
        }
    }

    emit sigHighlightArea("");
}
