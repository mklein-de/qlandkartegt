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
#ifndef CDLGEDITDISTANCE_H
#define CDLGEDITDISTANCE_H

#include <QDialog>
#include "ui_IDlgEditDistance.h"

class COverlayDistance;

class CDlgEditDistance : public QDialog, private Ui::IDlgEditDistance
{
    Q_OBJECT;
    public:
        CDlgEditDistance(COverlayDistance &ovl, QWidget * parent);
        virtual ~CDlgEditDistance();

    public slots:
        void accept();

    private:
        COverlayDistance& ovl;
};
#endif                           //CDLGEDITDISTANCE_H
