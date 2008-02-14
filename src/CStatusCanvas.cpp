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

#include "CStatusCanvas.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>

CStatusCanvas::CStatusCanvas(CCanvas * parent)
    : QWidget(parent)
    , canvas(parent)
{
    setupUi(this);
//     checkShading->setChecked(parent->showShading);

    connect(radioNone, SIGNAL(clicked(bool)), this, SLOT(slotShowShading(bool)));
    connect(radioShading, SIGNAL(clicked(bool)), this, SLOT(slotShowShading(bool)));
    connect(radioContour, SIGNAL(clicked(bool)), this, SLOT(slotShowShading(bool)));
}

CStatusCanvas::~CStatusCanvas()
{

}

void CStatusCanvas::updateShadingType()
{
    switch(CMapDB::self().getMap().getOverlay()){
        case IMap::eNone:
            radioNone->setChecked(true);
            break;
        case IMap::eShading:
            radioShading->setChecked(true);
            break;
        case IMap::eContour:
            radioContour->setChecked(true);
            break;
    }
}

void CStatusCanvas::slotShowShading(bool checked)
{
    IMap& map = CMapDB::self().getMap();

    QString button = sender()->objectName();
    if(button == "radioNone"){
        map.setOverlay(IMap::eNone);
    }
    else if(button == "radioShading"){
        map.setOverlay(IMap::eShading);
    }
    else if(button == "radioContour"){
        map.setOverlay(IMap::eContour);
    }

    canvas->update();
}

