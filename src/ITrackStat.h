/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        ITrackStat.h

  Module:

  Description:

  Created:     09/15/2008

  (C) 2008 DSP Solutions. All rights reserved.


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
        struct wpt_t {
            wpt_t() : wpt(0), d(1e25f), x(0), y(0) {}
            CWpt * wpt;
            double d;
            double x;
            double y;
            CTrack::pt_t trkpt;
        };

        void mousePressEvent(QMouseEvent * e);

        void addWptTags(QVector<wpt_t>& wpts);

        CPlot * plot;
        QPointer<CTrack> track;
};

#endif //ITRACKSTAT_H

