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

#include "CMap3D.h"

#include <QtGui>

CMap3D::CMap3D(IMap * map, QWidget * parent)
: QGLWidget(parent)
, xRotation(45)
, yRotation(0)
, zRotation(0)
, xpos(0)
, ypos(-400)
, zpos(-200)
{
    theMap = map;
    connect(map, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(map, SIGNAL(sigResize(const QSize&)), this, SLOT(slotMapResize(const QSize&)));

    slotMapResize(map->getSize());
}

CMap3D::~CMap3D()
{
    qDebug() << "CMap3D::~CMap3D()";
}

void CMap3D::initializeGL()
{
    qDebug() << "void CMap3D::initializeGL()";

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void CMap3D::resizeGL(int width, int height)
{
    qDebug() << "void CMap3D::resizeGL(int width, int height)" << width << height;

    glViewport (0, 0, (GLsizei)width, (GLsizei)height); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection
    glLoadIdentity ();
    gluPerspective (60, (GLfloat)width / (GLfloat)height, 1.0, 1000.0); //set the perspective (angle of sight, width, height, , depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model

    ypos = height;
}

void CMap3D::camera (void)
{
    qDebug() << xRotation << yRotation << xpos << ypos << zpos;
    glRotatef(xRotation,1.0,0.0,0.0);  //rotate our camera on teh x-axis (left and right)
    glRotatef(yRotation,0.0,1.0,0.0);  //rotate our camera on the y-axis (up and down)
    glTranslated(-xpos,-ypos,-zpos); //translate the screen to the position of our camera
}

void CMap3D::paintGL()
{
    glClearColor (0.0,0.0,0.0,1.0); //clear the screen to black
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
    glLoadIdentity();

    camera();

    glEnable (GL_DEPTH_TEST); //enable the depth testing
    glEnable (GL_LIGHTING); //enable the lighting
    glEnable (GL_LIGHT0); //enable LIGHT0, our Diffuse Light
    glShadeModel (GL_SMOOTH); //set the shader to smooth shader

    // coord sytsem lines
    glBegin(GL_LINES);

    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-100.0, 0.0, 0.0);
    glVertex3f( 100.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -100.0, 0.0);
    glVertex3f(0.0,  100.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, -100.0);
    glVertex3f(0.0, 0.0,  100.0);

    glEnd();

    /*draw the grid*/
    int i, d = 100, n = 10;

    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);
    for(i = -n; i <= n; i ++)
    {
        glVertex3f(-d * n, i * d, 0);
        glVertex3f(d * n, i * d, 0);

        glVertex3f(i * d, -d * n, 0);
        glVertex3f(i * d, d * n, 0);
    }
    glEnd();
}

void CMap3D::slotMapResize(const QSize& size)
{
    qDebug() << "void CMap3D::slotMapResize()";
    xsize = size.width();
    ysize = size.height();
}

void CMap3D::mouseMoveEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton)
    {
        QPoint diff = mousePos - lastPos;
        xRotation = normalizeAngle(xRotation + (double) diff.y()); //set the xrot to xrot with the addition of the difference in the y position
        yRotation = normalizeAngle(yRotation + (double) diff.x());  //set the xrot to yrot with the addition of the difference in the x position
//         xRotation = normalizeAngle(xRotation - 0.3 * dy);
//         zRotation = normalizeAngle(zRotation + 0.3 * dx);
    }

    lastPos = mousePos;
    updateGL();
}

void CMap3D::keyPressEvent ( QKeyEvent * event )
{

}

double CMap3D::normalizeAngle(double angle)
{
    while (angle < 0)
    {
        angle += 360;
    }

    while (angle > 360)
    {
        angle -= 360;
    }

    return angle;
}

