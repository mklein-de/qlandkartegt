/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTrackFilterWidget.cpp

  Module:      

  Description:

  Created:     06/22/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CTrackFilterWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CTrackEditWidget.h"
#include "GeoMath.h"
#include "CSettings.h"
#include "IUnit.h"

#include <QtGui>

enum meter_feet_index
{
    METER_INDEX,
    FEET_INDEX,
};


CTrackFilterWidget::CTrackFilterWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("CTrackFilterWidget");
    connect(pushApply, SIGNAL(clicked()), this, SLOT(slotApplyFilter()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotHighlightTrack(CTrack*)));

    connect(toolAddHidePoints1, SIGNAL(clicked()), this, SLOT(slotAddFilterHidePoints1()));
    connect(comboMeterFeet1, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotComboMeterFeet1(const QString &)));

    // ----------- read in GUI configuration -----------
    SETTINGS;
    // Filter: Hide points 1
    if(IUnit::self().baseunit == "ft")
    {
        comboMeterFeet1->setCurrentIndex((int)FEET_INDEX);
        spinDistance1->setSuffix("ft");
    }
    else
    {
        comboMeterFeet1->setCurrentIndex((int)METER_INDEX);
        spinDistance1->setSuffix("m");
    }
    spinDistance1->setValue(cfg.value("trackfilter/HidePoints1/distance",10).toInt());
    spinAzimuthDelta1->setValue(cfg.value("trackfilter/HidePoints1/azimuthdelta",10).toInt());


    slotHighlightTrack(CTrackDB::self().highlightedTrack());
}

CTrackFilterWidget::~CTrackFilterWidget()
{
    // ----------- store GUI configuration -----------
    SETTINGS;
    // Filter: Hide points 1
    cfg.setValue("trackfilter/HidePoints1/distance",spinDistance1->value());
    cfg.setValue("trackfilter/HidePoints1/azimuthdelta",spinAzimuthDelta1->value());
}


void CTrackFilterWidget::setTrackEditWidget(CTrackEditWidget * w)
{
    trackEditWidget = w;
//    connect(pushReset, SIGNAL(clicked()), trackEditWidget, SLOT(slotReset()));
//    connect(pushDelete, SIGNAL(clicked()), trackEditWidget, SLOT(slotDelete()));
}

void CTrackFilterWidget::slotHighlightTrack(CTrack * trk)
{
    track = trk;
    if(!track.isNull())
    {
        // todo add track dependend setup
    }
}

void CTrackFilterWidget::slotComboMeterFeet1(const QString &text)
{
    spinDistance1->setSuffix(text);
}


void CTrackFilterWidget::slotResetFilterList()
{

}

void CTrackFilterWidget::slotAddFilterHidePoints1()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    double d =  spinDistance1->value();
    if(spinDistance1->suffix() == "ft")
    {
        d *= 0.3048f;
    }

    double a = spinAzimuthDelta1->value();
    stream << quint32(eHidePoints1) << d << a;

    QListWidgetItem * item = new QListWidgetItem(listFilters);
    item->setIcon(QIcon(":/icons/iconTrack16x16.png"));
    item->setText(tr("Reduce points 1"));
    item->setData(Qt::UserRole, args);

    pushApply->setEnabled(true);
    pushResetFilterList->setEnabled(true);
}

void CTrackFilterWidget::slotApplyFilter()
{
    if(track.isNull()) return;

    QList<CTrack*> tracks;
    tracks << track;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    const int N = listFilters->count();
    for(int i = 0; i < N; i++)
    {
        bool cancelled = true;
        quint32 type;
        QListWidgetItem * item = listFilters->item(i);
        QByteArray data = item->data(Qt::UserRole).toByteArray();
        QDataStream args(&data, QIODevice::ReadOnly);

        args >> type;
        switch(type)
        {
            case eHidePoints1:
                cancelled = filterHidePoints1(args, tracks);
                break;
            default:
                qDebug() << "unknown filter" << type;
                cancelled = true;

        }

        if(cancelled)
        {
            break;
        }
    }

    track->rebuild(false);
    CTrackDB::self().emitSigModified();
    QApplication::restoreOverrideCursor();
}


bool CTrackFilterWidget::filterHidePoints1(QDataStream& args, QList<CTrack*>& tracks)
{
    foreach(CTrack * trk, tracks)
    {

        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts = trkpts.count();

        QProgressDialog progress(tr("Filter track..."), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle("Filter Progress");
        progress.setWindowModality(Qt::WindowModal);

        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

        int i               = 1;
        double lastEle      = trkpt->ele;
        double lastAzimuth  = trkpt->azimuth;
        double deltaAzimuth = 0;
        double minDistance, minAzimuthDelta;
        args >> minDistance >> minAzimuthDelta;

        projXY p1, p2;
        p1.u = DEG_TO_RAD * trkpt->lon;
        p1.v = DEG_TO_RAD * trkpt->lat;
        ++trkpt;

        while(trkpt != trkpts.end())
        {
            p2.u = DEG_TO_RAD * trkpt->lon;
            p2.v = DEG_TO_RAD * trkpt->lat;
            double a1, a2;

            double delta = distance(p1,p2,a1,a2);

            if (abs(trkpt->azimuth) <= 180)
            {
                if(abs(trkpt->azimuth - lastAzimuth) > 180)
                {
                    deltaAzimuth = 360 - abs(trkpt->azimuth - lastAzimuth);
                }
                else
                {
                    deltaAzimuth = abs(trkpt->azimuth - lastAzimuth);
                }
            }
            else
            {
                deltaAzimuth = 0;
            }


            double deltaEle = abs(lastEle - trkpt->ele);

            if (delta < minDistance || (deltaAzimuth < minAzimuthDelta))
            {
                if(deltaEle < 3)
                {
                    trkpt->flags |= CTrack::pt_t::eDeleted;
                }

            }
            else
            {
                p1 = p2;
                progress.setValue(i);
                qApp->processEvents();
                if(deltaAzimuth >= minAzimuthDelta)
                {
                    lastAzimuth = trkpt->azimuth;
                }

                lastEle = trkpt->ele;
            }
            ++trkpt;
            ++i;
            if (progress.wasCanceled())
            {
                return true;
            }
        }
    }

    return false;
}
