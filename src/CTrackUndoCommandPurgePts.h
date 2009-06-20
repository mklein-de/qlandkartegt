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

#ifndef CTrackUndoCommandPurgePts_T_H_
#define CTrackUndoCommandPurgePts_T_H_
#include <QUndoCommand>
#include <QSet>
#include "CTrack.h"

class CTrackUndoCommandPurgePts : public QUndoCommand
{
    public:
        CTrackUndoCommandPurgePts(CTrack *track);
        virtual ~CTrackUndoCommandPurgePts();
        virtual void undo();
        virtual void redo();
    private:
        CTrack *track;
        QSet<int> purgedList;
};
#endif                           /* CTRACKUNDOCOMMANDDELETEPT_TS_H_ */
