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
#include "CTrackDB.h"
#include "CTrack.h"
#include "CMapDB.h"

#include <QtGui>

CTrackToolWidget::CTrackToolWidget(QToolBox * parent)
    : QWidget(parent)
    , originator(false)
{
    setupUi(this);
    setObjectName("Tracks");
    parent->addItem(this,QIcon(":/icons/iconTrack16x16"),tr("Tracks"));

    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listTracks,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listTracks,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));


}

CTrackToolWidget::~CTrackToolWidget()
{

}

void CTrackToolWidget::slotDBChanged()
{
    if(originator) return;

    QPixmap icon(15,15);
    listTracks->clear();

    QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::const_iterator track = tracks.begin();
    while(track != tracks.end()){
        QListWidgetItem * item = new QListWidgetItem(listTracks);
        icon.fill((*track)->getColor());
        item->setText((*track)->getName());
        item->setData(Qt::UserRole, (*track)->key());
        item->setIcon(icon);
        ++track;
    }
}

void CTrackToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();
    QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();
    if(!tracks.contains(key)){
        return;
    }

    double north =  -90.0;
    double south =  +90.0;
    double west  = +180.0;
    double east  = -180.0;

    CTrack * track = tracks[key];
    QVector<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()){
        if(trkpt->lon < west)  west  = trkpt->lon;
        if(trkpt->lon > east)  east  = trkpt->lon;
        if(trkpt->lat < south) south = trkpt->lat;
        if(trkpt->lat > north) north = trkpt->lat;
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

