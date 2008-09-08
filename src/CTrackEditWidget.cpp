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

#include "CTrackEditWidget.h"
#include "CTrackStatWidget.h"
#include "CTrack.h"
#include "CTrackDB.h"
#include "CResources.h"
#include "GeoMath.h"
#include "CMainWindow.h"
#include "CTabWidget.h"
#include "IUnit.h"

#include <QtGui>

CTrackEditWidget::CTrackEditWidget(QWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);

    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    toolGraph->setIcon(QIcon(":/icons/iconGraph16x16.png"));
    connect(toolGraph, SIGNAL(clicked()), this, SLOT(slotToggleStat()));

    QPixmap icon(16,8);
    for(int i=0; i < 17; ++i) {
        icon.fill(CTrack::colors[i]);
        comboColor->addItem(icon,"",QVariant(i));
    }

    connect(checkRemoveDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckRemove(bool)));
    connect(checkResetDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckReset(bool)));
    connect(buttonBox,SIGNAL(clicked (QAbstractButton*)),this,SLOT(slotApply()));
    connect(treePoints,SIGNAL(itemSelectionChanged()),this,SLOT(slotPointSelectionChanged()));
    connect(treePoints,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotPointSelection(QTreeWidgetItem*)));

}


CTrackEditWidget::~CTrackEditWidget()
{
    if(!trackStat.isNull()){
        delete trackStat;
    }
}


void CTrackEditWidget::keyPressEvent(QKeyEvent * e)
{
    if(track.isNull()) return;

    if(e->key() == Qt::Key_Delete) {
        slotPurge();
    }
    else {
        QWidget::keyPressEvent(e);
    }
}


void CTrackEditWidget::slotSetTrack(CTrack * t)
{
    if(originator) return;

    if(track) {
        disconnect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
        disconnect(track,SIGNAL(destroyed(QObject*)), this, SLOT(close()));
    }

    track = t;
    if(track.isNull()) {
        close();
        return;
    }

    connect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
    connect(track,SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    slotUpdate();
}


void CTrackEditWidget::slotUpdate()
{

    if(originator) return;

    lineName->setText(track->getName());
    comboColor->setCurrentIndex(track->getColorIdx());

    treePoints->clear();


    QString str, val, unit;
    QTreeWidgetItem * focus                 = 0;
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        QTreeWidgetItem * item = new QTreeWidgetItem(treePoints);
        item->setData(0, Qt::UserRole, trkpt->idx);

        // gray shade deleted items
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }

        // temp. store item of user focus
        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focus = item;
        }

        // point number
        item->setText(eNum,QString::number(trkpt->idx));

        // timestamp
        if(trkpt->timestamp != 0x00000000 && trkpt->timestamp != 0xFFFFFFFF) {
            QDateTime time = QDateTime::fromTime_t(trkpt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();
        }
        else {
            str = "-";
        }

        item->setText(eTime,str);

        // altitude
        if(trkpt->ele != WPT_NOFLOAT) {
            IUnit::self().meter2elevation(trkpt->ele, val, unit);
            str = tr("%1 %2").arg(val).arg(unit);
        }
        else {
            str = "-";
        }
        item->setText(eAltitude,str);

        // delta
        IUnit::self().meter2distance(trkpt->delta, val, unit);
        item->setText(eDelta, tr("%1 %2").arg(val).arg(unit));

        // azimuth
        if(trkpt->azimuth != WPT_NOFLOAT) {
            str.sprintf("%1.0f\260",trkpt->azimuth);
        }
        else {
            str = "-";
        }
        item->setText(eAzimuth,str);

        // distance
        IUnit::self().meter2distance(trkpt->distance, val, unit);
        item->setText(eDistance, tr("%1 %2").arg(val).arg(unit));
        IUnit::self().meter2elevation(trkpt->ascend, val, unit);
        item->setText(eAscend, tr("%1 %2").arg(val).arg(unit));
        IUnit::self().meter2elevation(trkpt->descend, val, unit);
        item->setText(eDescend, tr("%1 %2").arg(val).arg(unit));

        // speed
        if(trkpt->speed > 0) {
            IUnit::self().meter2speed(trkpt->speed, val, unit);
            str = tr("%1 %2").arg(val).arg(unit);
        }
        else {
            str = "-";
        }
        item->setText(eSpeed,str);

        GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
        item->setText(ePosition,str);

        item->setTextAlignment(eNum,Qt::AlignLeft);
        item->setTextAlignment(eAltitude,Qt::AlignRight);
        item->setTextAlignment(eDelta,Qt::AlignRight);
        item->setTextAlignment(eAzimuth,Qt::AlignRight);
        item->setTextAlignment(eDistance,Qt::AlignRight);
        item->setTextAlignment(eAscend,Qt::AlignRight);
        item->setTextAlignment(eDescend,Qt::AlignRight);
        item->setTextAlignment(eSpeed,Qt::AlignRight);

        ++trkpt;
    }

    // adjust column sizes to fit
    treePoints->header()->setResizeMode(0,QHeaderView::Interactive);
    for(int i=0; i < eMaxColumn - 1; ++i) {
        treePoints->resizeColumnToContents(i);
    }

    // scroll to item of user focus
    if(focus) {
        treePoints->setCurrentItem(focus);
        treePoints->scrollToItem(focus);
    }

}


void CTrackEditWidget::slotCheckReset(bool checked)
{
    if(checked) {
        checkRemoveDelTrkPt->setChecked(false);
    }
}


void CTrackEditWidget::slotCheckRemove(bool checked)
{
    if(checked) {
        checkResetDelTrkPt->setChecked(false);
    }
}


void CTrackEditWidget::slotApply()
{
    if(track.isNull()) return;

    originator = true;

    if(checkResetDelTrkPt->isChecked()) {
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end()) {
            trkpt->flags &= ~CTrack::pt_t::eDeleted;
            ++trkpt;
        }
        checkResetDelTrkPt->setChecked(false);
        originator = false;
    }

    if(checkRemoveDelTrkPt->isChecked()) {
        QMessageBox::warning(0,tr("Remove track points ...")
            ,tr("You are about to remove purged track points permanently. If you press 'yes', all information will be lost.")
            ,QMessageBox::Yes|QMessageBox::No);
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end()) {

            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                trkpt = trkpts.erase(trkpt);
            }
            else {
                ++trkpt;
            }
        }
        checkRemoveDelTrkPt->setChecked(false);
        originator = false;
    }

    track->setName(lineName->text());
    track->setColor(comboColor->currentIndex());
    track->rebuild(true);
    originator = false;
    emit CTrackDB::self().sigModified();
}


void CTrackEditWidget::slotPointSelectionChanged()
{
    if(track.isNull()) return;

//     qDebug() << "CTrackEditWidget::slotPointSelectionChanged()";

    // reset previous selections
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        trkpt->flags &= ~CTrack::pt_t::eSelected;
        ++trkpt;
    }

    // set eSelected flag for selected points
    QList<QTreeWidgetItem*> items = treePoints->selectedItems();
    QList<QTreeWidgetItem*>::const_iterator item = items.begin();
    while(item != items.end()) {
        quint32 idxTrkPt = (*item)->data(0,Qt::UserRole).toUInt();
        trkpts[idxTrkPt].flags |= CTrack::pt_t::eSelected;
        ++item;
    }
    originator = true;
    track->rebuild(false);
    originator = false;
}


void CTrackEditWidget::slotPointSelection(QTreeWidgetItem * item)
{
    if(track.isNull()) return;
//     qDebug() << "CTrackEditWidget::slotPointSelection()";
    originator = true;
    track->setPointOfFocus(item->data(0,Qt::UserRole).toInt());
    originator = false;
}


void CTrackEditWidget::slotPurge()
{
    QList<CTrack::pt_t>& trkpts                   = track->getTrackPoints();
    QList<QTreeWidgetItem*> items                   = treePoints->selectedItems();
    QList<QTreeWidgetItem*>::const_iterator item    = items.begin();
    while(item != items.end()) {
        quint32 idxTrkPt = (*item)->data(0,Qt::UserRole).toUInt();
        if(trkpts[idxTrkPt].flags & CTrack::pt_t::eDeleted) {
            trkpts[idxTrkPt].flags &= ~CTrack::pt_t::eDeleted;
        }
        else {
            trkpts[idxTrkPt].flags |= CTrack::pt_t::eDeleted;
        }

        ++item;
    }
    track->rebuild(false);
    emit CTrackDB::self().sigModified();
}

void CTrackEditWidget::slotToggleStat()
{
    if(trackStat.isNull()){
        trackStat = new CTrackStatWidget(this);
        theMainWindow->getCanvasTab()->addTab(trackStat, tr("Track"));
    }
    else{
        delete trackStat;
    }
}
