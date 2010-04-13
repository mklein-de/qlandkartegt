/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CDLGCONFIG3D_H
#define CDLGCONFIG3D_H

#include <QDialog>
#include "ui_IDlgConfig3D.h"

class CMap3D;

class CDlgConfig3D : public QDialog, private Ui::IDlgConfig3D
{
    Q_OBJECT;
    public:
        CDlgConfig3D(CMap3D& parent);
        virtual ~CDlgConfig3D();

    public slots:
        void exec();
        void accept();

    private:
        CMap3D& view3D;
};
#endif                           //CDLGCONFIG3D_H
