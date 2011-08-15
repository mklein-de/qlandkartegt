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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef CPLOT_H
#define CPLOT_H

#include <QWidget>
#include <QPointer>

#include "CPlotData.h"
#include "CTrack.h"

class CPlot : public QWidget
{
    Q_OBJECT
        public:

        enum mode_e {eNormal, eIcon};

        CPlot(CPlotData::axis_type_e type, mode_e mode, QWidget * parent);
        virtual ~CPlot();


        void setShowScale(bool show){showScale = show;}
        void setThinLine(bool thin){thinLine = thin;}
        void setYLabel(const QString& str);
        void setXLabel(const QString& str);

        void setSelTrackPoint(CTrack::pt_t * pt){selTrkPt = pt;}

        void newLine(const QPolygonF& line, const QPointF& focus, const QString& label);
        void addLine(const QPolygonF& line, const QString& label);
        void newMarks(const QPolygonF& line);
        void addTag(CPlotData::point_t& tag);
        void setLimits();

        void clear();

        double getXValByPixel(int px);
        double getYValByPixel(int px);

        void draw(QPainter& p);

    signals:
        void sigActivePoint(double dist);
        void sigFocusPoint(double dist);
        void sigClicked();

    public slots:
        void slotTrkPt(CTrack::pt_t * pt);


    protected slots:
        void slotSave();

    protected:
        void draw();
        void contextMenuEvent(QContextMenuEvent *event);
        void mousePressEvent(QMouseEvent * e);
        void mouseMoveEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void wheelEvent ( QWheelEvent * e );
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);
        void leaveEvent(QEvent * event);
        void enterEvent(QEvent * event);

        /// draw the actual plot
        void drawLabels(QPainter& p);
        void drawXScale(QPainter& p);
        void drawYScale(QPainter& p);
        void drawXTic(QPainter& p);
        void drawYTic(QPainter& p);
        void drawGridX(QPainter& p);
        void drawGridY(QPainter& p);
        void drawData(QPainter& p);
        void drawLegend(QPainter& p);
        void drawTags(QPainter& p);

        void setSizes();
        void setLRTB();
        void setSizeIconArea();
        void setSizeXLabel();
        void setSizeYLabel();
        void setSizeDrawArea();
        void zoom(CPlotAxis &axis, bool in, int cur = 0);

        void createActions();
        QAction *hZoomAct;
        QAction *vZoomAct;
        QAction *resetZoomAct;
        QAction *save;

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
        QRect rectIconArea;

        QFontMetrics fm;

        QPoint startMovePos;


        double initialYMax;
        double initialYMin;

        mode_e mode;
        bool showScale;
        bool thinLine;
        bool cursorFocus;
        bool needsRedraw;
        bool mouseMoveMode;
        bool checkClick;

        QPoint posMouse;
        QImage buffer;
        CTrack::pt_t * selTrkPt;
    public slots:
        void resetZoom();
};
#endif                           //CPLOT_H
