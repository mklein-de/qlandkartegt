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

#include "CMapUndoCommandMove.h"
#include <QMap>
#include <QString>
#include <QDebug>
#include "IMap.h"

CMapUndoCommandMove::CMapUndoCommandMove(IMap *map, const QPoint &from, const QPoint &to)
: map(map) , from(from), to(to)
{
    setText(QObject::tr("Move Map"));
}


CMapUndoCommandMove::~CMapUndoCommandMove()
{

}


void CMapUndoCommandMove::redo()
{
    //qDebug() << Q_FUNC_INFO;
    map->move(from,to);
}


void CMapUndoCommandMove::undo()
{
    //qDebug() << Q_FUNC_INFO ;
    map->move(to, from);
}
