/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CDlgMapTDBConfig.h"
#include "CMapTDB.h"

CDlgMapTDBConfig::CDlgMapTDBConfig(CMapTDB * map)
: map(map)
{
    setupUi(this);

    checkUseTyp->setChecked(map->useTyp);
    checkBitmapLines->setChecked(map->useBitmapLines);
    checkGrowLines->setChecked(map->growLines);
    checkTextAboveLine->setChecked(map->textAboveLine);
}

CDlgMapTDBConfig::~CDlgMapTDBConfig()
{

}

void CDlgMapTDBConfig::accept()
{
    map->useTyp         = checkUseTyp->isChecked();
    map->useBitmapLines = checkBitmapLines->isChecked();
    map->growLines      = checkGrowLines->isChecked();
    map->textAboveLine  = checkTextAboveLine->isChecked();

    QDialog::accept();
}
