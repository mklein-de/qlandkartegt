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
#include "CDlgDeviceExportPath.h"

#include <QtGui>

CDlgDeviceExportPath::CDlgDeviceExportPath(const QString& what, QDir &dir, QString& subdir, QWidget * parent)
    : QDialog(parent)
    , subdir(subdir)
{
    setupUi(this);

    labelHead->setText(tr("Where should I place all %1?").arg(what));

    QStringList dirs = dir.entryList(QStringList("*"), QDir::AllDirs|QDir::NoDotAndDotDot);

    foreach(const QString& d, dirs)
    {
        QListWidgetItem * item = new QListWidgetItem(listWidget);
        item->setText(d);
        item->setIcon(QIcon(":/icons/iconFolderBlue16x16.png"));
    }

    lineEdit->setText(QString("Data_%1").arg(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd")));
    lineEdit->setFocus();
    lineEdit->selectAll();

    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotItemClicked(QListWidgetItem*)));
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
}

CDlgDeviceExportPath::~CDlgDeviceExportPath()
{

}

void CDlgDeviceExportPath::slotItemClicked(QListWidgetItem*item)
{
    if(item == 0) return;

    subdir = item->text();
    QDialog::accept();
}

void CDlgDeviceExportPath::slotReturnPressed()
{
    subdir = lineEdit->text();
    QDialog::accept();
}

