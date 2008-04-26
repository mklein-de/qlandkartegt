/**********************************************************************************************
    Copyright (C) 2006, 2007 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#ifndef CPLOT_H
#define CPLOT_H

#include <QWidget>

class CPlotData;

class CPlot : public QWidget
{
    Q_OBJECT
        public:
        CPlot(QWidget * parent);
        virtual ~CPlot();

        void setLine(const QPolygonF& line);
        void setLine(const QPolygonF& line, const QPolygonF& marks);
        void setLine(const QPolygonF& line, const QPolygonF& marks, const QPointF& focus);

        void clear();

        double getXValByPixel(int px);

    protected:
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);

        /// draw the actual plot
        void draw(QPainter& p);
        void drawLabels(QPainter& p);
        void drawXScale(QPainter& p);
        void drawYScale(QPainter& p);
        void drawXTic(QPainter& p);
        void drawYTic(QPainter& p);
        void drawGridX(QPainter& p);
        void drawGridY(QPainter& p);
        void drawData(QPainter& p);

        void setSizes();
        void setLRTB();
        void setSizeXLabel();
        void setSizeYLabel();
        void setSizeDrawArea();

        CPlotData * m_pData;

        ///width of the used font
        int fontWidth;
        ///height of the used font
        int fontHeight;
        ///width of the scale at the bottom of the plot
        int scaleWidthX1;
        ///width of the scale at the left of the plot
        int scaleWidthY1;

        int deadAreaX;
        int deadAreaY;

        int left;
        int right;
        int top;
        int bottom;

        QRect rectX1Label;
        QRect rectY1Label;
        QRect rectGraphArea;

        QFontMetrics fm;

};
#endif                           //CPLOT_H
