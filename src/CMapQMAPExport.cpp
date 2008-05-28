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

#include "CMapQMAPExport.h"
#include "CMapSelection.h"
#include "CMapDB.h"

#include <QtGui>

CMapQMAPExport::CMapQMAPExport(const CMapSelection& mapsel, QWidget * parent)
    : QDialog(parent)
    , mapsel(mapsel)
{
    setupUi(this);

    toolPath->setIcon(QPixmap(":/icons/iconFileLoad16x16"));

    QSettings cfg;
    labelPath->setText(cfg.value("path/export","./").toString());

    IMap& map = CMapDB::self().getMap();
    linePrefix->setText(QString("%1_%2_%3").arg(QFileInfo(map.getFilename()).baseName()).arg(mapsel.lon1 * RAD_TO_DEG).arg(mapsel.lat1 * RAD_TO_DEG));
    lineDescription->setText(QFileInfo(map.getFilename()).baseName());

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotOutputPath()));
    connect(pushExport, SIGNAL(clicked()), this, SLOT(slotStart()));
}

CMapQMAPExport::~CMapQMAPExport()
{

}

void CMapQMAPExport::slotOutputPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select ouput path..."), labelPath->text());
    if(path.isEmpty()) return;

    QSettings cfg;
    cfg.setValue("path/export", path);
    labelPath->setText(path);
}

void CMapQMAPExport::slotStart()
{

}
