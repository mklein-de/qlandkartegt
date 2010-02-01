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

#include "CTrackUndoCommandSelect.h"
#include <QObject>
#include <QDebug>
#include "CTrackDB.h"

CTrackUndoCommandSelect::CTrackUndoCommandSelect(CTrack *track, const QRect& rect, bool select)
: track(track), select(select) , rect(rect)
{
    if(select)
        setText(QObject::tr("Select Trackpoints"));
    else
        setText(QObject::tr("Unselect Trackpoints"));

}


CTrackUndoCommandSelect::~CTrackUndoCommandSelect()
{

}


void CTrackUndoCommandSelect::redo()
{
    //qDebug() << Q_FUNC_INFO;

    selectionList.clear();
    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    // trkpt = trkpts.begin();
    for (trkpt = trkpts.begin(); trkpt != trkpts.end(); ++trkpt)
    {
        //     qDebug() << &*trkpt ;//<< *trkpt ;
        if(rect.contains(trkpt->px) && !(trkpt->flags & CTrack::pt_t::eDeleted))
        {
            if (select)
            {
                trkpt->flags |= CTrack::pt_t::eSelected;
                selectionList.insert(trkpt->idx);
            }
            else
            {
                if (trkpt->flags & CTrack::pt_t::eSelected)
                {
                    selectionList.insert(trkpt->idx);
                    trkpt->flags &= ~CTrack::pt_t::eSelected;
                }
            }

        }
    }

    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}


void CTrackUndoCommandSelect::undo()
{
    //qDebug() << Q_FUNC_INFO;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    for (trkpt = trkpts.begin(); trkpt != trkpts.end(); ++trkpt)
    {
        if (selectionList.contains(trkpt->idx))
        {
            if (!select)
                trkpt->flags |= CTrack::pt_t::eSelected;
            else
                trkpt->flags &= ~CTrack::pt_t::eSelected;
        }
    }

    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}
