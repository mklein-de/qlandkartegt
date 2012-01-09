/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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

    images << img_t(tr("leave right")       , "01.jpg", ":/pics/roadbook/01.png");
    images << img_t(tr("leave left")        , "02.jpg", ":/pics/roadbook/02.png");
    images << img_t(tr("straight on")       , "03.jpg", ":/pics/roadbook/03.png");
    images << img_t(tr("straight on")       , "04.jpg", ":/pics/roadbook/04.png");
    images << img_t(tr("turn right")        , "05.jpg", ":/pics/roadbook/05.png");
    images << img_t(tr("turn left")         , "06.jpg", ":/pics/roadbook/06.png");
    images << img_t(tr("straight on")       , "07.jpg", ":/pics/roadbook/07.png");
    images << img_t(tr("straight on")       , "08.jpg", ":/pics/roadbook/08.png");
    images << img_t(tr("hard right turn")   , "09.jpg", ":/pics/roadbook/09.png");
    images << img_t(tr("hard left turn")    , "10.jpg", ":/pics/roadbook/10.png");
    images << img_t(tr("straight on")       , "11.jpg", ":/pics/roadbook/11.png");
    images << img_t(tr("straight on")       , "12.jpg", ":/pics/roadbook/12.png");
    images << img_t(tr("go left")           , "13.jpg", ":/pics/roadbook/13.png");
    images << img_t(tr("go right")          , "14.jpg", ":/pics/roadbook/14.png");
    images << img_t(tr("take right")        , "15.jpg", ":/pics/roadbook/15.png");
    images << img_t(tr("take left")         , "16.jpg", ":/pics/roadbook/16.png");
    images << img_t(tr("hard right turn")   , "17.jpg", ":/pics/roadbook/17.png");
    images << img_t(tr("hard left turn")    , "18.jpg", ":/pics/roadbook/18.png");
    images << img_t(tr("go left")           , "19.jpg", ":/pics/roadbook/19.png");
    images << img_t(tr("go right")          , "20.jpg", ":/pics/roadbook/20.png");
    images << img_t(tr("turn right @x-ing") , "21.jpg", ":/pics/roadbook/21.png");
    images << img_t(tr("turn left @x-ing")  , "22.jpg", ":/pics/roadbook/22.png");
    images << img_t(tr("straight on")       , "23.jpg", ":/pics/roadbook/23.png");
    images << img_t(tr("u-turn right")      , "25.jpg", ":/pics/roadbook/25.png");
    images << img_t(tr("u-turn left")       , "26.jpg", ":/pics/roadbook/26.png");
    images << img_t(tr("river")             , "27.jpg", ":/pics/roadbook/27.png");
    images << img_t(tr("attention")         , "28.jpg", ":/pics/roadbook/28.png");

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
        emit sigSelectImage(images[idx]);
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
