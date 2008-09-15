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

class CPlot;
class CTrack;


class ITrackStat : public QWidget, private Ui::ITrackStatWidget
{
    Q_OBJECT;
    public:
        ITrackStat(QWidget * parent);
        virtual ~ITrackStat();

    protected:
        CPlot * plot;
        QPointer<CTrack> track;
};

#endif //ITRACKSTAT_H

