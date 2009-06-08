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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CTrackEditWidget.h"
#include "CTrackStatProfileWidget.h"
#include "CTrackStatSpeedWidget.h"
#include "CTrackStatTraineeWidget.h"
#include "CTrack.h"
#include "CTrackDB.h"
#include "CResources.h"
#include "GeoMath.h"
#include "CMainWindow.h"
#include "CTabWidget.h"
#include "IUnit.h"

#include <QtGui>

bool CTrackTreeWidgetItem::operator< ( const QTreeWidgetItem & other ) const
{
    const QString speed("/h");
    const QRegExp distance("(ft|ml|m|km)");
    double d1 = 0, d2 = 0;

    int sortCol = treeWidget()->sortColumn();
    QString str1 = text(sortCol);
    QString str2 = other.text(sortCol);

    if (str1.contains(speed) && str2.contains(speed)) {
        d1 = IUnit::self().str2speed(str1);
        d2 = IUnit::self().str2speed(str2);
    }
    else if (str1.contains(distance) && str2.contains(distance)) {
        d1 = IUnit::self().str2distance(str1);
        d2 = IUnit::self().str2distance(str2);
    }
    else {
        /* let's assume it's a double without any unit ... */
        d1 = str1.toDouble();
        d2 = str2.toDouble();
    }

    return d1 < d2;
}


CTrackEditWidget::CTrackEditWidget(QWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);

    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    toolGraphDistance->setIcon(QIcon(":/icons/iconGraph16x16.png"));
    connect(toolGraphDistance, SIGNAL(clicked()), this, SLOT(slotToggleStatDistance()));

    toolGraphTime->setIcon(QIcon(":/icons/iconTime16x16.png"));
    connect(toolGraphTime, SIGNAL(clicked()), this, SLOT(slotToggleStatTime()));

    traineeGraph->setIcon(QIcon(":/icons/package_favorite.png"));
    connect(traineeGraph, SIGNAL(clicked()), this, SLOT(slotToggleTrainee()));

    QPixmap icon(16,8);
    for(int i=0; i < 17; ++i) {
        icon.fill(CTrack::lineColors[i]);
        comboColor->addItem(icon,"",QVariant(i));
    }

    connect(checkRemoveDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckRemove(bool)));
    connect(checkResetDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckReset(bool)));
    connect(buttonBox,SIGNAL(clicked (QAbstractButton*)),this,SLOT(slotApply()));
    connect(treePoints,SIGNAL(itemSelectionChanged()),this,SLOT(slotPointSelectionChanged()));
    connect(treePoints,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotPointSelection(QTreeWidgetItem*)));

    treePoints->sortByColumn(eNum, Qt::AscendingOrder);
}


CTrackEditWidget::~CTrackEditWidget()
{
    if(!trackStatProfileDist.isNull()) {
        delete trackStatProfileDist;
    }
    if(!trackStatSpeedDist.isNull()) {
        delete trackStatSpeedDist;
    }
    if(!trackStatProfileTime.isNull()) {
        delete trackStatProfileTime;
    }
    if(!trackStatSpeedTime.isNull()) {
        delete trackStatSpeedTime;
    }
    if(!trackStatTrainee.isNull()) {
        delete trackStatTrainee;
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

        // clean view
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end()) {
            trkpt->editItem = 0;
            ++trkpt;
        }
        treePoints->clear();     // this also delete the items
    }

    track = t;
    if(track.isNull()) {
        close();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        trkpt->editItem = 0;
        ++trkpt;
    }

    connect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
    connect(track,SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    slotUpdate();

    treePoints->setUpdatesEnabled(false);
    for(int i=0; i < eMaxColumn - 1; ++i) {
        treePoints->resizeColumnToContents(i);
    }
    treePoints->setUpdatesEnabled(true);

    QApplication::restoreOverrideCursor();
}


void CTrackEditWidget::slotUpdate()
{
    if (track->hasTraineeData())
        traineeGraph->setEnabled(true);
    else {
        traineeGraph->setEnabled(false);
        if (!trackStatTrainee.isNull())
            delete trackStatTrainee;
    }

    if(originator) return;

    lineName->setText(track->getName());
    comboColor->setCurrentIndex(track->getColorIdx());

    treePoints->setUpdatesEnabled(false);
    treePoints->setSelectionMode(QAbstractItemView::MultiSelection);

    QString str, val, unit;
    CTrackTreeWidgetItem * focus                 = 0;
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        CTrackTreeWidgetItem * item;
        if ( !trkpt->editItem ) {
            trkpt->editItem = (QObject*)new CTrackTreeWidgetItem(treePoints);
            item = (CTrackTreeWidgetItem *)trkpt->editItem;
            item->setTextAlignment(eNum,Qt::AlignLeft);
            item->setTextAlignment(eAltitude,Qt::AlignRight);
            item->setTextAlignment(eDelta,Qt::AlignRight);
            item->setTextAlignment(eAzimuth,Qt::AlignRight);
            item->setTextAlignment(eDistance,Qt::AlignRight);
            item->setTextAlignment(eAscend,Qt::AlignRight);
            item->setTextAlignment(eDescend,Qt::AlignRight);
            item->setTextAlignment(eSpeed,Qt::AlignRight);
            trkpt->flags.setChanged(true);
        }

        if ( !trkpt->flags.isChanged() ) {
            ++trkpt;
            continue;
        }

        item = (CTrackTreeWidgetItem *)trkpt->editItem;
        item->setData(0, Qt::UserRole, trkpt->idx);

        // gray shade deleted items
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        else {
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        }

        // temp. store item of user focus
        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focus = item;
        }

        if(trkpt->flags & CTrack::pt_t::eSelected) {
            if ( !item->isSelected() )
                item->setSelected(true);
        }
        else {
            if ( item->isSelected() )
                item->setSelected(false);
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

        trkpt->flags.setChanged(false);

        ++trkpt;
    }

    // adjust column sizes to fit
    treePoints->header()->setResizeMode(0,QHeaderView::Interactive);

    // scroll to item of user focus
    if(focus) {
        //         treePoints->setCurrentItem(focus);
        treePoints->scrollToItem(focus);
    }
    treePoints->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treePoints->setUpdatesEnabled(true);
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
                if ( trkpt->editItem ) {
                    int idx = treePoints->indexOfTopLevelItem((CTrackTreeWidgetItem *)trkpt->editItem);
                    if ( idx != -1 )
                        treePoints->takeTopLevelItem(idx);
                    delete (CTrackTreeWidgetItem *)trkpt->editItem;
                    trkpt->editItem = 0;
                }
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

    if(treePoints->selectionMode() == QAbstractItemView::MultiSelection) return;

    //    qDebug() << Q_FUNC_INFO;

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


void CTrackEditWidget::slotToggleStatDistance()
{
    if(trackStatSpeedDist.isNull()) {
        trackStatSpeedDist = new CTrackStatSpeedWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->addTab(trackStatSpeedDist, tr("Speed/Dist."));
    }
    else {
        delete trackStatSpeedDist;
    }

    if(trackStatProfileDist.isNull()) {
        trackStatProfileDist = new CTrackStatProfileWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->addTab(trackStatProfileDist, tr("Profile/Dist."));
    }
    else {
        delete trackStatProfileDist;
    }
}


void CTrackEditWidget::slotToggleStatTime()
{
    if(trackStatSpeedTime.isNull()) {
        trackStatSpeedTime = new CTrackStatSpeedWidget(ITrackStat::eOverTime, this);
        theMainWindow->getCanvasTab()->addTab(trackStatSpeedTime, tr("Speed/Time"));
    }
    else {
        delete trackStatSpeedTime;
    }

    if(trackStatProfileTime.isNull()) {
        trackStatProfileTime = new CTrackStatProfileWidget(ITrackStat::eOverTime, this);
        theMainWindow->getCanvasTab()->addTab(trackStatProfileTime, tr("Profile/Time"));
    }
    else {
        delete trackStatProfileTime;
    }
}


void CTrackEditWidget::slotToggleTrainee()
{
    if(trackStatTrainee.isNull()) {
        trackStatTrainee = new CTrackStatTraineeWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->addTab(trackStatTrainee, tr("Trainee"));
    }
    else {
        delete trackStatTrainee;
    }
}
