/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CDLGDEVICEEXPORTPATH_H
#define CDLGDEVICEEXPORTPATH_H

#include "ui_IDlgDeviceExportPath.h"
#include <QDialog>

class QDir;
class QString;

class CDlgDeviceExportPath : public QDialog, private Ui::IDlgDeviceExportPath
{
    Q_OBJECT;
    public:
        CDlgDeviceExportPath(const QString &what, QDir &dir, QString &subdir, QWidget *parent);
        virtual ~CDlgDeviceExportPath();

    private slots:
        void slotItemClicked(QListWidgetItem*item);
        void slotReturnPressed();

    private:
        QString& subdir;

};
#endif                           //CDLGDEVICEEXPORTPATH_H
