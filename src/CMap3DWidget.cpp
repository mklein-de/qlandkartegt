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
#include "CMap3DWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CMapQMAP.h"
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

CMap3DWidget::CMap3DWidget(QWidget * parent)
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
    maxElevation = 50;
    minElevation = 0;

    wallCollor = QColor::fromCmykF(0.40, 0.0, 1.0, 0);
    highBorderColor = QColor::fromRgbF(0.0, 0.0, 1.0, 0);

    selTrkPt = 0;

    createActions();
    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    setFocusPolicy(Qt::StrongFocus);
}

void CMap3DWidget::loadMap()
{
    map = &CMapDB::self().getMap();
    connect(map, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(map, SIGNAL(sigChanged()),this,SLOT(slotChanged()));
}

void CMap3DWidget::createActions()
{
    map3DAct = new QAction("3D map", this);
    map3DAct->setCheckable(true);
    map3DAct->setChecked(false);
    connect(map3DAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    showTrackAct = new QAction("show track", this);
    showTrackAct->setCheckable(true);
    showTrackAct->setChecked(true);
    connect(showTrackAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    mapEleAct = new QAction("map elevation", this);
    mapEleAct->setCheckable(true);
    mapEleAct->setChecked(false);
    connect(mapEleAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    eleZoomInAct = new QAction(tr("zZoom In"), this);
    connect(eleZoomInAct, SIGNAL(triggered()), this, SLOT(eleZoomIn()));
    eleZoomOutAct = new QAction(tr("zZoom Out"), this);
    connect(eleZoomOutAct, SIGNAL(triggered()), this, SLOT(eleZoomOut()));
    eleZoomResetAct = new QAction(tr("reset zZoom"), this);
    connect(eleZoomResetAct, SIGNAL(triggered()), this, SLOT(eleZoomReset()));
}

void CMap3DWidget::eleZoomOut()
{
    eleZoomFactor = eleZoomFactor / 1.2;
    slotChanged();
}

void CMap3DWidget::eleZoomIn()
{
    eleZoomFactor = eleZoomFactor * 1.2;
    slotChanged();
}

void CMap3DWidget::eleZoomReset()
{
    eleZoomFactor = 1;
    slotChanged();
}
void CMap3DWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(eleZoomInAct);
    menu.addAction(eleZoomOutAct);
    menu.addAction(eleZoomResetAct);
    menu.addAction(map3DAct);
    menu.addAction(showTrackAct);
    menu.addAction(mapEleAct);

    menu.exec(event->globalPos());
}

CMap3DWidget::~CMap3DWidget()
{
    makeCurrent();
    deleteTexture(mapTexture);
    glDeleteLists(object, 1);
}

void CMap3DWidget::setMapTexture()
{
    QSize s = map->getSize();
    QPixmap pm(s.width(), s.height());
    QPainter p(&pm);
    p.eraseRect(pm.rect());
    map->draw(p);
    mapTexture = bindTexture(pm, GL_TEXTURE_2D);
    track = CTrackDB::self().highlightedTrack();
}

void CMap3DWidget::slotChanged()
{
    deleteTexture(mapTexture);
    setMapTexture();
    glDeleteLists(object, 1);
    object = makeObject();
    updateGL();
}

void CMap3DWidget::convertPt23D(double& u, double& v, double &ele)
{
    QSize s = map->getSize();
    u = u - s.width()/2;
    v = s.height()/2 - v;
    ele = ele * eleZoomFactor * (s.width() / 10.0) / maxElevation;
}

void CMap3DWidget::convert3D2Pt(double& u, double& v, double &ele)
{
    QSize s = map->getSize();
    u = u + s.width()/2;
    v = s.height()/2 - v;
    ele = ele / eleZoomFactor / (s.width() / 10.0) * maxElevation;
}

void CMap3DWidget::drawFlatMap()
{
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(-w/2, -h/2, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d( w/2, -h/2, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d( w/2,  h/2, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(-w/2,  h/2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CMap3DWidget::draw3DMap()
{
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();

    int i, j;
    double step = 5;
    double x, y, u, v;
    GLdouble vertices[4][3];
    GLdouble texCoords[4][2];

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glBindTexture(GL_TEXTURE_2D, mapTexture);

    IMap& dem = CMapDB::self().getDEM();
    /*
     * next code can be more optimal if used array of coordinates or VBO
     */
    for (y = 0; y < h - step; y += step) {
        for (x = 0; x < w; x += step) {
            /* copy points 3 -> 0, 2 -> 1 */
            for (j = 0; j < 3; j++) {
                    vertices[0][j] = vertices[3][j];
                    vertices[1][j] = vertices[2][j];
                    if (j < 2) {
                            texCoords[0][j] = texCoords[3][j];
                            texCoords[1][j] = texCoords[2][j];
                    }
            }
            /* compute values for points 2,3 */
            for (i =2; i < 4; i ++) {
                vertices[i][0] = x;
                vertices[i][1] = y;
                if ((i % 4) == 2)
                        vertices[i][1] += 10;
                u = vertices[i][0];
                v = vertices[i][1];
                texCoords[i][0] = u / w;
                texCoords[i][1] = 1 - v / h;
                map->convertPt2Rad(u, v);
                // FIXME can't use map instead of dem. need investigation.
                vertices[i][2] = dem.getElevation(u,v);
                convertPt23D(vertices[i][0], vertices[i][1], vertices[i][2]);
            }
            /* points 0, 1 are absent on the first iteration, so spip it*/
            if (x > step/2)
                for (i = 0; i < 4; i++) {
                    glTexCoord2d(texCoords[i][0], texCoords[i][1]);
                    glVertex3d(vertices[i][0], vertices[i][1], vertices[i][2]);
                }
        }
    }

    glEnd();
    glDisable(GL_TEXTURE_2D);

}

void CMap3DWidget::drawTrack()
{
    glLineWidth(2.0);
    double ele1, ele2;
    IMap& dem = CMapDB::self().getDEM();

    if (! track.isNull()) {
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

        pt1.u = trkpt->lon * DEG_TO_RAD;
        pt1.v = trkpt->lat * DEG_TO_RAD;
        if (mapEleAct->isChecked())
            ele1 = dem.getElevation(pt1.u, pt1.v) + 1;
        else
            ele1 = trkpt->ele;
        map->convertRad2Pt(pt1.u, pt1.v);
        convertPt23D(pt1.u, pt1.v, ele1);

        while(trkpt != trkpts.end()) {
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }
            pt2.u = trkpt->lon * DEG_TO_RAD;
            pt2.v = trkpt->lat * DEG_TO_RAD;
            if (mapEleAct->isChecked())
                ele2 = dem.getElevation(pt2.u, pt2.v) + 1;
            else
                ele2 = trkpt->ele +1;
            map->convertRad2Pt(pt2.u, pt2.v);
            convertPt23D(pt2.u, pt2.v, ele2);

            quad(pt1.u, pt1.v, ele1, pt2.u, pt2.v, ele2);
            ele1 = ele2;
            pt1 = pt2;
            ++trkpt;
        }
    }

    // restore line width by default
    glLineWidth(1);

}

GLuint CMap3DWidget::makeObject()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    if (showTrackAct->isChecked())
        drawTrack();

    //draw map
    if (!map3DAct->isChecked())
            /*draw flat map*/
            drawFlatMap();
    else
            /*using DEM data file to display terrain in 3D*/
            draw3DMap();

    glEndList();

    return list;
}

void CMap3DWidget::setXRotation(double angle)
{
    normalizeAngle(&angle);
    if (angle > 0 && angle < 90) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void CMap3DWidget::setZRotation(double angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void CMap3DWidget::initializeGL()
{
    loadMap();
    QSize s = map->getSize();
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
}

void CMap3DWidget::paintGL()
{
    QSize s = map->getSize();
    int side = qMax(s.width(), s.height());
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
            pt.u = selTrkPt->lon * DEG_TO_RAD;
            pt.v = selTrkPt->lat * DEG_TO_RAD;
            map->convertRad2Pt(pt.u, pt.v);

            pt.u -= s.width()/2;
            pt.v = s.height()/2 - pt.v;
            ele = selTrkPt->ele * eleZoomFactor * (s.width() / 10.0) / maxElevation;

            glVertex3d(pt.u,pt.v,ele);
            glEnd();
    }
}

void CMap3DWidget::resizeGL(int width, int height)
{
    int side = qMax(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* 20 is equal to value of a maximum zoom factor. */
    glFrustum(-width/200, width/200, -height/200, height/200, side/100, 20 * side);
    //glOrtho(-width, width, -height, height, 0, 20 * side);
    glMatrixMode(GL_MODELVIEW);
}

void CMap3DWidget::convertDsp2Z0(QPoint &a)
{
    GLdouble projection[16];
	GLdouble modelview[16];
	GLdouble k1, z1, x1, y1, x0, xk, y0, yk, z0, zk;
	GLint viewport[4];
    GLsizei vx, vy;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    vx = a.x();
    vy = height() - a.y();
    gluUnProject(vx, vy, 0, modelview, projection, viewport, &x0, &y0, &z0);
    gluUnProject(vx, vy, 1, modelview, projection, viewport, &xk, &yk, &zk);

    xk -= x0;
    yk -= y0;
    zk -= z0;
    /* the line equation A0 + tAk, where A0 = |x0, y0, z0|, Ak = |xk, yk, zk| */
    /* point of intersection with flat z = 0 */
    z1 = 0;
    k1 = (z1 - z0) / zk;
    a.rx() = x0 + xk * k1;
    a.ry() = y0 + yk * k1;
}

void CMap3DWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
    GLdouble projection[16];
    GLdouble modelview[16];
    GLdouble gl_x0, gl_y0, gl_z0;
    double x0, y0, z0;
    GLsizei vx, vy;
    GLfloat depth;
    GLint viewport[4];
    vx = event->pos().x();
    vy = height() - event->pos().y();
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    glReadPixels(vx, vy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    gluUnProject(vx, vy, depth, modelview, projection, viewport, &gl_x0, &gl_y0, &gl_z0);
    x0 = gl_x0;
    y0 = gl_y0;
    z0 = gl_z0;
    convert3D2Pt(x0, y0, z0);

    selTrkPt = 0;
    int d1 = 20;

    XY p;

    QList<CTrack::pt_t>& pts          = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator pt  = pts.begin();
    while(pt != pts.end()) {
        if(pt->flags & CTrack::pt_t::eDeleted) {
            ++pt; continue;
        }
        p.u = pt->lon * DEG_TO_RAD;
        p.v = pt->lat * DEG_TO_RAD;
        map->convertRad2Pt(p.u, p.v);

        int d2 = abs(x0 - p.u) + abs(y0 - p.v);

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

void CMap3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void CMap3DWidget::expandMap(bool zoomIn)
{
    QSize s = map->getSize();
    double zoomFactor = zoomIn ? 1.1 : 1/1.1;
    XY pv;
    /*save coord of the center map*/
    pv.u = s.width() / 2;
    pv.v = s.height() / 2;
    map->convertPt2Rad(pv.u, pv.v);

    map->resize(QSize(s.width() * zoomFactor, s.height() * zoomFactor));

    /*restore coord of the center map*/
    map->convertRad2Pt(pv.u, pv.v);
    s = map->getSize();
    map->move(QPoint(pv.u, pv.v), QPoint(s.width()/2, s.height()/2));
}

void CMap3DWidget::keyReleaseEvent ( QKeyEvent * event )
{
    pressedKeys.remove(event->key());
}

void CMap3DWidget::keyPressEvent ( QKeyEvent * event )
{
    pressedKeys.insert(event->key());

    qint32 dx = 0, dy = 0;
    qint32 zoomMap = 0;
    QSize s = map->getSize();
    switch (event->key())
    {
        case Qt::Key_Up:
            dy -= 100;
            break;

        case Qt::Key_Down:
            dy += 100;
            break;

        case Qt::Key_Left:
            dx -= 100;
            break;

        case Qt::Key_Right:
            dx += 100;
            break;
        case Qt::Key_PageUp:
            zoomMap = 1;
            break;
        case Qt::Key_PageDown:
            zoomMap = -1;
            break;
        case Qt::Key_Home:
            expandMap(true);
            break;
        case Qt::Key_End:
            expandMap(false);
            break;
    }
    if (zoomMap)
            map->zoom(zoomMap > 0 ? true : false, QPoint(s.width() / 2, s.height() / 2));
    if (dx || dy)
        map->move(QPoint(dx, dy), QPoint(0, 0));
    updateGL();
}

void CMap3DWidget::focusOutEvent ( QFocusEvent * event )
{
    pressedKeys.clear();
}

void CMap3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (pressedKeys.contains(Qt::Key_M)) {
        QPoint p1 = event->pos(), p2 = lastPos;
        convertDsp2Z0(p1);
        convertDsp2Z0(p2);
        dx = -(p1.x() - p2.x());
        dy = p1.y() - p2.y();
        map->move(QPoint(dx, dy), QPoint(0, 0));
    } else if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot - xRotSens * dy);
        setZRotation(zRot + zRotSens * dx);
    } else if (event->buttons() & Qt::MidButton) {
        xShift += dx / zoomFactor;
        yShift -= dy / zoomFactor;
    }
    lastPos = event->pos();
    updateGL();
}

void CMap3DWidget::quad(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2)
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

void CMap3DWidget::normalizeAngle(double *angle)
{
    while (*angle < 0)
        *angle += 360;
    while (*angle > 360)
        *angle -= 360;
}

void CMap3DWidget::wheelEvent ( QWheelEvent * e )
{
    bool in = CResources::self().flipMouseWheel() ? (e->delta() > 0) : (e->delta() < 0);
    if (pressedKeys.contains(Qt::Key_M)) {
        QSize s = map->getSize();
        map->zoom(in, QPoint(s.width() / 2, s.height() / 2));
    } else {
        if (in) {
            qDebug() << "in" << endl;
            zoomFactor *= 1.1;
        } else {
            qDebug() << "out" << endl;
            zoomFactor /= 1.1;
        }
    }
    updateGL();
}
