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

#include "CMapSearchWidget.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CSearchDB.h"
#include "CPicProcess.h"
#include "CMapSearchCanvas.h"
#include "CTabWidget.h"
#include "CImage.h"
#include "CMapSearchThread.h"

#include <QtGui>

CMapSearchWidget::CMapSearchWidget(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("CMapSearchWidget");
    setAttribute(Qt::WA_DeleteOnClose,true);
    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    toolNewMask->setIcon(QIcon(":/icons/iconWizzard16x16.png"));
    toolSaveMask->setIcon(QIcon(":/icons/iconFileSave16x16.png"));
    toolDeleteMask->setIcon(QIcon(":/icons/iconDelete16x16.png"));

    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushArea, SIGNAL(clicked()), this, SLOT(slotSelectArea()));
    connect(pushSearch, SIGNAL(clicked()), this, SLOT(slotSearch()));
    connect(sliderThreshold, SIGNAL( valueChanged (int)), this, SLOT(slotThreshold(int)));
    connect(toolSaveMask, SIGNAL(clicked()), this, SLOT(slotSaveMask()));
    connect(toolNewMask, SIGNAL(clicked()), this, SLOT(slotSelectMask()));
    connect(comboSymbols, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotSelectMaskByName(const QString&)));
    connect(toolDeleteMask, SIGNAL(clicked()), this, SLOT(slotDeleteMask()));

    mask = new CImage(this);

    loadMaskCollection();

    thread = new CMapSearchThread(this);
    connect(thread, SIGNAL(finished()), this, SLOT(slotSearchFinished()));
}


CMapSearchWidget::~CMapSearchWidget()
{
    if(!canvas.isNull()) {
        delete canvas;
    }

}


void CMapSearchWidget::slotSelectArea()
{
    theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseSelectArea);
}


void CMapSearchWidget::slotSelectMask()
{
    labelMask->setText(tr("No mask selected."));

    if(canvas.isNull()) {
        canvas = new CMapSearchCanvas(this);
        connect(canvas, SIGNAL(sigSelection(const QPixmap&)), this, SLOT(slotMaskSelection(const QPixmap&)));
        theMainWindow->getCanvasTab()->addTab(canvas, tr("Symbols"));
    }

    binarizeViewport(-1);
}



void CMapSearchWidget::slotSearch()
{

/*
    CImage xxx(CMapDB::self().getMap().getBuffer());
    xxx.binarize(sliderThreshold->value());

    QList<QPoint> symbols;
    xxx.findSymbol(symbols, *mask);

    IMap& map = CMapDB::self().getMap();
    QPoint symbol;
    int cnt = 0;
    QString name = lineMaskName->text().isEmpty() ? "Sym." : lineMaskName->text();
    foreach(symbol, symbols){
        double u = symbol.x();
        double v = symbol.y();

        map.convertPt2Rad(u,v);
        CSearchDB::self().add(tr("%2 %1").arg(++cnt).arg(name), u, v);
    }

*/
    thread->start(sliderThreshold->value(), mask->rgb(), area);
    pushSearch->setEnabled(false);

}

void CMapSearchWidget::slotSearchFinished()
{
    QPoint symbol;
    const QList<QPoint>& symbols = thread->getLastResult();

    IMap& map = CMapDB::self().getMap();

    int cnt = 0;
    QString name = lineMaskName->text().isEmpty() ? "Sym." : lineMaskName->text();

    foreach(symbol, symbols){
        double u = symbol.x();
        double v = symbol.y();

        map.convertM2Rad(u,v);
        CSearchDB::self().add(tr("%2 %1").arg(++cnt).arg(name), u, v);
    }

}

void CMapSearchWidget::slotThreshold(int i)
{
    binarizeViewport(sliderThreshold->value());
}

void CMapSearchWidget::slotMaskSelection(const QPixmap& pixmap)
{
    mask->setPixmap(pixmap.toImage());
    labelMask->setPixmap(QPixmap::fromImage(mask->mask()));

    checkGui();
}

void CMapSearchWidget::setArea(const CMapSelection& ms)
{
    area = ms;
    QString pos1, pos2;

    GPS_Math_Deg_To_Str(ms.lon1 * RAD_TO_DEG, ms.lat1 * RAD_TO_DEG, pos1);
    GPS_Math_Deg_To_Str(ms.lon2 * RAD_TO_DEG, ms.lat2 * RAD_TO_DEG, pos2);

    labelArea->setText(QString("%1\n%2\n%3").arg(ms.description).arg(pos1).arg(pos2));
}


void CMapSearchWidget::binarizeViewport(int t)
{
    if(canvas.isNull()) {
        return;
    }

    CImage xxx(CMapDB::self().getMap().getBuffer());
    canvas->setBuffer(QPixmap::fromImage(xxx.binarize(t)));

    if(t < 0)sliderThreshold->setValue(xxx.getThreshold());
}

void CMapSearchWidget::slotSaveMask()
{
    if(lineMaskName->text().isEmpty()){
        QMessageBox::warning(this, tr("Missing name..."), tr("Please provide a symbol name to save the symbol."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    QFile file(QDir::home().filePath(QString(".config/QLandkarteGT/%1.msk").arg(lineMaskName->text())));
    file.open(QIODevice::WriteOnly);

    QDataStream out(&file);
    out << sliderThreshold->value();
    out << (int)1;
    out << mask->rgb();
    out << (int)0;

    file.close();

    loadMaskCollection();
}

void CMapSearchWidget::slotSelectMaskByName(const QString& name)
{
    QImage pixmap;
    int     intval;

    QDir path(QDir::home().filePath(".config/QLandkarteGT/"));
    QFile file(path.filePath(name + ".msk"));
    if(file.open(QIODevice::ReadOnly)){
        QDataStream in(&file);

        in >> intval;
        sliderThreshold->setValue(intval);
        in >> intval;
        in >> pixmap;

        lineMaskName->setText(name);

        mask->setPixmap(pixmap);
        labelMask->setPixmap(QPixmap::fromImage(mask->mask()));

        pushSearch->setEnabled(true);
        toolSaveMask->setEnabled(true);

        file.close();
    }
    else{
        labelMask->setText(tr("No mask selected."));
    }

    checkGui();
}

void CMapSearchWidget::slotDeleteMask()
{
    QString name    = lineMaskName->text();
    int index       = comboSymbols->findText(name);

    if(index != -1){
        comboSymbols->removeItem(index);
        QDir path(QDir::home().filePath(".config/QLandkarteGT/"));
        QFile file(path.filePath(name + ".msk"));
        file.remove();

        labelMask->setText(tr("No mask selected."));
    }

    checkGui();
}

void CMapSearchWidget::loadMaskCollection()
{
    QString name = lineMaskName->text();

    comboSymbols->clear();
    comboSymbols->addItem(tr("no mask"));

    QDir path(QDir::home().filePath(".config/QLandkarteGT/"));
    QStringList filter;
    filter << "*.msk";
    QStringList maskfiles = path.entryList(filter, QDir::Files, QDir::Name);
    QString     maskfile;

    foreach(maskfile, maskfiles){
        QImage pixmap;
        int     intval;

        QFile file(path.filePath(maskfile));
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in >> intval;
        in >> intval;
        in >> pixmap;

        file.close();

        CImage imgMask(pixmap);
        pixmap = imgMask.mask();

        comboSymbols->addItem(QPixmap::fromImage(pixmap), QFileInfo(maskfile).baseName());
    }

    comboSymbols->setCurrentIndex(comboSymbols->findText(name));

    checkGui();
}

void CMapSearchWidget::checkGui()
{
    if(!labelMask->text().isEmpty()){
        lineMaskName->clear();
        pushSearch->setEnabled(false);
        toolSaveMask->setEnabled(false);
        toolDeleteMask->setEnabled(false);
    }
    else{
        pushSearch->setEnabled(true);
        toolSaveMask->setEnabled(true);
        toolDeleteMask->setEnabled(true);
    }
}
