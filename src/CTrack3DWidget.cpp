/**********************************************************************************************
    Copyright (C) 2008 Andrew Vagin <avagin@gmail.com>

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
#include "CTrack3DWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"
#include "IMap.h"
#include "CMapDB.h"
#include "CResources.h"

#include <QtGui>
#include <QtOpenGL>
#include <QPixmap>
#include <QPainter>
#include <QGLPixelBuffer>

#include <math.h>

CTrack3DWidget::CTrack3DWidget(QWidget * parent)
    : QGLWidget(parent)
{
    object = 0;
    xRot = 45;
    zRot = 0;
    xRotSens = 0.3;
    zRotSens = 0.3;

    xShift = 0;
    yShift = 0;
    zoomFactor = 1;
    eleZoomFactor = 1;

    wallCollor = QColor::fromCmykF(0.40, 0.0, 1.0, 0);
    highBorderColor = QColor::fromRgbF(0.0, 0.0, 1.0, 0);

    selTrkPt = 0;

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
}


CTrack3DWidget::~CTrack3DWidget()
{
    makeCurrent();
    deleteTexture(mapTexture);
    glDeleteLists(object, 1);
}

void CTrack3DWidget::setMapTexture()
{
    QPixmap pm(width(), height());
    QPainter p(&pm);
    CMapDB::self().draw(p,pm.rect());
    mapTexture = bindTexture(pm, GL_TEXTURE_2D);
    track = CTrackDB::self().highlightedTrack();
}

void CTrack3DWidget::slotChanged()
{
    deleteTexture(mapTexture);
    setMapTexture();
    glDeleteLists(object, 1);
    object = makeObject();
    updateGL();
}

GLuint CTrack3DWidget::makeObject()
{
    int w = width();
    int h = height();
    double ele1, ele2;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glLineWidth(2.0);

    if(! track.isNull()) {
        IMap& map = CMapDB::self().getMap();
        XY pt1, pt2;

        QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
        QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
        maxElevation = trkpt->ele;
        minElevation = trkpt->ele;
        while(trkpt != trkpts.end()) {
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }
            if (trkpt->ele > maxElevation)
                maxElevation = trkpt->ele;
            if (trkpt->ele < minElevation)
                minElevation = trkpt->ele;
            ++trkpt;
        }

        trkpt = trkpts.begin();
        pt1.u = trkpt->px.x() - w/2;
        pt1.v = h/2 - trkpt->px.y();
        ele1 = trkpt->ele * eleZoomFactor * (width() / 10.0) / maxElevation;

        while(trkpt != trkpts.end()) {
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }
            pt2.u = trkpt->px.x() - w/2;;
            pt2.v = h/2 - trkpt->px.y();
            ele2 = trkpt->ele * eleZoomFactor * (width() / 10.0) / maxElevation;
            quad(pt1.u, pt1.v, ele1, pt2.u, pt2.v, ele2);
            ele1 = ele2;
            pt1 = pt2;
            ++trkpt;
        }
    }

    // restore line width by default
    glLineWidth(1);

    //draw map
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(-width()/2, -height()/2, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(width()/2, -height()/2, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(width()/2, height()/2, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(-width()/2, height()/2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);


    glEndList();

    return list;
}

QSize CTrack3DWidget::minimumSizeHint() const
{
    return QSize(400, 400);
}

QSize CTrack3DWidget::sizeHint() const
{
    return QSize(700, 700);
}

void CTrack3DWidget::setXRotation(double angle)
{
    normalizeAngle(&angle);
    if (angle > 0 && angle < 90) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void CTrack3DWidget::setZRotation(double angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void CTrack3DWidget::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    setMapTexture();
    object = makeObject();
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    slotChanged();
}

void CTrack3DWidget::paintGL()
{
    int side = qMax(width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslated(0.0, -0.25 * side, 0.0);
    glTranslated(0.0, 0.0, - 3 * side);
    glRotated(-xRot, 1.0, 0.0, 0.0);
    glScalef(zoomFactor, zoomFactor, zoomFactor);
    glTranslated(xShift * 2, 2 * yShift, 0.0);

    glRotated(zRot, 0.0, 0.0, 1.0);
    glCallList(object);

    /*draw axis*/
    glBegin(GL_LINES);

    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(100.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 100.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 100.0);

    glEnd();

    /*draw the grid*/
    int i, d = 100, n = 10;

    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);
    for(i = -n; i <= n; i ++) {
        glVertex3f(-d * n, i * d, 0.0);
        glVertex3f(d * n, i * d, 0.0);

        glVertex3f(i * d, -d * n, 0.0);
        glVertex3f(i * d, d * n, 0.0);
    }
    glEnd();

    // draw a selected track point
    if (selTrkPt) {
            XY pt;
            double ele;
            glColor3f(1.0, 0.0, 0.0);
            glPointSize(3.0);
            glBegin(GL_POINTS);
            pt.u = selTrkPt->px.x();
            pt.v = selTrkPt->px.y();
            pt.u -= width()/2;
            pt.v = height()/2 - pt.v;
            ele = selTrkPt->ele * eleZoomFactor * (width() / 10.0) / maxElevation;

            glVertex3d(pt.u,pt.v,ele);
            glEnd();
    }
}

void CTrack3DWidget::resizeGL(int width, int height)
{
    int side = qMax(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-width, width, -height, height, 2 * side, 6 * side);
    //glOrtho(-width, width, -height, height, 2 * side, 6 * side);
    glMatrixMode(GL_MODELVIEW);
}

void CTrack3DWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
    GLdouble projection[16];
    GLdouble modelview[16];
    GLdouble x0, y0, z0;
    GLsizei vx, vy;
    GLfloat depth;
    GLint viewport[4];
    vx = event->pos().x();
    vy = height() - event->pos().y();
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    glReadPixels(vx, vy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    gluUnProject(vx, vy, depth, modelview, projection, viewport, &x0, &y0, &z0);

    x0 += width() / 2;
    y0 = height() / 2 - y0;

    selTrkPt = 0;
    int d1 = 20;

    QList<CTrack::pt_t>& pts          = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator pt  = pts.begin();
    while(pt != pts.end()) {
        if(pt->flags & CTrack::pt_t::eDeleted) {
            ++pt; continue;
        }

        int d2 = abs(x0 - pt->px.x()) + abs(y0 - pt->px.y());

        if(d2 < d1) {
            selTrkPt = &(*pt);
            d1 = d2;
        }

        ++pt;
    }
    if (selTrkPt) {
        selTrkPt->flags |= CTrack::pt_t::eSelected;
        track->setPointOfFocus(selTrkPt->idx);
        updateGL();
    }
}

void CTrack3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void CTrack3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot - xRotSens * dy);
        setZRotation(zRot + zRotSens * dx);
    } else if (event->buttons() & Qt::RightButton) {
        xShift += dx / zoomFactor;
        yShift -= dy / zoomFactor;
        updateGL();
    }
    lastPos = event->pos();
}

void CTrack3DWidget::quad(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2)
{
    glBegin(GL_QUADS);
    double c1, c2;
    // compute colors
    c1 = z1 / maxElevation * 255;
    c2 = z2 / maxElevation * 255;

    qglColor(wallCollor);
    glVertex3d(x2, y2, 0);
    glVertex3d(x1, y1, 0);
    qglColor(wallCollor.dark(c1));
    glVertex3d(x1, y1, z1);
    qglColor(wallCollor.dark(c2));
    glVertex3d(x2, y2, z2);

    qglColor(wallCollor.dark(c2));
    glVertex3d(x2, y2, z2);
    qglColor(wallCollor.dark(c1));
    glVertex3d(x1, y1, z1);
    qglColor(wallCollor);
    glVertex3d(x1, y1, 0);
    glVertex3d(x2, y2, 0);

    glEnd();

    glBegin(GL_LINES);
    qglColor(highBorderColor);
    glVertex3d(x1, y1, z1);
    glVertex3d(x2, y2, z2);
    glEnd();
}

void CTrack3DWidget::normalizeAngle(double *angle)
{
    while (*angle < 0)
        *angle += 360;
    while (*angle > 360)
        *angle -= 360;
}

void CTrack3DWidget::wheelEvent ( QWheelEvent * e )
{
    bool in = CResources::self().flipMouseWheel() ? (e->delta() > 0) : (e->delta() < 0);
    if (in) {
        qDebug() << "in" << endl;
        zoomFactor *= 1.1;
    } else {
        qDebug() << "out" << endl;
        zoomFactor /= 1.1;
    }
    updateGL();
}
