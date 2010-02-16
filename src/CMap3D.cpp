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
#include "CResources.h"

#include <QtGui>

CMap3D::CMap3D(IMap * map, QWidget * parent)
: QGLWidget(parent)
, xRotation(280)
, yRotation(0)
, zRotation(0)
, xpos(0)
, ypos(-400)
, zpos(200)
, zoomFactor(0.5)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    theMap = map;
    connect(map, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

CMap3D::~CMap3D()
{
    qDebug() << "CMap3D::~CMap3D()";

    for (int i = 0; i < 6; i++)
    {
        deleteTexture(skyBox[i]);
    }

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

    for (int i = 0; i < 6; i++)
    {
        QImage img(tr(":/skybox/%1.bmp").arg(i));
        skyBox[i] = bindTexture(img, GL_TEXTURE_2D);
    }

}

void CMap3D::resizeGL(int width, int height)
{
    qDebug() << "void CMap3D::resizeGL(int width, int height)" << width << height;

    int side = width > height ? width : height;
    xsize = width;
    ysize = height;
    zsize = side;

    glViewport (0, 0, (GLsizei)width, (GLsizei)height); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection
    glLoadIdentity ();
    gluPerspective (60, (GLfloat)width / (GLfloat)height, 1.0, 2*side); //set the perspective (angle of sight, width, height, , depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model


}

void CMap3D::setPOV (void)
{
    glRotatef(xRotation,1.0,0.0,0.0);
    glRotatef(yRotation,0.0,1.0,0.0);
    glRotatef(zRotation,0.0,0.0,1.0);
    glTranslated(-xpos,-ypos,-zpos);
    glScalef(zoomFactor, zoomFactor, zoomFactor);
}

void CMap3D::paintGL()
{
    glClearColor (0.0,0.0,0.0,0.0); //clear the screen to black
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
    glLoadIdentity();

    setPOV();

    drawSkybox();
    drawCenterStar();
    drawBaseGrid();
}

void CMap3D::drawSkybox()
{
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0, 1.0, 0.0,1.0f);

    // Save Current Matrix
    glPushMatrix();

    // First apply scale matrix
    glScalef(xsize, ysize, zsize);

    float f = 1;
    float r = 1.005f;            // If you have border issues change this to 1.005f
    glBindTexture(GL_TEXTURE_2D,skyBox[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f( r/f, 1.0f/f, r/f);
    glTexCoord2f(1, 1); glVertex3f(-r/f, 1.0f/f, r/f);
    glTexCoord2f(1, 0); glVertex3f(-r/f, 1.0f/f,-r/f);
    glTexCoord2f(0, 0); glVertex3f( r/f, 1.0f/f,-r/f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,skyBox[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-1.0f/f,  r/f, r/f);
    glTexCoord2f(1, 1); glVertex3f(-1.0f/f, -r/f, r/f);
    glTexCoord2f(1, 0); glVertex3f(-1.0f/f, -r/f,-r/f);
    glTexCoord2f(0, 0); glVertex3f(-1.0f/f,  r/f,-r/f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,skyBox[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-r/f, -1.0f/f,  r/f);
    glTexCoord2f(1, 1); glVertex3f( r/f, -1.0f/f,  r/f);
    glTexCoord2f(1, 0); glVertex3f( r/f, -1.0f/f, -r/f);
    glTexCoord2f(0, 0); glVertex3f(-r/f, -1.0f/f, -r/f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,skyBox[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(1.0f/f, -r/f, r/f);
    glTexCoord2f(1, 1); glVertex3f(1.0f/f,  r/f, r/f);
    glTexCoord2f(1, 0); glVertex3f(1.0f/f,  r/f,-r/f);
    glTexCoord2f(0, 0); glVertex3f(1.0f/f, -r/f,-r/f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,skyBox[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1); glVertex3f( r/f, r/f, 1.0f/f);
    glTexCoord2f(1, 0); glVertex3f( r/f,-r/f, 1.0f/f);
    glTexCoord2f(0, 0); glVertex3f(-r/f,-r/f, 1.0f/f);
    glTexCoord2f(0, 1); glVertex3f(-r/f, r/f, 1.0f/f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,skyBox[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f( r/f, r/f, -1.0f/f);
    glTexCoord2f(0, 0); glVertex3f(-r/f, r/f, -1.0f/f);
    glTexCoord2f(0, 1); glVertex3f(-r/f,-r/f, -1.0f/f);
    glTexCoord2f(1, 1); glVertex3f( r/f,-r/f, -1.0f/f);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void CMap3D::drawCenterStar()
{
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
}

void CMap3D::drawBaseGrid()
{
    /*draw the grid*/
    int i, d = 100, n;

    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);

    n = (xsize + 50) / 100;
    for(i = -n; i <= n; i ++)
    {
        glVertex3f(i * d, -ysize, 0);
        glVertex3f(i * d, +ysize, 0);
    }

    n = (ysize + 50) / 100;
    for(i = -n; i <= n; i ++)
    {
        glVertex3f(-xsize, i * d, 0);
        glVertex3f(+xsize, i * d, 0);
    }
    glEnd();
}

void CMap3D::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();

    lastPos = mousePos;
}

void CMap3D::mouseMoveEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton)
    {
        QPoint diff = mousePos - lastPos;
        xRotation = normalizeAngle(xRotation + (double) diff.y() * 0.3); //set the xrot to xrot with the addition of the difference in the y position
        zRotation = normalizeAngle(zRotation + (double) diff.x() * 0.3);  //set the xrot to yrot with the addition of the difference in the x position
    }

    lastPos = mousePos;
    updateGL();
}

void CMap3D::wheelEvent ( QWheelEvent * e )
{
    bool in = CResources::self().flipMouseWheel() ? (e->delta() > 0) : (e->delta() < 0);
//    if (pressedKeys.contains(Qt::Key_M))
//    {
//        map->zoom(in, QPoint(mapSize.width() / 2, mapSize.height() / 2));
//    }
//    else
//    {
        if (in)
        {
            zoomFactor *= 1.1;
        }
        else
        {
            zoomFactor /= 1.1;
        }
//    }
    updateGL();
}


void CMap3D::keyPressEvent ( QKeyEvent * e )
{
    switch (e->key())
    {
        case Qt::Key_W:
        {
            if(e->modifiers() & Qt::ShiftModifier)
            {
                zpos += 10;
            }
            else
            {
                double zRotRad = (zRotation / 180 * PI);
                xpos += sin(zRotRad) * 4;
                ypos += cos(zRotRad) * 4;
            }
            break;
        }

        case Qt::Key_S:
        {
            if(e->modifiers() & Qt::ShiftModifier)
            {
                zpos -= 10;
            }
            else
            {
                double zRotRad = (zRotation / 180 * PI);
                xpos -= sin(zRotRad) * 4;
                ypos -= cos(zRotRad) * 4;
            }
            break;
        }

        case Qt::Key_A:
        {
            double zRotRad = (zRotation / 180 * PI);
            xpos -= cos(zRotRad) * 4;
            ypos += sin(zRotRad) * 4;
            break;
        }

        case Qt::Key_D:
        {
            double zRotRad = (zRotation / 180 * PI);
            xpos += cos(zRotRad) * 4;
            ypos -= sin(zRotRad) * 4;
            break;
        }

        default:
            e->ignore();
    }

    updateGL();
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

