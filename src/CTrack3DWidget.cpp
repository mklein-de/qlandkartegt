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
    translateSens = 20;
    mouseRotSens = 0.3;
    keyboardRotSens = 1;

    eleZoomFactor = 1;
    cameraRotX = 0;

    wallCollor = QColor::fromCmykF(0.40, 0.0, 1.0, 0);
    highBorderColor = QColor::fromRgbF(0.0, 0.0, 1.0, 0);

    selTrkPt = 0;

    createActions();
    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    setFocusPolicy(Qt::StrongFocus);
}

void CTrack3DWidget::createActions()
{
    map3DAct = new QAction("3D map", this);
    map3DAct->setCheckable(true);
    map3DAct->setChecked(false);
    connect(map3DAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    showTrackAct = new QAction("show track", this);
    showTrackAct->setCheckable(true);
    showTrackAct->setChecked(true);
    connect(showTrackAct, SIGNAL(triggered()), this, SLOT(slotChanged()));

    eleZoomInAct = new QAction(tr("zZoom In"), this);
    connect(eleZoomInAct, SIGNAL(triggered()), this, SLOT(eleZoomIn()));
    eleZoomOutAct = new QAction(tr("zZoom Out"), this);
    connect(eleZoomOutAct, SIGNAL(triggered()), this, SLOT(eleZoomOut()));
    eleZoomResetAct = new QAction(tr("reset zZoom"), this);
    connect(eleZoomResetAct, SIGNAL(triggered()), this, SLOT(eleZoomReset()));
}

void CTrack3DWidget::eleZoomOut()
{
    eleZoomFactor = eleZoomFactor / 1.2;
    slotChanged();
}

void CTrack3DWidget::eleZoomIn()
{
    eleZoomFactor = eleZoomFactor * 1.2;
    slotChanged();
}

void CTrack3DWidget::eleZoomReset()
{
    eleZoomFactor = 1;
    slotChanged();
}
void CTrack3DWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(eleZoomInAct);
    menu.addAction(eleZoomOutAct);
    menu.addAction(eleZoomResetAct);
    menu.addAction(map3DAct);
    menu.addAction(showTrackAct);

    menu.exec(event->globalPos());
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

void CTrack3DWidget::convertPt23D(double& u, double& v, double &ele)
{
    u = u - width()/2;
    v = height()/2 - v;
    ele = ele * eleZoomFactor * (width() / 10.0) / maxElevation;
}

void CTrack3DWidget::convert3D2Pt(double& u, double& v, double &ele)
{
    u = u + width()/2;
    v = height()/2 - v;
    ele = ele / eleZoomFactor / (width() / 10.0) * maxElevation;
}

void CTrack3DWidget::drawFlatMap()
{
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
}

void CTrack3DWidget::draw3DMap()
{
    int i, j;
    double step = 5;
    double x, y, u, v;
    double w = width();
    double h = height();
    GLdouble vertices[4][3];
    GLdouble texCoords[4][2];

    IMap& map = CMapDB::self().getMap();

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glBindTexture(GL_TEXTURE_2D, mapTexture);

    IMap& dem = CMapDB::self().getDEM();
    /*
     * next code can be more optimal if used array of coordinates or VBO
     */
    for (y = 0; y < height() - step; y += step) {
        for (x = 0; x < width(); x += step) {
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
                map.convertPt2Rad(u, v);
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

void CTrack3DWidget::drawTrack()
{
    glLineWidth(2.0);
    double ele1, ele2;
    if(! track.isNull()) {
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

        pt1.u = trkpt->px.x();
        pt1.v = trkpt->px.y();
        ele1 = trkpt->ele;
        convertPt23D(pt1.u, pt1.v, ele1);

        while(trkpt != trkpts.end()) {
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }
            pt2.u = trkpt->px.x();
            pt2.v = trkpt->px.y();
            ele2 = trkpt->ele;
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

GLuint CTrack3DWidget::makeObject()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    if (showTrackAct->isChecked())
        drawTrack();

    //draw map
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    if (!map3DAct->isChecked()) {
            /*draw flat map*/
            drawFlatMap();
    } else {
            /*using DEM data file to display terrain in 3D*/
            draw3DMap();
    }
    glDisable(GL_TEXTURE_2D);

    glEndList();

    return list;
}

void CTrack3DWidget::initializeGL()
{
    int side = qMax(width(), height());
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

    glLoadIdentity();
//    glTranslated(0.0, 0.0, -2 * side);
    cameraTranslated(0.0, side/2, -2 * side);
    cameraRotated(-15, 0.1, 0.0, 0.0);

    slotChanged();
}

void CTrack3DWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    /* 20 is equal to value of a maximum zoom factor. */
    glFrustum(-width/200, width/200, -height/200, height/200, side/100, 20 * side);
    //glOrtho(-width, width, -height, height, 0, 20 * side);
    glMatrixMode(GL_MODELVIEW);
}

void CTrack3DWidget::cameraTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glLoadIdentity();
    glTranslated(x, y, z);
    glMultMatrixd(modelview);
}

void CTrack3DWidget::cameraRotated(GLdouble angle, const GLdouble x, GLdouble y, GLdouble z, bool correct)
{
    if (correct) {
            if (x < 0.1)
                cameraRotated(-cameraRotX, 1.0, 0.0, 0.0, false);
            else
                cameraRotX += angle;
    }

    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glLoadIdentity();
    glRotated(angle, x, y, z);
    glMultMatrixd(modelview);
    if (correct && x < 0.1)
        cameraRotated(cameraRotX, 1.0, 0.0, 0.0, false);
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

void CTrack3DWidget::keyPressEvent ( QKeyEvent * event )
{
    qDebug() << "CTrack3DWidget::keyPressEvent" << endl;
    switch (event->key())
    {
        case Qt::Key_Up:
        case Qt::Key_W:
            qDebug() << "up" << endl;
            cameraTranslated(0.0, 0.0, translateSens);
            break;

        case Qt::Key_Down:
        case Qt::Key_S:
            qDebug() << "down" << endl;
            cameraTranslated(0.0, 0.0, -translateSens);
            break;

        case Qt::Key_A:
            cameraTranslated(+translateSens, 0.0, 0.0);
            break;

        case Qt::Key_D:
            cameraTranslated(-translateSens, 0.0, 0.0);
            break;

        case Qt::Key_R:
            cameraRotated(-cameraRotX, 1.0, 0.0, 0.0, false);
            cameraTranslated(0.0, 0.0, -translateSens);
            cameraRotated(cameraRotX, 1.0, 0.0, 0.0, false);
            break;

        case Qt::Key_F:
            cameraRotated(-cameraRotX, 1.0, 0.0, 0.0, false);
            cameraTranslated(0.0, 0.0, translateSens);
            cameraRotated(cameraRotX, 1.0, 0.0, 0.0, false);
            break;

        case Qt::Key_Left:
            cameraRotated(-keyboardRotSens, 0.0, 0.0, 1.0);
            break;

        case Qt::Key_Right:
            cameraRotated(keyboardRotSens, 0.0, 0.0, 1.0);
            break;

        case Qt::Key_PageUp:
            cameraRotated(-keyboardRotSens, 1.0, 0.0, 0.0);
            break;

        case Qt::Key_PageDown:
            cameraRotated(keyboardRotSens, 1.0, 0.0, 0.0);
            break;

        case Qt::Key_Home:
            cameraRotated(-keyboardRotSens, 0.0, 1.0, 0.0);
            break;

        case Qt::Key_End:
            cameraRotated(keyboardRotSens, 0.0, 1.0, 0.0);
            break;
    }
    updateGL();
}

void CTrack3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    cameraRotated(mouseRotSens * dy, 1.0, 0.0, 0.0);
    if (event->buttons() & Qt::LeftButton) {
        cameraRotated(mouseRotSens * dx, 0.0, 0.0, 1.0);
    } else if (event->buttons() & Qt::MidButton) {
        cameraRotated(mouseRotSens * dx, 0.0, 0.0, 1.0);
    }
    lastPos = event->pos();
    updateGL();
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
