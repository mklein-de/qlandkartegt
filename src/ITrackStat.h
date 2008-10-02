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
#ifndef ITRACKSTAT_H
#define ITRACKSTAT_H

#include <QWidget>
#include <QPointer>
#include "ui_ITrackStatWidget.h"

#include "CTrack.h"

class CPlot;
class CWpt;

class ITrackStat : public QWidget, private Ui::ITrackStatWidget
{
    Q_OBJECT;
    public:
        ITrackStat(QWidget * parent);
        virtual ~ITrackStat();

    protected:
        struct wpt_t
        {
            wpt_t() : wpt(0), d(1e25f), x(0), y(0) {}
            CWpt * wpt;
            double d;
            double x;
            double y;
            CTrack::pt_t trkpt;
        };

        void addWptTags(QVector<wpt_t>& wpts);

        CPlot * plot;
        QPointer<CTrack> track;
    protected slots:
        void activePointEvent(double x);
};
#endif                           //ITRACKSTAT_H
