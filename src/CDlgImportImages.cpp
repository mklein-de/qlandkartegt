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
#include "CDlgImportImages.h"
#include "CSettings.h"
#include "config.h"
#include "CWptDB.h"

#include <QtGui>

CDlgImportImages::CDlgImportImages(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    SETTINGS;
    QString path = cfg.value("path/images", "./").toString();

    labelPath->setText(path);

    radioCopySmall->setChecked(cfg.value("imageImport/copy/small", true).toBool());
    radioCopyLarge->setChecked(cfg.value("imageImport/copy/large", false).toBool());
    radioCopyOriginal->setChecked(cfg.value("imageImport/copy/original", false).toBool());
    radioCopyLink->setChecked(cfg.value("imageImport/copy/link", false).toBool());

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotSelectPath()));

    searchForFiles(path);
}

CDlgImportImages::~CDlgImportImages()
{
}

void CDlgImportImages::accept()
{
    QStringList files;

    for(int i = 0; i < listImages->count(); i++)
    {
        files << listImages->item(i)->data(Qt::UserRole).toString();
    }

    CWptDB::exifMode_e mode = CWptDB::eExifModeSmall;
    if(radioCopyLarge->isChecked())
    {
        mode = CWptDB::eExifModeLarge;
    }
    else if(radioCopyOriginal->isChecked())
    {
        mode = CWptDB::eExifModeOriginal;
    }
    else if(radioCopyLink->isChecked())
    {
        mode = CWptDB::eExifModeLink;
    }

    CWptDB::self().createWaypointsFromImages(files, mode);


    SETTINGS;
    cfg.setValue("path/images", labelPath->text());
    cfg.setValue("imageImport/copy/small", radioCopySmall->isChecked());
    cfg.setValue("imageImport/copy/large", radioCopyLarge->isChecked());
    cfg.setValue("imageImport/copy/original", radioCopyOriginal->isChecked());
    cfg.setValue("imageImport/copy/link", radioCopyLink->isChecked());

    QDialog::accept();
}

void CDlgImportImages::searchForFiles(const QString& path)
{
    QDir dir(path);
    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png";
    QStringList files = dir.entryList(filter, QDir::Files);

    listImages->clear();
    foreach(const QString& file, files)
    {
        QListWidgetItem * item = new QListWidgetItem(listImages);
        item->setText(file);
        item->setData(Qt::UserRole, dir.filePath(file));
    }
}


void CDlgImportImages::slotSelectPath()
{
    QString path = labelPath->text();
    path = QFileDialog::getExistingDirectory(0, tr("Select path..."), path, FILE_DIALOG_FLAGS);
    if(path.isEmpty()) return;
    labelPath->setText(path);

    searchForFiles(path);
}
