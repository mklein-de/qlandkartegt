/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
    connect(pushResetFilterList, SIGNAL(clicked()), this, SLOT(slotResetFilterList()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotHighlightTrack(CTrack*)));

    connect(toolAddHidePoints1, SIGNAL(clicked()), this, SLOT(slotAddFilterHidePoints1()));
    connect(comboMeterFeet1, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotComboMeterFeet1(const QString &)));
    connect(toolAddSmoothProfile1, SIGNAL(clicked()), this, SLOT(slotAddFilterSmoothProfile1()));

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
    listFilters->clear();
    pushResetFilterList->setEnabled(false);
    pushApply->setEnabled(false);
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

    addFilter(groupReducePoints1->title(), ":/icons/iconTrack16x16.png", args);
}

void CTrackFilterWidget::slotAddFilterSmoothProfile1()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    quint32 tabs = spinSmoothProfileTabs1->value();
    stream << quint32(eSmoothProfile1) << tabs;

    addFilter(groupSmoothProfile1->title(), ":/icons/iconTrack16x16.png", args);
}

void CTrackFilterWidget::addFilter(const QString& name, const QString& icon, QByteArray& args)
{
    QListWidgetItem * item = new QListWidgetItem(listFilters);
    item->setIcon(QIcon(icon));
    item->setText(name);
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
            case eSmoothProfile1:
                cancelled = filterSmoothProfile1(args, tracks);
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
    double minDistance, minAzimuthDelta;
    args >> minDistance >> minAzimuthDelta;

    foreach(CTrack * trk, tracks)
    {

        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts = trkpts.count();

        QProgressDialog progress(groupReducePoints1->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupReducePoints1->title());
        progress.setWindowModality(Qt::WindowModal);

        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

        int i               = 1;
        double lastEle      = trkpt->ele;
        double lastAzimuth  = trkpt->azimuth;
        double deltaAzimuth = 0;

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

bool CTrackFilterWidget::filterSmoothProfile1(QDataStream &args, QList<CTrack *> &tracks)
{
    quint32 tabs;
    args >> tabs;

    foreach(CTrack * trk, tracks)
    {

        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts = trkpts.count();

        QProgressDialog progress(groupSmoothProfile1->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupSmoothProfile1->title());
        progress.setWindowModality(Qt::WindowModal);

        trk->medianFilter(tabs, progress);

        if(progress.wasCanceled())
        {
            return true;
        }
    }
    return false;
}
