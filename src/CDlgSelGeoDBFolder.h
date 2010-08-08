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
#ifndef CDLGSELGEODBFOLDER_H
#define CDLGSELGEODBFOLDER_H

#include <QDialog>
#include "ui_IDlgSelGeoDBFolder.h"

class QSqlDatabase;

class CDlgSelGeoDBFolder : public QDialog, private Ui::IDlgSelGeoDBFolder
{
    Q_OBJECT;
    public:
        CDlgSelGeoDBFolder(QSqlDatabase& db, quint64& result, bool topLevelToo = false);
        virtual ~CDlgSelGeoDBFolder();

    public slots:
        void accept();

    private:
        void queryChildrenFromDB(QTreeWidgetItem * parent);
        QSqlDatabase& db;
        quint64& result;
        bool topLevelToo;
};

#endif //CDLGSELGEODBFOLDER_H

