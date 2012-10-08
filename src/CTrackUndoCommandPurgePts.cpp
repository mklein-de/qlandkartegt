//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#include "CTrackUndoCommandPurgePts.h"
#include <QObject>
#include "CTrackDB.h"
#include <QDebug>
CTrackUndoCommandPurgePts::CTrackUndoCommandPurgePts(CTrack *track)
: track(track)
{
    setText(QObject::tr("Purge Selection"));
}


CTrackUndoCommandPurgePts::~CTrackUndoCommandPurgePts()
{

}


void CTrackUndoCommandPurgePts::redo()
{
    //qDebug() << Q_FUNC_INFO;
    purgedList.clear();
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & CTrack::pt_t::eSelected)
        {
            //            trkpt->flags |= CTrack::pt_t::eDeleted;
            trkpt->flags &= ~CTrack::pt_t::eSelected;
            if(trkpt->flags & CTrack::pt_t::eDeleted)
            {
                trkpt->flags &= ~CTrack::pt_t::eDeleted;
            }
            else
            {
                trkpt->flags |= CTrack::pt_t::eDeleted;
            }

            purgedList << trkpt->idx;
        }
        ++trkpt;
    }
    track->rebuild(false);
    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}


void CTrackUndoCommandPurgePts::undo()
{
    //qDebug() << Q_FUNC_INFO;
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

    while(trkpt != trkpts.end())
    {
        if(purgedList.contains(trkpt->idx))
        {
            if(trkpt->flags & CTrack::pt_t::eDeleted)
            {
                trkpt->flags &= ~CTrack::pt_t::eDeleted;
            }
            else
            {
                trkpt->flags |= CTrack::pt_t::eDeleted;
            }
            //            trkpt->flags |= CTrack::pt_t::eSelected;
        }
        ++trkpt;
    }
    track->rebuild(false);
    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}
