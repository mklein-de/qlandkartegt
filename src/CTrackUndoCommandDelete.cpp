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

#include "CTrackUndoCommandDelete.h"
#include <QMap>
#include <QString>
#include <QDebug>
#include "CTrack.h"
#include "CTrackDB.h"
#include "CTrackToolWidget.h"

CTrackUndoCommandDelete::CTrackUndoCommandDelete(CTrackDB *trackDB, const QString &key, bool silent)
: trackDB(trackDB) , key(key), silent(silent)
{
    setText(QObject::tr("delete track"));
    track = 0;
}


CTrackUndoCommandDelete::~CTrackUndoCommandDelete()
{
    qDebug() << Q_FUNC_INFO << track << track->ref;
    if (track->ref < 1)
    {
        delete track;
    }
}


void CTrackUndoCommandDelete::redo()
{
    qDebug() << Q_FUNC_INFO;
    track = trackDB->take(key, silent);
    track->ref++;
}


void CTrackUndoCommandDelete::undo()
{
    qDebug() << Q_FUNC_INFO << track;
    trackDB->insert(key, track, silent);
    track->ref--;
}
