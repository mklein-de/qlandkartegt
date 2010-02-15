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
        void slotMapResize(const QSize& size);

    protected:
        void paintGL();
        void initializeGL();
        void resizeGL(int width, int height);

        void camera (void);

        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent ( QKeyEvent * event );

        /// the attached parent map object
        QPointer<IMap> theMap;
        /// the width [px] of the current parent map
        int xsize;
        /// the height [px] of the current parent map
        int ysize;

        double xRotation;
        double yRotation;
        double zRotation;

        double xpos;
        double ypos;
        double zpos;

        QPoint mousePos;

        QPoint lastPos;

    private:
        double normalizeAngle(double angle);
};

#endif //CMAP3D_H

