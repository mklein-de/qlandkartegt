/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTrackFilterWidget.h

  Module:      

  Description:

  Created:     06/22/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CTRACKFILTERWIDGET_H
#define CTRACKFILTERWIDGET_H

#include <QWidget>
#include <QPointer>

#include "ui_ITrackFilterWidget.h"

class CTrackEditWidget;
class CTrack;

class CTrackFilterWidget : public QWidget, private Ui::ITrackFilterWidget
{
    Q_OBJECT;
    public:
        CTrackFilterWidget(QWidget * parent);
        virtual ~CTrackFilterWidget();

        void setTrackEditWidget(CTrackEditWidget * w);

    private slots:
        void slotApplyFilter();
        void slotHighlightTrack(CTrack * trk);
        void slotComboMeterFeet1(const QString &text);
        void slotResetFilterList();
        void slotAddFilterHidePoints1();

    private:
        bool filterHidePoints1(QDataStream &args, QList<CTrack *> &tracks);

        enum filterType_e {eHidePoints1};

        QPointer<CTrackEditWidget> trackEditWidget;
        QPointer<CTrack> track;
};

#endif //CTRACKFILTERWIDGET_H

