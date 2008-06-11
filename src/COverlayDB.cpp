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

#include "COverlayDB.h"
#include "COverlayToolWidget.h"
#include "COverlayText.h"
#include "COverlayTextBox.h"

#include <QtGui>

COverlayDB * COverlayDB::m_self = 0;

COverlayDB::COverlayDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    toolview    = new COverlayToolWidget(tb);
}

COverlayDB::~COverlayDB()
{

}

void COverlayDB::draw(QPainter& p, const QRect& r)
{
    IOverlay * overlay;
    foreach(overlay, overlays){
        overlay->draw(p);
    }
}

void COverlayDB::loadGPX(CGpx& gpx)
{
}

void COverlayDB::saveGPX(CGpx& gpx)
{
}

void COverlayDB::loadQLB(CQlb& qlb)
{
}

void COverlayDB::saveQLB(CQlb& qlb)
{
}

void COverlayDB::clear()
{
}

void COverlayDB::addText(const QRect& rect)
{
    IOverlay * overlay = new COverlayText(rect, this);
    overlays[QString("%1_%2").arg(overlay->type).arg(QDateTime::currentDateTime().toString())] = overlay;

    emit sigChanged();
}

void COverlayDB::addTextBox(const QPointF& anchor, const QRect& rect)
{
    IOverlay * overlay = new COverlayTextBox(anchor, rect, this);
    overlays[QString("%1_%2").arg(overlay->type).arg(QDateTime::currentDateTime().toString())] = overlay;

    emit sigChanged();
}

