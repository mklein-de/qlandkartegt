/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CMAP3D_H
#define CMAP3D_H


#include <QGLWidget>
#include <QPointer>
#include <QPoint>

#include "IMap.h"

class CMap3D : public QGLWidget
{
    Q_OBJECT;
    public:
        CMap3D(IMap * map, QWidget * parent);
        virtual ~CMap3D();
    void setXRotation(double arg1);


    protected slots:


    protected:
        void paintGL();
        void initializeGL();
        void resizeGL(int width, int height);

        double normalizeAngle(double angle);

        /// set the point of view
        void setPOV (void);
        void drawSkybox();
        void drawCenterStar();
        void drawBaseGrid();

        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void keyPressEvent ( QKeyEvent * event );
        void wheelEvent ( QWheelEvent * e );

        /// the attached parent map object
        QPointer<IMap> theMap;
        /// the width of the skybox
        int xsize;
        /// the depth of the skybox
        int ysize;
        /// the vertical height of the skybox
        int zsize;

        /// the rotation in the x axis in[°]
        double xRotation;
        /// the rotation in the y axis in[°]
        double yRotation;
        /// the rotation in the z axis in[°]
        double zRotation;

        double xpos;
        double ypos;
        double zpos;

        double zoomFactor;

        QPoint mousePos;

        QPoint lastPos;

        quint32 skyBox[6];
};

#endif //CMAP3D_H

