/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "IMouse.h"
#include "CCanvas.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CRouteDB.h"
#include "COverlayDB.h"
#include "CWpt.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IOverlay.h"
#include "IUnit.h"
#include "IMap.h"
#include "CDlgEditWpt.h"
#include "GeoMath.h"
#include "CSearch.h"
#include "CSearchDB.h"
#include "IMapSelection.h"
#include <QtGui>

QPointer<IOverlay> IMouse::selOverlay;

QPointF IMouse::pos1Pixel(-1,-1);
QPointF IMouse::pos1LonLat(-1,-1);

IMouse::IMouse(CCanvas * canvas)
: QObject(canvas)
, cursor(QPixmap(":/cursor/Arrow"))
, canvas(canvas)
, selTrkPt(0)
, selRtePt(0)
, doSpecialCursorWpt(false)
, doSpecialCursorSearch(false)
, doShowWptBuddies(false)
{
    rectMarkWpt         = QRect(16,-8,16,16);
    rectDelWpt          = QRect(-5, 3,16,16);
    rectMoveWpt         = QRect(37, 3,16,16);
    rectEditWpt         = QRect(-5,29,16,16);
    rectCopyWpt         = QRect(37,29,16,16);
    rectViewWpt         = QRect(16,40,16,16);

    rectDelSearch       = QRect(0,0,16,16);
    rectConvertSearch   = QRect(0,32,16,16);
    rectCopySearch      = QRect(32,32,16,16);

}


IMouse::~IMouse()
{

}


void IMouse::startRect(const QPoint& p)
{
    rect.setTopLeft(p);
    rect.setSize(QSize(0,0));
}


void IMouse::resizeRect(const QPoint& p)
{
    rect.setBottomRight(p);
    canvas->update();
}


void IMouse::drawRect(QPainter& p)
{
    p.setBrush(QBrush( QColor(230,230,255,100) ));
    //     p.setPen(QPen(QColor(150,150,255),3));
    p.setPen(QPen(QColor(255,255,0),3));
    p.drawRect(rect);
}


#define LENGTH 100
void IMouse::drawPos1(QPainter& p)
{
    if(pos1Pixel.x() == -1 || pos1Pixel.y() == -1)
    {
        return;
    }

    IMap& map = CMapDB::self().getMap();
    double u = pos1LonLat.x();
    double v = pos1LonLat.y();
    map.convertRad2Pt(u,v);

    p.setPen(QPen(Qt::white, 5));
    p.drawLine(u - 10, v, u + LENGTH, v);
    p.drawLine(u, v - 10, u, v + LENGTH);

    p.setPen(QPen(Qt::blue, 3));
    p.drawLine(u - 10, v, u + LENGTH, v);
    p.drawLine(u, v - 10, u, v + LENGTH);

    CCanvas::drawText("Pos. 1",p, QRect(u,v, LENGTH, - 20));

    QString fn = map.getFilename(u, v);
    QFileInfo fi(fn);
    CCanvas::drawText(fi.fileName(), p, QRect(u,v, LENGTH, 20));
}

void IMouse::drawSelWpt(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    if(!selWpt.isNull())
    {
        double u = selWpt->lon * DEG_TO_RAD;
        double v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPixmap icon = selWpt->getIcon().scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


        p.setPen(CCanvas::penBorderBlue);
        p.setBrush(CCanvas::brushBackWhite);
        p.drawEllipse(u - 35, v - 35, 70, 70);
        p.drawPixmap(u - 8 , v - 8, icon);

        p.save();
        p.translate(u - 24, v - 24);

        if(selWpt->isDeletable())
        {
            p.drawPixmap(rectDelWpt, QPixmap(":/icons/iconClear16x16.png"));
        }

        if(selWpt->isMovable())
        {
            p.drawPixmap(rectMoveWpt, QPixmap(":/icons/iconMove16x16.png"));
        }
        else if(selWpt->isGeoCache() && selWpt->hasBuddies())
        {
            p.drawPixmap(rectMoveWpt, QPixmap(":/icons/iconWaypoint16x16.png"));
        }

        p.drawPixmap(rectEditWpt, QPixmap(":/icons/iconEdit16x16.png"));
        p.drawPixmap(rectCopyWpt, QPixmap(":/icons/iconClipboard16x16.png"));
        if(!selWpt->images.isEmpty() && !selWpt->images[0].filePath.isEmpty())
        {
            p.drawPixmap(rectViewWpt, QPixmap(":/icons/iconRaster16x16.png"));
        }

        if(selWpt->selected)
        {
            p.drawPixmap(rectMarkWpt, QPixmap(":/icons/iconCheckbox16x16.png"));
        }

        p.restore();

        if(doShowWptBuddies)
        {
            return;
        }

        QString         str = selWpt->getInfo();
        QFont           f   = CResources::self().getMapFont();
        QFontMetrics    fm(f);
        QRect           r1  = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
        r1.moveTopLeft(QPoint(u + 55, v));

        QRect           r2 = r1;
        r2.setWidth(r1.width() + 20);
        r2.moveLeft(r1.left() - 10);
        r2.setHeight(r1.height() + 20);
        r2.moveTop(r1.top() - 10);

        p.setPen(CCanvas::penBorderBlue);
        p.setBrush(CCanvas::brushBackWhite);
        PAINT_ROUNDED_RECT(p,r2);


        p.setFont(CResources::self().getMapFont());
        p.setPen(Qt::darkBlue);
        p.drawText(r1, Qt::AlignJustify|Qt::AlignTop|Qt::TextWordWrap,str);

        if(!selWpt->images.isEmpty())
        {
            QRect r = selWpt->images[0].pixmap.rect();

            if(r.width() > 300)
            {
                qint32 h = r.height() * 300 / r.width();

                r.setWidth(300);
                r.setHeight(h);
            }

            p.save();
            p.translate(u - (r.width() + 45), v);
            r.adjust(-1,-1,+1,+1);

            p.drawPixmap(0,0,selWpt->images[0].pixmap.scaled(r.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            p.setPen(CCanvas::penBorderBlue);
            p.setBrush(Qt::NoBrush);
            PAINT_ROUNDED_RECT(p,r);

            p.restore();
        }

    }
}


void IMouse::drawSelSearch(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    if(!selSearch.isNull())
    {
        double u = selSearch->lon * DEG_TO_RAD;
        double v = selSearch->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(CCanvas::penBorderBlue);
        p.setBrush(CCanvas::brushBackWhite);
        p.drawEllipse(u - 35, v - 35, 70, 70);
        p.drawPixmap(u-8 , v-8, QPixmap(":/icons/iconBullseye16x16"));

        p.save();
        p.translate(u - 24, v - 24);
        p.drawPixmap(rectDelSearch, QPixmap(":/icons/iconClear16x16.png"));
        p.drawPixmap(rectConvertSearch, QPixmap(":/icons/iconAdd16x16.png"));
        p.drawPixmap(rectCopySearch, QPixmap(":/icons/iconClipboard16x16.png"));
        p.restore();

    }
}


void IMouse::drawSelTrkPt(QPainter& p)
{
    IMap& map       = CMapDB::self().getMap();

    if(selTrkPt && !selWpt)
    {
        CTrack * track = CTrackDB::self().highlightedTrack();
        if(track == 0)
        {
            return;
        }

        QString val, unit;
        double u = selTrkPt->lon * DEG_TO_RAD;
        double v = selTrkPt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(CCanvas::penBorderBlue);
        p.setBrush(CCanvas::brushBackWhite);
        p.drawEllipse(QRect(u - 5,  v - 5, 11, 11));

        QString str;
        if(selTrkPt->timestamp != 0x00000000 && selTrkPt->timestamp != 0xFFFFFFFF)
        {
            QDateTime time = QDateTime::fromTime_t(selTrkPt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();

            quint32 total = track->getTotalTime();
            if(total)
            {
                quint32 t1s = selTrkPt->timeSinceStart;
                quint32 t2s = total - selTrkPt->timeSinceStart;

                quint32 t1h = qreal(t1s)/3600;
                quint32 t2h = qreal(t2s)/3600;

                quint32 t1m = quint32(qreal(t1s - t1h * 3600)/60  + 0.5);
                quint32 t2m = quint32(qreal(t2s - t2h * 3600)/60  + 0.5);

                quint32 t1p = quint32(qreal(100 * t1s) / total + 0.5);
                quint32 t2p = 100 - t1p;


                str += "\n";
                str += tr("%4 %3 %1:%2h (%5%)").arg(t1h).arg(t1m, 2, 10, QChar('0')).arg(QChar(0x21A4)).arg(QChar(0x2690)).arg(t1p);
                str += tr(" | (%5%) %1:%2h %3 %4").arg(t2h).arg(t2m, 2, 10, QChar('0')).arg(QChar(0x21A6)).arg(QChar(0x2691)).arg(t2p);
            }

        }

        if(str.count()) str += "\n";
        IUnit::self().meter2distance(selTrkPt->distance, val, unit);
        str += tr("%5 %4 %1%2 (%3%)").arg(val).arg(unit).arg(selTrkPt->distance * 100.0 / track->getTotalDistance(),0,'f',0).arg(QChar(0x21A4)).arg(QChar(0x2690));
        IUnit::self().meter2distance(track->getTotalDistance() - selTrkPt->distance, val, unit);
        str += tr(" | (%3%) %1%2 %4 %5").arg(val).arg(unit).arg((track->getTotalDistance() - selTrkPt->distance) * 100.0 / track->getTotalDistance(),0,'f',0).arg(QChar(0x21A6)).arg(QChar(0x2691));

        if(selTrkPt->ele != WPT_NOFLOAT)
        {
            if(str.count()) str += "\n";
            IUnit::self().meter2elevation(selTrkPt->ele, val, unit);
            str += tr("elevation: %1 %2").arg(val).arg(unit);
        }



        //-----------------------------------------------------------------------------------------------------------
        //TODO: HOVERTEXT FOR EXTENSIONS
#ifdef GPX_EXTENSIONS
        if (!selTrkPt->gpx_exts.values.empty())
        {
            QList<QString> ext_list = selTrkPt->gpx_exts.values.keys();
            QString ex_name, ex_val;

            for(int i=0; i < selTrkPt->gpx_exts.values.size(); ++i)
            {
                ex_name = ext_list.value(i);
                ex_val = selTrkPt->gpx_exts.getValue(ex_name);

                if (ex_val != "") {str += tr("\n %1: %2 ").arg(ex_name).arg(ex_val);}

            }

        }
#endif
        //-----------------------------------------------------------------------------------------------------------
        if (str != "")
        {
            QFont           f = CResources::self().getMapFont();
            QFontMetrics    fm(f);
            QRect           r1 = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
            r1.moveTopLeft(QPoint(u + 45, v));

            QRect           r2 = r1;
            r2.setWidth(r1.width() + 20);
            r2.moveLeft(r1.left() - 10);
            r2.setHeight(r1.height() + 20);
            r2.moveTop(r1.top() - 10);


            p.setPen(QPen(CCanvas::penBorderBlue));
            p.setBrush(CCanvas::brushBackWhite);
            PAINT_ROUNDED_RECT(p,r2);

            p.setFont(CResources::self().getMapFont());
            p.setPen(Qt::darkBlue);
            p.drawText(r1, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,str);
        }
    }
}

void IMouse::drawSelRtePt(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    if(selRtePt && !selWpt)
    {
        double u = selRtePt->lon * DEG_TO_RAD;
        double v = selRtePt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(CCanvas::penBorderBlue);
        p.setBrush(CCanvas::brushBackWhite);
        p.drawEllipse(QRect(u - 5,  v - 5, 11, 11));

        QString         str = selRtePt->action;
        QFont           f = CResources::self().getMapFont();
        QFontMetrics    fm(f);
        QRect           r1 = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
        r1.moveTopLeft(QPoint(u + 45, v));

        QRect           r2 = r1;
        r2.setWidth(r1.width() + 20);
        r2.moveLeft(r1.left() - 10);
        r2.setHeight(r1.height() + 20);
        r2.moveTop(r1.top() - 10);


        p.setPen(QPen(CCanvas::penBorderBlue));
        p.setBrush(CCanvas::brushBackWhite);
        PAINT_ROUNDED_RECT(p,r2);

        p.setFont(CResources::self().getMapFont());
        p.setPen(Qt::darkBlue);
        p.drawText(r1, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,str);
    }
}



void IMouse::mouseMoveEventWpt(QMouseEvent * e)
{
    QPoint pos      = e->pos();
    IMap& map       = CMapDB::self().getMap();
    CWpt * oldWpt   = selWpt; selWpt = 0;

    // find the waypoint close to the cursor
    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end())
    {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(((pos.x() - u) * (pos.x() - u) + (pos.y() - v) * (pos.y() - v)) < 1225)
        {
            selWpt = *wpt;
            break;
        }

        ++wpt;
    }

    // check for cursor-over-function
    if(selWpt)
    {
        double u = selWpt->lon * DEG_TO_RAD;
        double v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPoint pt = pos - QPoint(u - 24, v - 24);

        if(rectDelWpt.contains(pt) || rectCopyWpt.contains(pt) || rectMoveWpt.contains(pt) || rectEditWpt.contains(pt) || rectViewWpt.contains(pt))
        {
            if(!doSpecialCursorWpt)
            {
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
                doSpecialCursorWpt = true;
            }
        }
        else
        {
            if(doSpecialCursorWpt)
            {
                QApplication::restoreOverrideCursor();
                doSpecialCursorWpt = false;
            }
        }

        if(selWpt->isGeoCache())
        {
            if(!doShowWptBuddies && rectMoveWpt.contains((pt)))
            {
                selWpt->showBuddies(true);
                doShowWptBuddies = true;
                canvas->update();
            }
            else if(!rectMoveWpt.contains(pt))
            {
                selWpt->showBuddies(false);
                doShowWptBuddies = false;
                canvas->update();
            }
        }
    }
    else
    {
        if(doSpecialCursorWpt)
        {
            QApplication::restoreOverrideCursor();
            doSpecialCursorWpt = false;
        }

        if(oldWpt)
        {
            oldWpt->showBuddies(false);
            doShowWptBuddies = false;
        }
    }

    // do a canvas update on a change only
    if(oldWpt != selWpt)
    {
        canvas->update();
    }
}


void IMouse::mouseMoveEventSearch(QMouseEvent * e)
{
    QPoint pos          = e->pos();
    IMap& map           = CMapDB::self().getMap();
    CSearch * oldSearch = selSearch; selSearch = 0;

    // find the search close to the cursor
    QMap<QString,CSearch*>::const_iterator search = CSearchDB::self().begin();
    while(search != CSearchDB::self().end())
    {
        double u = (*search)->lon * DEG_TO_RAD;
        double v = (*search)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(((pos.x() - u) * (pos.x() - u) + (pos.y() - v) * (pos.y() - v)) < 1225)
        {
            selSearch = *search;
            break;
        }

        ++search;
    }

    // check for cursor-over-function
    if(selSearch)
    {
        double u = selSearch->lon * DEG_TO_RAD;
        double v = selSearch->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPoint pt = pos - QPoint(u - 24, v - 24);

        if(rectDelSearch.contains(pt) || rectCopySearch.contains(pt) ||  rectConvertSearch.contains(pt))
        {
            if(!doSpecialCursorSearch)
            {
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
                doSpecialCursorSearch = true;
            }
        }
        else
        {
            if(doSpecialCursorSearch)
            {
                QApplication::restoreOverrideCursor();
                doSpecialCursorSearch = false;
            }
        }
    }
    else
    {
        if(doSpecialCursorSearch)
        {
            QApplication::restoreOverrideCursor();
            doSpecialCursorSearch = false;
        }
    }

    // do a canvas update on a change only
    if(oldSearch != selSearch)
    {
        canvas->update();
    }
}


void IMouse::mousePressEventWpt(QMouseEvent * e)
{
    if(selWpt.isNull()) return;

    IMap& map   = CMapDB::self().getMap();
    QPoint pos  = e->pos();
    double u    = selWpt->lon * DEG_TO_RAD;
    double v    = selWpt->lat * DEG_TO_RAD;
    map.convertRad2Pt(u,v);

    QPoint pt = pos - QPoint(u - 24, v - 24);
    if(rectDelWpt.contains(pt) && !selWpt->sticky)
    {
        CWptDB::self().delWpt(selWpt->getKey(), false, true);
    }
    else if(rectMoveWpt.contains(pt) && selWpt->isMovable())
    {
        canvas->setMouseMode(CCanvas::eMouseMoveWpt);

        QMouseEvent event1(QEvent::MouseMove, QPoint(u,v), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(canvas,&event1);

        QMouseEvent event2(QEvent::MouseButtonPress, QPoint(u,v), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(canvas,&event2);
    }
    else if(rectEditWpt.contains(pt))
    {
        CDlgEditWpt dlg(*selWpt,canvas);
        dlg.exec();
    }
    else if(rectCopyWpt.contains(pt))
    {
        QString position;
        GPS_Math_Deg_To_Str(selWpt->lon, selWpt->lat, position);

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(position);

        selWpt = 0;
        canvas->update();
    }
    else if(rectViewWpt.contains(pt) && !selWpt->images.isEmpty() && !selWpt->images[0].filePath.isEmpty())
    {
        QDesktopServices::openUrl(QUrl("file:///" + selWpt->images[0].filePath));
    }
    else if(rectMarkWpt.contains(pt))
    {
        CWptDB::self().selWptByKey(selWpt->getKey(), true);
        canvas->update();
    }
}


void IMouse::mousePressEventSearch(QMouseEvent * e)
{
    if(selSearch.isNull()) return;

    IMap& map   = CMapDB::self().getMap();
    QPoint pos  = e->pos();
    double u    = selSearch->lon * DEG_TO_RAD;
    double v    = selSearch->lat * DEG_TO_RAD;
    map.convertRad2Pt(u,v);

    QPoint pt = pos - QPoint(u - 24, v - 24);
    if(rectDelSearch.contains(pt))
    {
        QStringList keys;
        keys << selSearch->getKey();
        CSearchDB::self().delResults(keys);
        selSearch = 0;
        canvas->update();
    }
    else if(rectConvertSearch.contains(pt))
    {
        QString key = selSearch->getKey();
        float ele = CMapDB::self().getDEM().getElevation(selSearch->lon * DEG_TO_RAD, selSearch->lat * DEG_TO_RAD);
        CWpt * wpt = CWptDB::self().newWpt(selSearch->lon * DEG_TO_RAD, selSearch->lat * DEG_TO_RAD, ele);
        if(wpt)
        {
            selWpt = wpt;
            QStringList keys;

            keys << key;
            CSearchDB::self().delResults(keys);
            canvas->update();
        }

    }
    else if(rectCopyWpt.contains(pt))
    {
        QString position;
        GPS_Math_Deg_To_Str(selSearch->lon, selSearch->lat, position);

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(position);

        selSearch = 0;
        canvas->update();
    }
}



void IMouse::mouseMoveEventTrack(QMouseEvent * e)
{
    CTrack * track = CTrackDB::self().highlightedTrack();
    if(track == 0) return;

    CTrack::pt_t * oldTrackPt = selTrkPt;
    int d1      = 20;
    selTrkPt    = 0;

    QList<CTrack::pt_t>& pts          = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator pt  = pts.begin();
    while(pt != pts.end())
    {
        if(pt->flags & CTrack::pt_t::eDeleted)
        {
            ++pt; continue;
        }

        int d2 = abs(e->pos().x() - pt->px.x()) + abs(e->pos().y() - pt->px.y());

        if(d2 < d1)
        {
            selTrkPt = &(*pt);
            d1 = d2;
        }

        ++pt;
    }

    if(oldTrackPt != selTrkPt)
    {
        canvas->update();
    }

}

void IMouse::mouseMoveEventRoute(QMouseEvent * e)
{
    CRoute * route = CRouteDB::self().highlightedRoute();
    if(route == 0) return;

    IMap& map = CMapDB::self().getMap();
    double u,v;

    CRoute::pt_t * oldRoutePt = selRtePt;
    int d1      = 20;
    selRtePt    = 0;

    QVector<CRoute::pt_t>& pts          = route->getSecRtePoints().isEmpty() ? route->getPriRtePoints() : route->getSecRtePoints();
    QVector<CRoute::pt_t>::iterator pt  = pts.begin();
    while(pt != pts.end())
    {
        if(pt->action.isEmpty())
        {
            ++pt; continue;
        }

        u = pt->lon * DEG_TO_RAD;
        v = pt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        int d2 = abs(e->pos().x() - u) + abs(e->pos().y() - v);

        if(d2 < d1)
        {
            selRtePt = &(*pt);
            d1 = d2;
        }

        ++pt;
    }

    if(oldRoutePt != selRtePt)
    {
        canvas->update();
    }

}


void IMouse::mouseMoveEventOverlay(QMouseEvent * e)
{
    IOverlay * oldOverlay = selOverlay;

    if(!selOverlay || !selOverlay->mouseActionInProgress())
    {

        QMap<QString, IOverlay*>::const_iterator overlay = COverlayDB::self().begin();
        while(overlay != COverlayDB::self().end())
        {
            if((*overlay)->isCloseEnough(e->pos())) break;
            ++overlay;
        }

        if(overlay != COverlayDB::self().end())
        {
            (*overlay)->select(*overlay);
            selOverlay = *overlay;
        }
        else
        {
            IOverlay::select(0);
            selOverlay = 0;
        }

    }

    if(oldOverlay != selOverlay)
    {
        if(selOverlay && selOverlay->visible())
        {
            canvas->setMouseMode(CCanvas::eMouseOverlay);
        }
        else
        {
            canvas->setMouseMode(CCanvas::eMouseMoveArea);
        }
        canvas->update();
    }
}

void IMouse::mouseMoveEventMapSel(QMouseEvent * e)
{

    IMapSelection * oldSel = selMap;

    IMap& map = CMapDB::self().getMap();
    double u = e->pos().x();
    double v = e->pos().y();

    map.convertPt2Rad(u,v);

    selMap = CMapDB::self().getSelectedMap(u,v);

    if(selMap && !oldSel)
    {
        theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseSelectArea);
    }
    else if(!selMap && oldSel)
    {
        theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseMoveArea);
    }


    if(selMap != oldSel)
    {
        canvas->update();
    }

}

void IMouse::slotSetPos1()
{
    IMap& map = CMapDB::self().getMap();

    double u,v;
    u = mousePos.x();
    v = mousePos.y();
    map.convertPt2Pixel(u,v);
    pos1Pixel = QPointF(u,v);

    u = mousePos.x();
    v = mousePos.y();
    map.convertPt2Rad(u,v);
    pos1LonLat = QPointF(u,v);
}


void IMouse::setSelTrackPt(CTrack::pt_t * pt)
{
    selTrkPt = pt;
    canvas->update();
}
