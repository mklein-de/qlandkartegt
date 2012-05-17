/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-830551
  Fax:         +49-941-83055-79

  File:        CDlgDeviceExportPath.cpp

  Module:

  Description:

  Created:     05/17/2012

  (C) 2012 DSP Solutions. All rights reserved.


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

