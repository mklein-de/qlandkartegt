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
#include "CMainWindow.h"

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
    step = 5;

    xShift = 0;
    yShift = 0;
    zoomFactor = 1;

    eleZoomFactor = 1;
    maxElevation = 0;
    minElevation = 0;

    wallCollor = QColor::fromCmykF(0.40, 0.0, 1.0, 0);
    highBorderColor = QColor::fromRgbF(0.0, 0.0, 1.0, 0);

    selTrkPt = 0;

    createActions();
    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    setFocusPolicy(Qt::StrongFocus);

    QSettings cfg;
    map3DAct->setChecked(cfg.value("map/3D/3dmap", true).toBool());
    mapEleAct->setChecked(cfg.value("map/3D/trackonmap", false).toBool());
}

CMap3DWidget::~CMap3DWidget()
{
    makeCurrent();
    deleteTexture(mapTexture);
    glDeleteLists(object, 1);

    QSettings cfg;
    cfg.setValue("map/3D/3dmap", map3DAct->isChecked());
    cfg.setValue("map/3D/trackonmap", mapEleAct->isChecked());
}


void CMap3DWidget::loadMap()
{
    map = &CMapDB::self().getMap();
    connect(map, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(map, SIGNAL(sigChanged()),this,SLOT(slotChanged()));
}


void CMap3DWidget::createActions()
{
    map3DAct = new QAction(tr("3D Map"), this);
    map3DAct->setCheckable(true);
    map3DAct->setChecked(true);
    connect(map3DAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    showTrackAct = new QAction(tr("Show Track"), this);
    showTrackAct->setCheckable(true);
    showTrackAct->setChecked(true);
    connect(showTrackAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    mapEleAct = new QAction(tr("Track on Map"), this);
    mapEleAct->setCheckable(true);
    mapEleAct->setChecked(false);
    connect(mapEleAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    eleZoomInAct = new QAction(tr("zZoom In"), this);
    eleZoomInAct->setIcon(QIcon(":/icons/iconInc16x16"));
    connect(eleZoomInAct, SIGNAL(triggered()), this, SLOT(eleZoomIn()));
    eleZoomOutAct = new QAction(tr("zZoom Out"), this);
    eleZoomOutAct->setIcon(QIcon(":/icons/iconDec16x16"));
    connect(eleZoomOutAct, SIGNAL(triggered()), this, SLOT(eleZoomOut()));
    eleZoomResetAct = new QAction(tr("Reset zZoom"), this);
    eleZoomResetAct->setIcon(QIcon(":/icons/iconClear16x16"));
    connect(eleZoomResetAct, SIGNAL(triggered()), this, SLOT(eleZoomReset()));
}

void CMap3DWidget::changeMode()
{
    map3DAct->setChecked(!map3DAct->isChecked());
    slotChanged();
}

void CMap3DWidget::eleZoomOut()
{
    eleZoomFactor = eleZoomFactor / 1.2;
    updateGL();
}


void CMap3DWidget::eleZoomIn()
{
    eleZoomFactor = eleZoomFactor * 1.2;
    updateGL();
}


void CMap3DWidget::eleZoomReset()
{
    eleZoomFactor = 1;
    updateGL();
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
}


void CMap3DWidget::convert3D2Pt(double& u, double& v, double &ele)
{
    QSize s = map->getSize();
    u = u + s.width()/2;
    v = s.height()/2 - v;
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
    glVertex3d(-w/2, -h/2, minElevation);
    glTexCoord2d(1.0, 0.0);
    glVertex3d( w/2, -h/2, minElevation);
    glTexCoord2d(1.0, 1.0);
    glVertex3d( w/2,  h/2, minElevation);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(-w/2,  h/2, minElevation);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

int CMap3DWidget::getEleRegionSize()
{
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();
    return (int)(w/step + 1) * (int)(h/step + 1);
}

void CMap3DWidget::getEleRegion(float *buffer)
{
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();

    IMap& dem = CMapDB::self().getDEM();
    XY p1, p2;
    p1.u = 0;
    p1.v = 0;
    p2.u = w;
    p2.v = h;
    map->convertPt2Rad(p1.u, p1.v);
    map->convertPt2Rad(p2.u, p2.v);
    dem.getRegion(buffer, p1, p2, w/step + 1, h/step + 1);
}

float CMap3DWidget::getRegionValue(float *buffer, int x, int y) {
    QSize s = map->getSize();
    int w = s.width() / step + 1;
    return buffer[x + y * w];
}

void CMap3DWidget::draw3DMap()
{
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();
    float eleData[getEleRegionSize()];

    int ix, iy, iv, it, j, k, end;
    double x, y, u, v;
    GLdouble *vertices;
    GLdouble *texCoords;
    GLuint idx[4];

    // increment xcount, because the number of points are on one more
    // than number of lengths |--|--|--|--|
    int xcount = (w / step + 1);
    int ycount = (h / step + 1);

    double current_step_x = w / (double) (xcount - 1);
    double current_step_y = h / (double) (ycount - 1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mapTexture);

    /*
     * next code can be more optimal if used array of coordinates or VBO
     */
    vertices = new GLdouble[xcount * 3 * 2];
    texCoords = new GLdouble[xcount * 2 * 2];
    ix = 0;
    idx[0] = 0 + xcount;
    idx[1] = 1 + xcount;
    idx[2] = 1;
    idx[3] = 0;
    glVertexPointer(3, GL_DOUBLE, 0, vertices);
    glTexCoordPointer(2, GL_DOUBLE, 0, texCoords);
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(2.0);

    getEleRegion(eleData);
    for (iy = 0, y = 0; iy < ycount; y += current_step_y, iy++) {
        ix = ix % (xcount * 2);
        end = ix + xcount;
        for (x = 0, iv = ix * 3, it = ix * 2; ix < end; x += current_step_x, iv += 3, it += 2, ix++) {
            vertices[iv + 0] = x;
            vertices[iv + 1] = y;
            u = x;
            v = y;
            texCoords[it  + 0] = u / w;
            texCoords[it + 1] = 1 - v / h;
            vertices[iv + 2] = getRegionValue(eleData, ix, iy);
            convertPt23D(vertices[iv + 0], vertices[iv + 1], vertices[iv + 2]);
        }

        for (j = 0; j < 4; j++)
            idx[j] = idx[j] % (xcount * 2);

        if (iy == 0)
                continue;

        for (k = 0; k < xcount - 1; k ++) {
            glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, idx);
            for (j = 0; j < 4; j++)
                idx[j]++;
        }
        for (j = 0; j < 4; j++)
                idx[j]++;
    }
    delete [] vertices;
    delete [] texCoords;
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

void CMap3DWidget::updateElevationLimits()
{
    double x, y, ele;
    QSize s = map->getSize();
    double w = s.width();
    double h = s.height();
    float eleData[getEleRegionSize()];

    getEleRegion(eleData);
    minElevation = maxElevation = getRegionValue(eleData, 0, 0);

    for (y = 0; y < h - step; y += step)
        for (x = 0; x < w; x += step) {
            ele = getRegionValue(eleData, x / step, y /step);
            if (ele > maxElevation)
                    maxElevation = ele;

            if (ele < minElevation)
                    minElevation = ele;
        }
    if (! track.isNull() && (maxElevation - minElevation < 1)) {
        /*selected track exist and dem isn't present for this map*/
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
    }

    if (maxElevation - minElevation < 1) {
            /*selected track and deb are absent*/
            maxElevation = 1;
            minElevation = 0;
    }

}

GLuint CMap3DWidget::makeObject()
{
    GLuint list = glGenLists(1);

    updateElevationLimits();

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

    /* subtract the offset and set the Z axis scale */
    glScalef(1.0, 1.0, eleZoomFactor * (s.width() / 10.0) / (maxElevation - minElevation));
    glTranslated(0.0, 0.0, -minElevation);

    glCallList(object);

    /*draw axis*/
/*    glBegin(GL_LINES);

    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(100.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 100.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 100.0);

    glEnd();*/

    /*draw the grid*/
    int i, d = 100, n = 10;

    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);
    for(i = -n; i <= n; i ++) {
        glVertex3f(-d * n, i * d, minElevation);
        glVertex3f(d * n, i * d, minElevation);

        glVertex3f(i * d, -d * n, minElevation);
        glVertex3f(i * d, d * n, minElevation);
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
        convertPt23D(pt.u, pt.v, ele);

        glVertex3d(pt.u, pt.v, ele);
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
    GLdouble k1, z1, x0, xk, y0, yk, z0, zk;
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


    if(track.isNull()) return;

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
    switch (event->key()) {
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
        default:
            event->ignore();
            return;
    }
    if (zoomMap){
        map->zoom(zoomMap > 0 ? true : false, QPoint(s.width() / 2, s.height() / 2));
    }

    if (dx || dy){
        map->move(QPoint(dx, dy), QPoint(0, 0));
    }
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
    }
    else if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot - xRotSens * dy);
        setZRotation(zRot + zRotSens * dx);
    }
    else if (event->buttons() & Qt::MidButton) {
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
    glVertex3d(x2, y2, minElevation);
    glVertex3d(x1, y1, minElevation);
    qglColor(wallCollor.dark(c1));
    glVertex3d(x1, y1, z1);
    qglColor(wallCollor.dark(c2));
    glVertex3d(x2, y2, z2);

    qglColor(wallCollor.dark(c2));
    glVertex3d(x2, y2, z2);
    qglColor(wallCollor.dark(c1));
    glVertex3d(x1, y1, z1);
    qglColor(wallCollor);
    glVertex3d(x1, y1, minElevation);
    glVertex3d(x2, y2, minElevation);

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
    }
    else {
        if (in) {
            qDebug() << "in" << endl;
            zoomFactor *= 1.1;
        }
        else {
            qDebug() << "out" << endl;
            zoomFactor /= 1.1;
        }
    }
    updateGL();
}
