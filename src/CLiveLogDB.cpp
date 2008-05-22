/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#include "CLiveLogDB.h"
#include "CLiveLogToolWidget.h"
#include "CLiveLog.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>

CLiveLogDB * CLiveLogDB::m_self = 0;

CLiveLogDB::CLiveLogDB(QTabWidget * tb, QObject * parent)
    : IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CLiveLogToolWidget(tb);

    connect(&CMapDB::self(), SIGNAL(sigChanged()), this, SLOT(slotMapChanged()));
}

CLiveLogDB::~CLiveLogDB()
{

}

void CLiveLogDB::slotLiveLog(const CLiveLog& log)
{
    m_log = log;


    CLiveLogToolWidget * w = qobject_cast<CLiveLogToolWidget*>(toolview);
    if(w == 0) return;

    float speed_km_h = log.velocity * 3.6;
    float heading = log.heading;
    if( speed_km_h < 0.3 ) {

        // some pretty arbitrary threshold ...
        // with a horizontal error of +/-5m it never goes above
        // 0.06 km/h while standing still, but you don't always
        // have +/-5m...
        speed_km_h = 0.0;
        heading = std::numeric_limits<float>::quiet_NaN();
    }

    m_log.heading = heading;

    QString pos;
    GPS_Math_Deg_To_Str(log.lon, log.lat, pos);


    if(log.fix == CLiveLog::e2DFix || log.fix == CLiveLog::e3DFix){
        w->lblPosition->setText(pos);
        w->lblAltitude->setText(tr("%1 m").arg(log.ele,0,'f',0));
        w->lblErrorHoriz->setText(tr("\261%1 m").arg(log.error_horz/2,0,'f',0));
        w->lblErrorVert->setText(tr("\261%1 m").arg(log.error_vert/2,0,'f',0));
        w->lblSpeed->setText(tr("%1 km/h").arg(speed_km_h, 0, 'f', 1));
        w->lblHeading->setText(tr("%1\260 T").arg(nearbyintf(heading),3,'f',0,'0'));
        w->lblTime->setText(QDateTime::fromTime_t(log.timestamp).toString());

        simplelog_t slog;
        slog.timestamp  = log.timestamp;
        slog.lon        = log.lon;
        slog.lat        = log.lat;
        slog.ele        = log.ele;
        track << slog;

        IMap& map = CMapDB::self().getMap();
        double u = slog.lon * DEG_TO_RAD;
        double v = slog.lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);
        polyline << QPoint(u,v);
    }
    else if(log.fix == CLiveLog::eNoFix){
        w->lblPosition->setText(tr("GPS signal low"));
    }
    else{
        w->lblPosition->setText(tr("GPS off"));
        w->lblAltitude->setText("-");
        w->lblErrorHoriz->setText("-");
        w->lblErrorVert->setText("-");
        w->lblSpeed->setText("-");
        w->lblHeading->setText("-");
        w->lblTime->setText("-");
    }

    emit sigChanged();
}

void CLiveLogDB::slotMapChanged()
{
    IMap& map = CMapDB::self().getMap();
    const quint32 limit = track.size();

    polyline.clear();
    const simplelog_t * plog = track.data();

    for(quint32 i = 0; i < limit; ++i){

        double u = plog->lon * DEG_TO_RAD;
        double v = plog->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);
        polyline << QPoint(u,v);
        ++plog;
    }
}

void CLiveLogDB::draw(QPainter& p)
{
    p.setPen(QPen(Qt::white,5));
    p.drawPolyline(polyline);
    p.setPen(QPen(Qt::black,3));
    p.drawPolyline(polyline);

    IMap& map = CMapDB::self().getMap();
    if(m_log.fix == CLiveLog::e2DFix || m_log.fix == CLiveLog::e3DFix){
        double u = m_log.lon * DEG_TO_RAD;
        double v = m_log.lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);



        float heading = m_log.heading;
        if(!isnan(heading) ) {
            p.save();
            p.translate(u,v);
            p.rotate(heading);
            p.drawPixmap(-23,-30,QPixmap(":/cursors/cursor1"));
            p.restore();
        }
        else{
            p.drawPixmap(u-20 , v-20, QPixmap(":/cursors/cursor2"));
        }

    }
}
