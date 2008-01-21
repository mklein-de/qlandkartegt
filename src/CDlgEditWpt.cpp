/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgEditWpt.h"
#include "CWpt.h"
#include "WptIcons.h"
#include "GeoMath.h"

#include <QtGui>

CDlgEditWpt::CDlgEditWpt(CWpt &wpt, QWidget * parent)
    : QDialog(parent)
    , wpt(wpt)
    , idxImg(0)
{
    setupUi(this);
    connect(pushAdd, SIGNAL(clicked()), this, SLOT(slotAddImage()));
    connect(pushDel, SIGNAL(clicked()), this, SLOT(slotDelImage()));
    connect(pushNext, SIGNAL(clicked()), this, SLOT(slotNextImage()));
    connect(pushPrev, SIGNAL(clicked()), this, SLOT(slotPrevImage()));
}

CDlgEditWpt::~CDlgEditWpt()
{

}

int CDlgEditWpt::exec()
{
    toolIcon->setIcon(getWptIconByName(wpt.icon));
    toolIcon->setObjectName(wpt.icon);

    lineName->setText(wpt.name);

    QString pos;
    GPS_Math_Deg_To_Str(wpt.lon, wpt.lat, pos);
    linePosition->setText(pos);

    //TODO: that has to be metric/imperial
    lineAltitude->setText(QString::number(wpt.altitude,'f',0));
    lineProximity->setText(QString::number(wpt.proximity,'f',1));

    textComment->setHtml(wpt.comment);

    if(wpt.images.count() != 0){
        showImage(0);
        pushDel->setEnabled(true);
    }

    return QDialog::exec();
}

void CDlgEditWpt::accept()
{
    if(lineName->text().isEmpty()){
        QMessageBox::warning(0,tr("Error"),tr("You must provide a waypoint indentifier."),QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    if(linePosition->text().isEmpty()){
        QMessageBox::warning(0,tr("Error"),tr("You must provide a waypoint position."),QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    if(!GPS_Math_Str_To_Deg(linePosition->text(), wpt.lon, wpt.lat)){
        return;
    }
    wpt.icon        = toolIcon->objectName();
    wpt.name        = lineName->text();
    wpt.altitude    = lineAltitude->text().toFloat();
    wpt.proximity   = lineProximity->text().toFloat();
    wpt.comment     = textComment->toHtml();

    QDialog::accept();
}

void CDlgEditWpt::slotAddImage()
{
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select image file")
                                                    ,"./"
                                                    ,"Image (*)"
                                                );
    if(filename.isEmpty()) return;

    QString info =  QInputDialog::getText( this, tr("Add comment ..."), tr("comment"), QLineEdit::Normal, QFileInfo(filename).fileName());


    CWpt::image_t img;
    img.info = info;
    img.pixmap = QPixmap(filename);
    wpt.images.push_back(img);
    showImage(wpt.images.count() - 1);

    pushDel->setEnabled(true);

}

void CDlgEditWpt::slotDelImage()
{
    wpt.images.removeAt(idxImg);
    while(idxImg >= wpt.images.count()) --idxImg;
    showImage(idxImg);

    pushDel->setEnabled(wpt.images.count() != 0);
}

void CDlgEditWpt::slotNextImage()
{
    showImage(idxImg + 1);
}

void CDlgEditWpt::slotPrevImage()
{
    showImage(idxImg - 1);
}

void CDlgEditWpt::showImage(int idx)
{
    if(idx < 0) idx = 0;

    if(idx < wpt.images.count()){
        idxImg = idx;

        CWpt::image_t& img = wpt.images[idx];
        labelImage->setPixmap(img.pixmap);
        labelInfo->setText(img.info);

        pushNext->setEnabled(idx < (wpt.images.count() - 1) && wpt.images.count() != 1);
        pushPrev->setEnabled(idx > 0);
    }
    else{
        labelImage->setText(tr("no image"));
        labelInfo->setText("");
    }
}
