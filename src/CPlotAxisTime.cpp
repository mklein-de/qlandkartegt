/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#include "CPlotAxisTime.h"

#include <math.h>
#include <QtGui>

CPlotAxisTime::CPlotAxisTime(QObject * parent)
: CPlotAxis(parent)
{

}


CPlotAxisTime::~CPlotAxisTime()
{

}


void CPlotAxisTime::calc()
{

    int dSec    = used_max - used_min;
    tic_start   = used_min;

    strFormat = "hh:mm:ss";

    if(dSec < 20)
    {
        interval = 1;
        tic_start = used_min;
    }
    else if(dSec < 100)
    {
        interval = 5;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 200)
    {
        interval = 10;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 600)
    {
        interval = 30;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 1200)
    {
        interval = 60;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 6000)
    {
        interval = 600;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 12000)
    {
        interval = 600;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 36000)
    {
        interval = 1800;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 72000)
    {
        interval = 3600;
        tic_start = ceil(used_min / interval) * interval;
    }
    else if(dSec < 216000)
    {
        interval = 10800;
        tic_start = ceil(used_min / interval) * interval;
    }
    else
    {
        qDebug() << "ouch";
    }

    //         elif dSec < 216000:
    //             interval = 10800
    //             format = "%d %b %H:%M"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 3 h"
    //         elif dSec < 432000:
    //             interval = 21600
    //             format = "%d %b %H:%M"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 6 h"
    //         elif dSec < 864000:
    //             interval = 43200
    //             format = "%d %b %H:%M"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 12 h"
    //         elif dSec < 1728000:
    //             interval = 86400
    //             format = "%d %b %H:%M"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 1 day"
    //         elif dSec < 3456000:
    //             interval = 172800
    //             format = "%d %b"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 2 days"
    //         elif dSec < 12096000:
    //             interval = 604800
    //             format = "%d %b"
    //             tic_start = ceil(float(self.used_min)/interval)*interval + time.timezone
    //             if tic_start < self.used_min:
    //                 tic_start += interval
    //             self.labelInterval = "dt = 1 week"
    //         else:
    //             print "could not scale time axis"
    //
    //         self.tics = []
    //         while ( tic_start - self.used_max ) <= interval / 40:
    //             self.tics.append((tic_start,time.strftime(format,time.localtime(tic_start))))
    //             tic_start += interval
    //
    //         self.int_interval = interval

    if ( autoscale )
    {
        used_min = floor( used_min / interval ) * interval;
        used_max = ceil( used_max / interval ) * interval;
    }
    else
    {
        used_min = used_min;
        used_max = used_max;
    }

    int t1 = ( int )( used_min / interval + 0.5);
    tic_start = interval * t1;
    if ( tic_start < used_min )
    {
        tic_start += interval;
    }

}


const CPlotAxis::TTic* CPlotAxisTime::ticmark( const TTic * t )
{
    const TTic * _tic_ = CPlotAxis::ticmark(t);
    if(_tic_)
    {
        QDateTime time = QDateTime::fromTime_t(tic.val);
        time.setTimeSpec(Qt::LocalTime);
        tic.lbl = time.toString(strFormat);
    }

    return _tic_;
}
