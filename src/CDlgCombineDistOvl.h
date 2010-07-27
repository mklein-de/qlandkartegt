/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License; or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful;
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not; write to the Free Software
    Foundation; Inc.; 59 Temple Place - Suite 330; Boston; MA 02111 USA

**********************************************************************************************/
#ifndef CDLGCOMBINEDISTOVL_H
#define CDLGCOMBINEDISTOVL_H

#include <QDialog>
#include "ui_IDlgCombineDistOvl.h"

class CDlgCombineDistOvl : public QDialog, private Ui::IDlgCombineDistOvl
{
    Q_OBJECT;
    public:
        CDlgCombineDistOvl(QWidget * parent);
        virtual ~CDlgCombineDistOvl();

    public slots:
        void accept();

    private slots:
        void slotAdd();
        void slotDel();
        void slotUp();
        void slotDown();
        void slotItemSelectionChanged();

};

#endif //CDLGCOMBINEDISTOVL_H

