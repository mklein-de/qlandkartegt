/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CImageSelect.cpp

  Module:

  Description:

  Created:     11/26/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CImageSelect.h"
#include "CWpt.h"

#include <QtGui>

#define WIDTH   100.0
#define HEIGHT  100.0


CImageSelect::CImageSelect(QWidget * parent)
    : QWidget(parent)
    , wpt(0)
{
    scrollBar = 0;
    setupUi(this);

    images << img_t(tr("leave right")       , ":/pics/roadbook/01.png");
    images << img_t(tr("leave left")        , ":/pics/roadbook/02.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/03.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/04.png");
    images << img_t(tr("turn right")        , ":/pics/roadbook/05.png");
    images << img_t(tr("turn left")         , ":/pics/roadbook/06.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/07.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/08.png");
    images << img_t(tr("hard right turn")   , ":/pics/roadbook/09.png");
    images << img_t(tr("hard left turn")    , ":/pics/roadbook/10.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/11.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/12.png");
    images << img_t(tr("go left")           , ":/pics/roadbook/13.png");
    images << img_t(tr("go right")          , ":/pics/roadbook/14.png");
    images << img_t(tr("take right")        , ":/pics/roadbook/15.png");
    images << img_t(tr("take left")         , ":/pics/roadbook/16.png");
    images << img_t(tr("hard right turn")   , ":/pics/roadbook/17.png");
    images << img_t(tr("hard left turn")    , ":/pics/roadbook/18.png");
    images << img_t(tr("go left")           , ":/pics/roadbook/19.png");
    images << img_t(tr("go right")          , ":/pics/roadbook/20.png");
    images << img_t(tr("turn right @x-ing") , ":/pics/roadbook/21.png");
    images << img_t(tr("turn left @x-ing")  , ":/pics/roadbook/22.png");
    images << img_t(tr("straight on")       , ":/pics/roadbook/23.png");

    images << img_t(tr("u-turn right")      , ":/pics/roadbook/25.png");
    images << img_t(tr("u-turn left")       , ":/pics/roadbook/26.png");
    images << img_t(tr("river")             , ":/pics/roadbook/27.png");
    images << img_t(tr("attention")         , ":/pics/roadbook/28.png");

    setMaximumHeight(HEIGHT + scrollBar->height());

    scrollBar->setMinimum(0);
    scrollBar->setMaximum(images.size() - 1);

    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(update()));
}

CImageSelect::~CImageSelect()
{

}

void CImageSelect::resizeEvent(QResizeEvent * e)
{
    QSize s = e->size();
    scrollBar->setPageStep(s.width()/WIDTH);
    scrollBar->setMaximum(images.size() - scrollBar->pageStep());
}

void CImageSelect::mousePressEvent(QMouseEvent * e)
{
    QPoint p = e->pos();
    int idx = scrollBar->value() + p.x() / WIDTH;

    if(idx < images.size())
    {
        img_t& src =  images[idx];

        wpt->images.clear();
        CWpt::image_t tar;

        tar.pixmap  = src.img;
        tar.info    = src.title;
        wpt->images << tar;
        emit sigChangedImage(0);
    }
}

void CImageSelect::wheelEvent(QWheelEvent * e)
{
    if(!scrollBar->rect().contains(e->pos()))
    {
        int value = scrollBar->value() + ((e->delta() > 0) ? -1 : 1);
        scrollBar->setValue(value);
    }
}

void CImageSelect::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QSize s     = size();
    int start   = scrollBar->value();

    p.fillRect(rect(), Qt::white);

    int h       = size().height();
    int xoff    = 0;
    for(int i = start; i < images.size(); i++)
    {
        img_t& img = images[i];

        int wImg = img.img.width();
        int hImg = img.img.height();

        float f = WIDTH/wImg;

        if((hImg * f) > h)
        {
            p.drawPixmap(xoff, 0, img.img.scaledToHeight(h, Qt::SmoothTransformation));
        }
        else
        {
            p.drawPixmap(xoff, 0, img.img.scaledToWidth(WIDTH, Qt::SmoothTransformation));
        }

        xoff += WIDTH;

        if(xoff > width())
        {
            break;
        }
    }

}
