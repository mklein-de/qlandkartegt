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

#include "CTrackToolWidget.h"
#include "CTrackEditWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CMapDB.h"
#include "CMainWindow.h"

#include <QtGui>

#define N_LINES 6

CTrackToolWidget::CTrackToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Tracks");
    parent->addTab(this,QIcon(":/icons/iconTrack16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Tracks"));

    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listTracks,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listTracks,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    contextMenu->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);

    connect(listTracks,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    QFontMetrics fm(listTracks->font());
    listTracks->setIconSize(QSize(15,N_LINES*fm.height()));
}


CTrackToolWidget::~CTrackToolWidget()
{

}


void CTrackToolWidget::slotDBChanged()
{
    if(originator) return;

    QFontMetrics fm(listTracks->font());
    QPixmap icon(15,N_LINES*fm.height());
    listTracks->clear();

    QListWidgetItem * highlighted = 0;

    const QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::const_iterator track = tracks.begin();
    while(track != tracks.end()) {
        QListWidgetItem * item = new QListWidgetItem(listTracks);
        icon.fill((*track)->getColor());

        QString str     = (*track)->getName();
        double distance = (*track)->getTotalDistance();
        if(distance > 9999.9) {
            str += tr("\nlength: %1 km").arg(distance / 1000.0, 0, 'f', 3);
        }
        else {
            str += tr("\nlength: %1 m").arg(distance,0 ,'f', 0);
        }
        str += tr(", points: %1").arg((*track)->getTrackPoints().count());


        quint32 ttime = (*track)->getTotalTime();
        quint32 days  = ttime / 86400;

        QTime time;
        time = time.addSecs(ttime);
        if(days){
            str += tr("\ntime: %1:").arg(days) + time.toString("HH:mm:ss");
        }
        else{
            str += tr("\ntime: ") + time.toString("HH:mm:ss");
        }
        str += tr(", speed: %1 km/h").arg(distance * 3.6 / ttime, 0, 'f', 2);
        str += tr("\nstart: %1").arg((*track)->getStartTimestamp().isNull() ? tr("-") : (*track)->getStartTimestamp().toString());
        str += tr("\nend: %1").arg((*track)->getEndTimestamp().isNull() ? tr("-") : (*track)->getEndTimestamp().toString());
        str += tr("\n%1%2  m, %3%4 m").arg(QChar(0x2191)).arg((*track)->getAscend(),0,'f',0).arg(QChar(0x2193)).arg((*track)->getDescend(),0,'f',0);
        item->setText(str);
        item->setData(Qt::UserRole, (*track)->key());
        item->setIcon(icon);

        if((*track)->isHighlighted()) {
            highlighted = item;
        }

        ++track;
    }

    if(highlighted) {
        listTracks->setCurrentItem(highlighted);
    }
}


void CTrackToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();
    const QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();
    if(!tracks.contains(key)) {
        return;
    }

    double north =  -90.0;
    double south =  +90.0;
    double west  = +180.0;
    double east  = -180.0;

    CTrack * track = tracks[key];
    QVector<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(!(trkpt->flags & CTrack::pt_t::eDeleted)) {
            if(trkpt->lon < west)  west  = trkpt->lon;
            if(trkpt->lon > east)  east  = trkpt->lon;
            if(trkpt->lat < south) south = trkpt->lat;
            if(trkpt->lat > north) north = trkpt->lat;
        }
        ++trkpt;
    }

    CMapDB::self().getMap().zoom(west * DEG_TO_RAD, north * DEG_TO_RAD, east * DEG_TO_RAD, south * DEG_TO_RAD);
}


void CTrackToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    CTrackDB::self().highlightTrack(item->data(Qt::UserRole).toString());
    originator = false;
}


void CTrackToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete) {
        slotDelete();
        e->accept();
    }
    else {
        QWidget::keyPressEvent(e);
    }
}


void CTrackToolWidget::slotContextMenu(const QPoint& pos)
{
    if(listTracks->currentItem()) {
        QPoint p = listTracks->mapToGlobal(pos);
        contextMenu->exec(p);
    }

}


void CTrackToolWidget::slotEdit()
{
    const QListWidgetItem* item = listTracks->currentItem();

    if(item == 0) {
        QMessageBox::information(0,tr("Edit track ..."), tr("You have to select a track first."),QMessageBox::Ok,QMessageBox::Ok);
        return;
    };

    if(trackedit.isNull()) {
        trackedit = new CTrackEditWidget(theMainWindow->getCanvas());
        connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), trackedit, SLOT(slotSetTrack(CTrack*)));
        theMainWindow->setTempWidget(trackedit);
        trackedit->slotSetTrack(CTrackDB::self().highlightedTrack());
    }
    else{
        delete trackedit;
    }
}


void CTrackToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CTrackDB::self().delTracks(keys);
}
