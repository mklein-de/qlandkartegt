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
#include "CWptDB.h"

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

    connect(comboMeterFeet1, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotComboMeterFeet(const QString &)));
    connect(comboMeterFeet2, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotComboMeterFeet(const QString &)));
    connect(comboMeterFeet3, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotComboMeterFeet(const QString &)));

    connect(toolAddHidePoints1, SIGNAL(clicked()), this, SLOT(slotAddFilterHidePoints1()));
    connect(toolAddSmoothProfile1, SIGNAL(clicked()), this, SLOT(slotAddFilterSmoothProfile1()));
    connect(toolAddSplit1, SIGNAL(clicked()), this, SLOT(slotAddFilterSplit1()));
    connect(toolAddSplit2, SIGNAL(clicked()), this, SLOT(slotAddFilterSplit2()));
    connect(toolAddSplit3, SIGNAL(clicked()), this, SLOT(slotAddFilterSplit3()));
    connect(toolAddSplit4, SIGNAL(clicked()), this, SLOT(slotAddFilterSplit4()));

    // ----------- read in GUI configuration -----------
    SETTINGS;

    if(IUnit::self().baseunit == "ft")
    {
        comboMeterFeet1->setCurrentIndex((int)FEET_INDEX);
        comboMeterFeet2->setCurrentIndex((int)FEET_INDEX);
        comboMeterFeet3->setCurrentIndex((int)FEET_INDEX);
        spinDistance1->setSuffix("ft");
        spinSplit3->setSuffix("ft");
        spinSplit4->setSuffix("ft");
    }
    else
    {
        comboMeterFeet1->setCurrentIndex((int)METER_INDEX);
        comboMeterFeet2->setCurrentIndex((int)METER_INDEX);
        comboMeterFeet3->setCurrentIndex((int)METER_INDEX);
        spinDistance1->setSuffix("m");
        spinSplit3->setSuffix("m");
        spinSplit4->setSuffix("m");
    }

    // Filter: Hide points 1
    spinDistance1->setValue(cfg.value("trackfilter/HidePoints1/distance",10).toInt());
    spinAzimuthDelta1->setValue(cfg.value("trackfilter/HidePoints1/azimuthdelta",10).toInt());

    // Filter: Smooth profile 1
    spinSmoothProfileTabs1->setValue(cfg.value("trackfilter/SmoothProfile1/tabs",5).toInt());

    // Filter: Split 1
    spinSplit1->setValue(cfg.value("trackfilter/Split1/val",spinSplit1->value()).toInt());

    // Filter: Split 2
    spinSplit2->setValue(cfg.value("trackfilter/Split2/val",spinSplit2->value()).toInt());

    // Filter Split 3
    spinSplit3->setValue(cfg.value("trackfilter/Split3/val",spinSplit3->value()).toInt());

    // Filter: Split 4
    spinSplit4->setValue(cfg.value("trackfilter/Split4/val",spinSplit4->value()).toInt());

    // register current track
    slotHighlightTrack(CTrackDB::self().highlightedTrack());


    radioSplitTracks->setChecked(cfg.value("trackfilter/Split/asTrack", radioSplitTracks->isChecked()).toBool());
    radioSplitStages->setChecked(cfg.value("trackfilter/Split/asStages", radioSplitStages->isChecked()).toBool());
}

CTrackFilterWidget::~CTrackFilterWidget()
{
    // ----------- store GUI configuration -----------
    SETTINGS;
    // Filter: Hide points 1
    cfg.setValue("trackfilter/HidePoints1/distance",spinDistance1->value());
    cfg.setValue("trackfilter/HidePoints1/azimuthdelta",spinAzimuthDelta1->value());

    // Filter: Smooth profile 1
    cfg.setValue("trackfilter/SmoothProfile1/tabs",spinSmoothProfileTabs1->value());

    // Filter: Split 1
    cfg.setValue("trackfilter/Split1/val",spinSplit1->value());

    // Filter: Split 2
    cfg.setValue("trackfilter/Split2/val",spinSplit2->value());

    // Filter: Split 3
    cfg.setValue("trackfilter/Split3/val",spinSplit3->value());

    // Filter: Split 4
    cfg.setValue("trackfilter/Split4/val",spinSplit4->value());


    cfg.setValue("trackfilter/Split/asTrack", radioSplitTracks->isChecked());
    cfg.setValue("trackfilter/Split/asStages", radioSplitStages->isChecked());
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

void CTrackFilterWidget::slotComboMeterFeet(const QString &text)
{
    if(sender() == comboMeterFeet1)
    {
        spinDistance1->setSuffix(text);
    }
    else if(sender() == comboMeterFeet2)
    {
        spinSplit3->setSuffix(text);
    }
    else if(sender() == comboMeterFeet3)
    {
        spinSplit4->setSuffix(text);
    }
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

    QString name = groupReducePoints1->title() + QString(" (%1%2, %3\260)").arg(d).arg(spinDistance1->suffix()).arg(a);

    addFilter(name, ":/icons/iconTrack16x16.png", args);
}

void CTrackFilterWidget::slotAddFilterSmoothProfile1()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    quint32 tabs = spinSmoothProfileTabs1->value();
    stream << quint32(eSmoothProfile1) << tabs;

    QString name = groupSmoothProfile1->title() + QString(" (%1 points)").arg(tabs);
    addFilter(name, ":/icons/iconTrack16x16.png", args);
}

void CTrackFilterWidget::slotAddFilterSplit1()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    double val = spinSplit1->value();
    stream << quint32(eSplit1) << val;

    QString name = groupSplit1->title() + QString(" (%1 chunks)").arg(val);
    addFilter(name, ":/icons/editcut.png", args);
}

void CTrackFilterWidget::slotAddFilterSplit2()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    double val = spinSplit2->value();
    stream << quint32(eSplit2) << val;

    QString name = groupSplit2->title() + QString(" (%1 points)").arg(val);
    addFilter(name, ":/icons/editcut.png", args);
}

void CTrackFilterWidget::slotAddFilterSplit3()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    double val = spinSplit3->value();
    stream << quint32(eSplit3) << val;

    QString name = groupSplit3->title() + QString(" (%1%2)").arg(val).arg(spinSplit3->suffix());
    addFilter(name, ":/icons/editcut.png", args);
}

void CTrackFilterWidget::slotAddFilterSplit4()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);

    double val = spinSplit4->value();
    stream << quint32(eSplit4) << val;

    QString name = groupSplit4->title() + QString(" (%1%2)").arg(val).arg(spinSplit4->suffix());
    addFilter(name, ":/icons/editcut.png", args);
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

            case eSplit1:
                if(radioSplitTracks->isChecked())
                {
                    cancelled = filterSplit1Tracks(args, tracks);
                }
                else
                {
                    cancelled = filterSplit1Stages(args, tracks);
                }
                break;

            case eSplit2:
                if(radioSplitTracks->isChecked())
                {
                    cancelled = filterSplit2Tracks(args, tracks);
                }
                else
                {
                    cancelled = filterSplit2Stages(args, tracks);
                }
                break;

            case eSplit3:
                if(radioSplitTracks->isChecked())
                {
                    cancelled = filterSplit3Tracks(args, tracks);
                }
                else
                {
                    cancelled = filterSplit3Stages(args, tracks);
                }
                break;

            case eSplit4:
                if(radioSplitTracks->isChecked())
                {
                    cancelled = filterSplit4Tracks(args, tracks);
                }
                else
                {
                    cancelled = filterSplit4Stages(args, tracks);
                }
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
        trk->rebuild(false);
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

        trk->rebuild(false);
    }
    return false;
}

bool CTrackFilterWidget::filterSplit1Stages(QDataStream &args, QList<CTrack *> &tracks)
{
    double val;
    args >> val;

    foreach(CTrack * trk, tracks)
    {
        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts        = trkpts.count();
        int totalCnt    = 0;
        int trkptCnt    = 0;
        int trkCnt      = 1;
        int chunk       = (trkpts.size() + val) / val;

        QProgressDialog progress(groupSplit1->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupSplit1->title());
        progress.setWindowModality(Qt::WindowModal);

        foreach(const CTrack::pt_t& trkpt, trkpts)
        {
            progress.setValue(totalCnt++);
            qApp->processEvents();

            if(++trkptCnt >= chunk)
            {
                trkptCnt = 0;
                CWptDB::self().newWpt(trkpt.lon * DEG_TO_RAD, trkpt.lat * DEG_TO_RAD, trkpt.ele, QString("S%1").arg(trkCnt++));
            }

            if(progress.wasCanceled())
            {
                return true;
            }
        }
    }
    return false;

}

bool CTrackFilterWidget::filterSplit1Tracks(QDataStream &args, QList<CTrack *> &tracks)
{
    double val;
    args >> val;

    QList<CTrack *> newTracks;

    foreach(CTrack * trk, tracks)
    {
        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts        = trkpts.count();
        int totalCnt    = 0;
        int trkptCnt    = 0;
        int trkCnt      = 1;
        int chunk       = (trkpts.size() + val) / val;

        QProgressDialog progress(groupSplit1->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupSplit1->title());
        progress.setWindowModality(Qt::WindowModal);

        CTrack * newTrack = new CTrack(&CTrackDB::self());
        newTrack->setName(trk->getName() + QString("_%1").arg(trkCnt++));
        newTrack->setColor(trk->getColorIdx());
        newTracks << newTrack;

        foreach(const CTrack::pt_t& trkpt, trkpts)
        {
            progress.setValue(totalCnt++);
            qApp->processEvents();

            *newTrack << trkpt;
            if(++trkptCnt >= chunk)
            {
                CTrackDB::self().addTrack(newTrack, true);

                trkptCnt = 0;
                newTrack = new CTrack(&CTrackDB::self());
                newTrack->setName(trk->getName() + QString("_%1").arg(trkCnt++));
                newTrack->setColor(trk->getColorIdx());

                newTracks << newTrack;
            }

            if(progress.wasCanceled())
            {
                return true;
            }
        }
        CTrackDB::self().addTrack(newTrack, true);
    }
    tracks = newTracks;
    return false;
}

bool CTrackFilterWidget::filterSplit2Stages(QDataStream &args, QList<CTrack *> &tracks)
{
    double val;
    args >> val;

    foreach(CTrack * trk, tracks)
    {
        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts        = trkpts.count();
        int totalCnt    = 0;
        int trkptCnt    = 0;
        int trkCnt      = 1;
        int chunk       = val;

        QProgressDialog progress(groupSplit1->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupSplit1->title());
        progress.setWindowModality(Qt::WindowModal);

        foreach(const CTrack::pt_t& trkpt, trkpts)
        {
            progress.setValue(totalCnt++);
            qApp->processEvents();

            if(++trkptCnt >= chunk)
            {
                trkptCnt = 0;
                CWptDB::self().newWpt(trkpt.lon * DEG_TO_RAD, trkpt.lat * DEG_TO_RAD, trkpt.ele, QString("S%1").arg(trkCnt++));
            }

            if(progress.wasCanceled())
            {
                return true;
            }
        }
    }
    return false;
}

bool CTrackFilterWidget::filterSplit2Tracks(QDataStream &args, QList<CTrack *> &tracks)
{
    double val;
    args >> val;

    QList<CTrack *> newTracks;

    foreach(CTrack * trk, tracks)
    {
        QList<CTrack::pt_t>& trkpts = trk->getTrackPoints();
        int npts        = trkpts.count();
        int totalCnt    = 0;
        int trkptCnt    = 0;
        int trkCnt      = 1;
        int chunk       = val;

        QProgressDialog progress(groupSplit2->title(), tr("Abort filter"), 0, npts, this);
        progress.setWindowTitle(groupSplit2->title());
        progress.setWindowModality(Qt::WindowModal);

        CTrack * newTrack = new CTrack(&CTrackDB::self());
        newTrack->setName(trk->getName() + QString("_%1").arg(trkCnt++));
        newTrack->setColor(trk->getColorIdx());
        newTracks << newTrack;

        foreach(const CTrack::pt_t& trkpt, trkpts)
        {
            progress.setValue(totalCnt++);
            qApp->processEvents();

            *newTrack << trkpt;
            if(++trkptCnt >= chunk)
            {
                CTrackDB::self().addTrack(newTrack, true);

                trkptCnt = 0;
                newTrack = new CTrack(&CTrackDB::self());
                newTrack->setName(trk->getName() + QString("_%1").arg(trkCnt++));
                newTrack->setColor(trk->getColorIdx());

                newTracks << newTrack;
            }

            if(progress.wasCanceled())
            {
                return true;
            }
        }
        CTrackDB::self().addTrack(newTrack, true);
    }


    tracks = newTracks;
    return false;
}

bool CTrackFilterWidget::filterSplit3Stages(QDataStream &args, QList<CTrack *> &tracks)
{
    return false;
}

bool CTrackFilterWidget::filterSplit3Tracks(QDataStream &args, QList<CTrack *> &tracks)
{
    return false;
}

bool CTrackFilterWidget::filterSplit4Stages(QDataStream &args, QList<CTrack *> &tracks)
{
    return false;
}

bool CTrackFilterWidget::filterSplit4Tracks(QDataStream &args, QList<CTrack *> &tracks)
{
    return false;
}
