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
#include <QPen>

#include "IMap.h"

class CMap3D : public QGLWidget
{
    Q_OBJECT;
    public:
        CMap3D(IMap * map, QWidget * parent);
        virtual ~CMap3D();

    protected slots:
        void slotChanged();
        void slotFPVModeChanged();
        void slotResetLight();

    protected:
        void paintGL();
        void initializeGL();
        void resizeGL(int width, int height);

        double normalizeAngle(double angle);
        void convertPt23D(double& u, double& v, double &ele);
        void convertMouse23D(double &u, double& v, double &ele);
        bool getEleRegion(QVector<qint16>& eleData, int& xcount, int& ycount);
        void getPoint(double v[], int xi, int yi, int xi0, int yi0, int xcount, int ycount, double current_step_x, double current_step_y, qint16 *eleData);

        /// set min / max elevation limits and zoomFactorZ
        void setElevationLimits();
        /// set the point of view
        void setPOV (void);
        /// create calll list for map
        void setMapObject();
        /// draw sky and clouds
        void drawSkybox();
        /// draw axes in the coord center
        void drawCenterStar();
        /// draw a grid in the x/y plane
        void drawBaseGrid();
        /// draw map on a plane without elevation
        void drawFlatMap();
        /// draw map as real 3D model
        void draw3DMap();

        /// draw compass on bottom of screen
        void drawCompass(QPainter& p);

        void drawElevation(QPainter& p);

        void drawHorizont(QPainter& p);

        void mouseMoveEvent(QMouseEvent *e);
        void mousePressEvent(QMouseEvent *e);
        void keyPressEvent ( QKeyEvent * e );
        void wheelEvent ( QWheelEvent * e );
        void contextMenuEvent(QContextMenuEvent *e);
        void showEvent( QShowEvent * e);
        bool eventFilter(QObject *o, QEvent *e);

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
        /// the actual x position
        double xpos;
        /// the actual y position
        double ypos;
        /// the actual z position
        double zpos;
        /// minimum elevation
        double minEle;
        /// maximum elevation
        double maxEle;
        /// zom factor for elevation, multiplied with zoomFactorZ
        double zoomFactorEle;
        /// the over all zoom factor
        double zoomFactor;
        /// the base zoome factor for zaxis
        double zoomFactorZ;
        /// the actual mouse position
        QPoint mousePos;
        /// the last mouse position
        QPoint lastPos;
        /// skybox texture IDs
        GLuint skyBox[6];
        /// set true to update complete map
        bool needsRedraw;
        /// the texture ID of the map pixmap
        GLuint mapTextureId;
        /// the object ID to replay map render
        GLuint mapObjectId;

        QAction * act3DMap;
        QAction * actFPVMode;
        QAction * actResetLight;

        /// set true if shift key is pressed.
        bool keyShiftPressed;
        /// set true if L key is pressed.
        bool keyLPressed;

        /// set true to use elevation shading
        bool light;
        /// x pos of light source
        double xLight;
        /// y pos of light source
        double yLight;
        /// z pos of light source
        double zLight;

        /// the difference from to true north to map's north
        double angleNorth;

        QPen pen0;
        QPen pen1;
        QPen pen2;


};

#endif //CMAP3D_H

