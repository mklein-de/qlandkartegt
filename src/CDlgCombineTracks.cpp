/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License; or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful;
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not; write to the Free Software
    Foundation; Inc.; 59 Temple Place - Suite 330; Boston; MA 02111 USA

**********************************************************************************************/

#include "CDlgCombineTracks.h"
#include "CTrackDB.h"
#include "CTrack.h"

#include <QtGui>

CDlgCombineTracks::CDlgCombineTracks(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    toolAdd->setIcon(QPixmap(":/icons/iconRight16x16.png"));
    connect(toolAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));
    toolDel->setIcon(QPixmap(":/icons/iconLeft16x16.png"));
    connect(toolDel, SIGNAL(clicked()), this, SLOT(slotDel()));

    CTrack * track;
    QList<CTrack*> tracks =  CTrackDB::self().getTracks().values();

    foreach(track, tracks){
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(track->getName());
        item->setData(Qt::UserRole, track->key());
        listTracks->addItem(item);
    }

}

CDlgCombineTracks::~CDlgCombineTracks()
{

}

void CDlgCombineTracks::slotAdd()
{
    QListWidgetItem * item;
    QList<QListWidgetItem*> items = listTracks->selectedItems();

    foreach(item, items){
        listSelTracks->addItem(new QListWidgetItem(*item));
    }
}

void CDlgCombineTracks::slotDel()
{
    QListWidgetItem * item;
    QList<QListWidgetItem*> items = listSelTracks->selectedItems();

    foreach(item, items){
        delete item;
    }
}

void CDlgCombineTracks::accept()
{
    const QMap<QString,CTrack*>& dict = CTrackDB::self().getTracks();
    QList<CTrack*> tracks;
    QListWidgetItem * item;
    QList<QListWidgetItem*> items = listSelTracks->selectedItems();

    foreach(item, items){
        tracks << dict[item->data(Qt::UserRole).toString()];
    }

    if(tracks.isEmpty()) return;
}

