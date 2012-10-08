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

#ifndef CDLGEDITFOLDER_H
#define CDLGEDITFOLDER_H

#include <QDialog>
#include "ui_IDlgEditFolder.h"

class CDlgEditFolder : public QDialog, private Ui::IDlgEditFolder
{
    Q_OBJECT;
    public:
        CDlgEditFolder(QString& name, QString& comment, int& type);
        virtual ~CDlgEditFolder();

    public slots:
        void accept();

    private:
        QString& name;
        QString& comment;
        int& type;
};
#endif                           //CDLGEDITFOLDER_H
