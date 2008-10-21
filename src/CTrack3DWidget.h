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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#ifndef CTRACK3DWIDGET_H
#define CTRACK3DWIDGET_H

#include <QGLWidget>
#include <QPointer>
#include <QWidget>

#include "CTrack.h"
#include "CMapQMAP.h"
#include "IMap.h"

class CTrack3DWidget: public QGLWidget
{
    Q_OBJECT;
    public:
        CTrack3DWidget(QWidget * parent);
        virtual ~CTrack3DWidget();
        void convertPt23D(double& u, double& v, double &ele);
        void convert3D2Pt(double& u, double& v, double &ele);

    protected:
        QPointer<CTrack> track;
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent ( QMouseEvent * event );
        void contextMenuEvent(QContextMenuEvent *event);
        void keyPressEvent ( QKeyEvent * event );
        void createActions();
        void cameraTranslated(GLdouble x, GLdouble y, GLdouble z);
        void cameraRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z, bool correct = true);

        QAction *eleZoomInAct;
        QAction *eleZoomOutAct;
        QAction *eleZoomResetAct;
        QAction *map3DAct;
        QAction *showTrackAct;

    private:
	CMapQMAP *map;
	void loadMap();
        GLuint makeObject();
        void setMapTexture();
        void quad(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2);
        void normalizeAngle(double *angle);
        void drawFlatMap();
        /// using DEM data file to display terrain in 3D
        void draw3DMap();
        void drawTrack();

        GLuint object;
        double mouseRotSens;
        double keyboardRotSens;
        double translateSens;
        GLuint mapTexture;
        double eleZoomFactor;

        double maxElevation, minElevation;

        QPoint lastPos;
        QColor wallCollor;
        QColor highBorderColor;

        /// current selected trackpoint
        CTrack::pt_t * selTrkPt;
        double cameraRotX;

    private slots:
        void slotChanged();

    public slots:
        void eleZoomOut();
        void eleZoomIn();
        void eleZoomReset();

    signals:
        void xRotationChanged(double angle);
        void zRotationChanged(double angle);
};

#endif //CTRACK3DWIDGET_H

