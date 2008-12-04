/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CTRACK3DWIDGET_H
#define CTRACK3DWIDGET_H

#include <QGLWidget>
#include <QPointer>
#include <QWidget>
#include <QSet>

#include "CTrack.h"
#include "CMapQMAP.h"
#include "IMap.h"

class CMap3DWidget: public QGLWidget
{
    Q_OBJECT;
    public:
        CMap3DWidget(QWidget *parent);
        virtual ~CMap3DWidget();
        void convertPt23D(double& u, double& v, double &ele);
        void convert3D2Pt(double& u, double& v, double &ele);
        /// conver coord of point a on the window to the flat z = 0
        void convertDsp2Z0(QPoint &a);

    protected:
        QPointer<CTrack> track;
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent ( QMouseEvent * event );
        void wheelEvent ( QWheelEvent * e );
        void contextMenuEvent(QContextMenuEvent *event);
        void keyPressEvent ( QKeyEvent * event );
        void keyReleaseEvent ( QKeyEvent * event );
        void focusOutEvent ( QFocusEvent * event );
        void createActions();
        void updateElevationLimits();
        void getEleRegion(float *buffer, int xcount, int ycount);
        float getRegionValue(float *buffer, int x, int y);

        QAction *eleZoomInAct;
        QAction *eleZoomOutAct;
        QAction *eleZoomResetAct;
        QAction *map3DAct;
        QAction *showTrackAct;
        QAction *mapEleAct;
        QSet<int> pressedKeys;

    private:
        unsigned int skyBox[6];
        double step;
        QPointer<IMap> map;
        QSize mapSize;
        void loadMap();
        /// expand map relative to the center
        void expandMap(bool zoomIn);
        GLuint makeObject();
        void setMapTexture();
        void quad(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2);
        void normalizeAngle(double *angle);
        void drawFlatMap();
        /// using DEM data file to display terrain in 3D
        void draw3DMap();
        void drawTrack();
        void drawSkybox(double x, double y, double z, double xs, double ys, double zs);

        GLuint object;
        double xRot;
        double zRot;
        double xRotSens;
        double zRotSens;
        GLuint mapTexture;
        double xShift, yShift, zoomFactor, eleZoomFactor;

        double maxElevation, minElevation;

        QPoint lastPos;
        QColor wallCollor;
        QColor highBorderColor;

        /// current selected trackpoint
        CTrack::pt_t * selTrkPt;

    private slots:
        void slotChanged();
        void mapResize(const QSize& size);

    public slots:
        void setXRotation(double angle);
        void setZRotation(double angle);
        void eleZoomOut();
        void eleZoomIn();
        void eleZoomReset();
        void changeMode();

        signals:
        void xRotationChanged(double angle);
        void zRotationChanged(double angle);
};
#endif                           //CTRACK3DWIDGET_H
