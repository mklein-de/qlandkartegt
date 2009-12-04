/**********************************************************************************************
    Copyright (C) 2009 Joerg Wunsch <j@uriah.heep.sax.de>

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

#include "CDlgTrackFilter.h"
#include "CTrack.h"
#include "CTrackDB.h"
#include "GeoMath.h"
#include "IUnit.h"

#include <QtGui>

// Change this when changing comboMeterFeet
enum meter_feet_index {
    METER_INDEX,
    FEET_INDEX,
};


CDlgTrackFilter::CDlgTrackFilter(CTrack &track, QWidget * parent)
: QDialog(parent)
, track(track)
{
    setupUi(this);

    checkReduceDataset->setChecked(false);
    checkModifyTimestamps->setChecked(false);

    QList<CTrack::pt_t>& trkpts = track.getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

    if(IUnit::self().baseunit == "ft") {
        comboMeterFeet->setCurrentIndex((int)FEET_INDEX);
        spinDistance->setSuffix("ft");
    }
    else{
        comboMeterFeet->setCurrentIndex((int)METER_INDEX);
        spinDistance->setSuffix("m");
    }

    if(trkpt->timestamp == 0x000000000 || trkpt->timestamp == 0xFFFFFFFF) {
        // no track time available
        tabTimestamp->setEnabled(false);
        radioTimedelta->setEnabled(false);
        spinTimedelta->setEnabled(false);
        qDebug() << "Track has no timestamps that could be modified.";
    }
    else {
        tabTimestamp->setEnabled(true);
        radioTimedelta->setEnabled(true);
        spinTimedelta->setEnabled(true);

        QDateTime t = QDateTime::fromTime_t(trkpt->timestamp).toLocalTime();
        datetimeStartTime->setDateTime(t);
        radioLocalTime->setChecked(true);
        radioUTC->setChecked(false);
    }

    // user-tunable elements on "Modify Timestamps" tab
    connect(buttonReset1stOfMonth, SIGNAL(clicked()), this, SLOT(slotReset1stOfMonth()));
    connect(buttonResetEpoch, SIGNAL(clicked()), this, SLOT(slotResetEpoch()));
    connect(datetimeStartTime, SIGNAL(dateTimeChanged(const QDateTime &)), this,
            SLOT(slotDateTimeChanged(const QDateTime &)));

    // user-tunable elements on "Reduce Dataset" tab
    connect(radioDistance, SIGNAL(clicked()), this, SLOT(slotRadioDistance()));
    connect(radioTimedelta, SIGNAL(clicked()), this, SLOT(slotRadioTimedelta()));
    connect(spinDistance, SIGNAL(valueChanged(int)), this, SLOT(slotSpinDistance(int)));
    connect(spinTimedelta, SIGNAL(valueChanged(int)), this, SLOT(slotSpinTimedelta(int)));
    connect(comboMeterFeet, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(slotComboMeterFeet(const QString &)));
}


CDlgTrackFilter::~CDlgTrackFilter()
{

}


void CDlgTrackFilter::accept()
{
    qDebug() << "Accepting track filter";
    bool need_rebuild = false;
    QList<CTrack::pt_t>& trkpts = track.getTrackPoints();
    int npts = trkpts.count();

    QProgressDialog progress(tr("Filter track..."), tr("Abort filter"), 0, npts, this);
    progress.setWindowTitle("Filter Progress");
    progress.setWindowModality(Qt::WindowModal);

    if(tabTimestamp->isEnabled() && checkModifyTimestamps->isChecked()) {
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

        if(trkpt->timestamp != 0x000000000 && trkpt->timestamp != 0xFFFFFFFF) {
            qDebug() << "Modifying track timestamps";
            need_rebuild = true;

            QDateTime t;
            if(radioLocalTime->isChecked()) {
                t = QDateTime::fromTime_t(trkpt->timestamp).toLocalTime();
            }
            else {
                t = QDateTime::fromTime_t(trkpt->timestamp).toUTC();
            }
            QDateTime tn = datetimeStartTime->dateTime();
            int offset = (int)tn.toTime_t() - (int)t.toTime_t();
            int i = 0;

            while(trkpt != trkpts.end()) {
                if(radioDelta1s->isChecked()) {
                    trkpt->timestamp = tn.toTime_t() + i;
                }
                else {
                    trkpt->timestamp += offset;
                }
                ++trkpt;
                progress.setValue(i);
                qApp->processEvents(QEventLoop::AllEvents, 100);
                ++i;
                if (progress.wasCanceled())
                    break;
            }
        }
    }

    if(checkReduceDataset->isChecked()) {
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

        if(radioTimedelta->isEnabled() && radioTimedelta->isChecked()) {
            qDebug() << "Reducing track dataset by timedelta";
            need_rebuild = true;

            quint32 timedelta = spinTimedelta->value();
            quint32 lasttime = trkpt->timestamp;
            ++trkpt;

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            int i = 1;
            while(trkpt != trkpts.end()) {
                if(trkpt->timestamp < lasttime + timedelta) {
                    trkpt->flags |= CTrack::pt_t::eDeleted;
                }
                else {
                    lasttime = trkpt->timestamp;
                }
                ++trkpt;
                ++i;
                progress.setValue(i);
                qApp->processEvents(QEventLoop::AllEvents, 100);
                if (progress.wasCanceled())
                    break;
            }

            QApplication::restoreOverrideCursor();
        }
        else if(radioDistance->isChecked()) {
            qDebug() << "Reducing track dataset by distance";
            need_rebuild = true;

            float min_distance = spinDistance->value();
            if(spinDistance->suffix() == "ft") {
                min_distance *= 0.3048;
            }
            XY p1, p2;
            p1.u = DEG_TO_RAD * trkpt->lon;
            p1.v = DEG_TO_RAD * trkpt->lat;
            ++trkpt;

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            int i = 1;

            while(trkpt != trkpts.end()) {
                p2.u = DEG_TO_RAD * trkpt->lon;
                p2.v = DEG_TO_RAD * trkpt->lat;
                double a1, a2;

                double delta = distance(p1,p2,a1,a2);

                if(delta < min_distance) {
                    trkpt->flags |= CTrack::pt_t::eDeleted;
                }
                else {
                    p1 = p2;
                    progress.setValue(i);
                    qApp->processEvents(QEventLoop::AllEvents, 100);
                }
                ++trkpt;
                ++i;
                if (progress.wasCanceled())
                    break;
            }

            QApplication::restoreOverrideCursor();
        }
    }

    progress.setValue(npts);
    if(need_rebuild) {
        track.rebuild(false);
    }

    QDialog::accept();
}


void CDlgTrackFilter::reject()
{

    qDebug() << "Rejecting track filter";

    QDialog::reject();
}


void CDlgTrackFilter::slotReset1stOfMonth()
{
    QDateTime t = datetimeStartTime->dateTime();

    int day = t.date().day();
    int hour = t.time().hour();
    int offset = (day - 1) * 86400 + hour * 3600;
    QDateTime tn = t.addSecs(-offset);

    qDebug() << "Resetting starttime:" << t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'")
             << "to:" << tn.toString("yyyy-MM-dd'T'hh:mm:ss'Z'");

    datetimeStartTime->setDateTime(tn);

    checkModifyTimestamps->setChecked(true);
}

void CDlgTrackFilter::slotResetEpoch()
{
    QDateTime t = datetimeStartTime->dateTime();

    qDebug() << "Resetting starttime:" << t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'")
             << "to epoch";

    QDateTime tn;
    radioUTC->setChecked(true);
    radioLocalTime->setChecked(false);
    tn.setTimeSpec(Qt::UTC);
    tn.setDate(QDate(1970, 1, 1));
    tn.setTime(QTime(0, 0, 0));

    datetimeStartTime->setDateTime(tn);

    checkModifyTimestamps->setChecked(true);
}

void CDlgTrackFilter::slotDateTimeChanged(const QDateTime &tn)
{
    checkModifyTimestamps->setChecked(true);

    qDebug() << "Resetting starttime to:" << tn.toString("yyyy-MM-dd'T'hh:mm:ss'Z'");
}

void CDlgTrackFilter::slotRadioDistance()
{
    checkReduceDataset->setChecked(true);
}

void CDlgTrackFilter::slotSpinDistance(int i)
{
    radioDistance->setChecked(true);
    radioTimedelta->setChecked(false);
    checkReduceDataset->setChecked(true);
}

void CDlgTrackFilter::slotRadioTimedelta()
{
    checkReduceDataset->setChecked(true);
}

void CDlgTrackFilter::slotSpinTimedelta(int i)
{
    radioTimedelta->setChecked(true);
    radioDistance->setChecked(false);
    checkReduceDataset->setChecked(true);
}

void CDlgTrackFilter::slotComboMeterFeet(const QString &text)
{
    spinDistance->setSuffix(text);
}

