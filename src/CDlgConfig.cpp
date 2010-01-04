/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#include "config.h"
#include "CDlgConfig.h"
#include "CResources.h"
#include "IDevice.h"
#include "CUnitImperial.h"
#include "CUnitNautic.h"
#include "CUnitMetric.h"
#include "CMapTDB.h"

#include <QtGui>

#define XSTR(x) STR(x)
#define STR(x) #x

CDlgConfig::CDlgConfig(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);
    connect(toolFont,SIGNAL(clicked()),this,SLOT(slotSelectFont()));
    fillTypeCombo();
}


CDlgConfig::~CDlgConfig()
{

}


void CDlgConfig::exec()
{
    CResources& resources = CResources::self();

    checkProxy->setChecked(resources.m_useHttpProxy);
    lineProxyURL->setText(resources.m_httpProxy);
    lineProxyPort->setText(QString("%1").arg(resources.m_httpProxyPort));

    labelFont->setFont(resources.m_mapfont);
    if(resources.unit->type == "metric") {
        radioMetric->setChecked(true);
    }
    else if(resources.unit->type == "nautic") {
        radioNautic->setChecked(true);
    }
    else if(resources.unit->type == "imperial") {
        radioImperial->setChecked(true);
    }

    checkPlaySound->setChecked(resources.m_playSound);
    checkFlipMouseWheel->setChecked(resources.m_flipMouseWheel);

    connect(comboBrowser,SIGNAL(currentIndexChanged(int)),this,SLOT(slotBrowserChanged(int)));
    comboBrowser->setCurrentIndex(resources.m_eBrowser);
    lineBrowserCmd->setText(resources.cmdOther);

    comboDevice->addItem(tr(""),"");
    comboDevice->addItem(tr("QLandkarte M"), "QLandkarteM");
    comboDevice->addItem(tr("Garmin"), "Garmin");
    comboDevice->addItem(tr("NMEA"), "NMEA");
    comboDevice->addItem(tr("Mikrokopter"), "Mikrokopter");

    connect(comboDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDeviceChanged(int)));
    comboDevice->setCurrentIndex(comboDevice->findData(resources.m_devKey));

    lineDevIPAddr->setText(resources.m_devIPAddress);
    lineDevIPPort->setText(QString::number(resources.m_devIPPort));
    lineDevSerialPort->setText(resources.m_devSerialPort);

    checkDownloadTrk->setChecked(IDevice::m_DownloadAllTrk);
    checkDownloadWpt->setChecked(IDevice::m_DownloadAllWpt);
    checkDownloadRte->setChecked(IDevice::m_DownloadAllRte);
    checkUploadTrk->setChecked(IDevice::m_UploadAllTrk);
    checkUploadWpt->setChecked(IDevice::m_UploadAllWpt);
    checkUploadRte->setChecked(IDevice::m_UploadAllRte);

    QDialog::exec();
}


void CDlgConfig::accept()
{
    CResources& resources = CResources::self();

    resources.m_useHttpProxy    = checkProxy->isChecked();
    resources.m_httpProxy       = lineProxyURL->text();
    resources.m_httpProxyPort   = lineProxyPort->text().toUInt();

    emit resources.sigProxyChanged();

    resources.m_mapfont         = labelFont->font();

    if(radioMetric->isChecked()) {
        resources.unit = new CUnitMetric(&resources);
    }
    if(radioImperial->isChecked()) {
        resources.unit = new CUnitImperial(&resources);
    }
    if(radioNautic->isChecked()) {
        resources.unit = new CUnitNautic(&resources);
    }

    resources.m_flipMouseWheel  = checkFlipMouseWheel->isChecked();
    resources.m_playSound       = checkPlaySound->isChecked();

    resources.m_eBrowser        = (CResources::browser_e)comboBrowser->currentIndex();
    resources.cmdOther          = lineBrowserCmd->text();

    resources.m_devKey          = comboDevice->itemData(comboDevice->currentIndex()).toString();
    resources.m_devIPAddress    = lineDevIPAddr->text();
    resources.m_devIPPort       = lineDevIPPort->text().toUShort();
    resources.m_devSerialPort   = lineDevSerialPort->text();
    resources.m_devType         = comboDevType->itemText(comboDevType->currentIndex());

    if(resources.m_device) {
        delete resources.m_device;
        resources.m_device = 0;
    }

    emit resources.sigDeviceChanged();

    IDevice::m_DownloadAllTrk   = checkDownloadTrk->isChecked();
    IDevice::m_DownloadAllWpt   = checkDownloadWpt->isChecked();
    IDevice::m_DownloadAllRte   = checkDownloadRte->isChecked();
    IDevice::m_UploadAllWpt     = checkUploadWpt->isChecked();
    IDevice::m_UploadAllTrk     = checkUploadTrk->isChecked();
    IDevice::m_UploadAllRte     = checkUploadRte->isChecked();

    QDialog::accept();
}


void CDlgConfig::slotCurrentDeviceChanged(int index)
{
    lineDevIPAddr->setEnabled(false);
    labelDevIPAddr->setEnabled(false);
    lineDevIPPort->setEnabled(false);
    labelDevIPPort->setEnabled(false);
    lineDevSerialPort->setEnabled(false);
    labelDevSerialPort->setEnabled(false);
    comboDevType->setEnabled(false);
    labelDevType->setEnabled(false);

    if(comboDevice->itemData(index) == "QLandkarteM") {
        lineDevIPAddr->setEnabled(true);
        labelDevIPAddr->setEnabled(true);
        lineDevIPPort->setEnabled(true);
        labelDevIPPort->setEnabled(true);
    }
    else if(comboDevice->itemData(index) == "Garmin") {
        comboDevType->setEnabled(true);
        lineDevSerialPort->setEnabled(true);
        labelDevSerialPort->setEnabled(true);
        labelDevType->setEnabled(true);
        fillTypeCombo();
    }
    else if(comboDevice->itemData(index) == "Mikrokopter") {
        lineDevSerialPort->setEnabled(true);
        labelDevSerialPort->setEnabled(true);
    }
    else if(comboDevice->itemData(index) == "NMEA") {
        lineDevSerialPort->setEnabled(true);
        labelDevSerialPort->setEnabled(true);
    }
}


void CDlgConfig::slotSelectFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, labelFont->font(), this);
    if(ok) {
        labelFont->setFont(font);
    }

}


void CDlgConfig::fillTypeCombo()
{
    comboDevType->clear();

    CResources& resources = CResources::self();
    QRegExp regex("lib(.*)\\" XSTR(SHARED_LIB_EXT));
    QString file;
    QStringList files;
    QDir inst_dir(XSTR(PLUGINDIR));
    files = inst_dir.entryList(QString("*" XSTR(SHARED_LIB_EXT)).split(','));

    foreach(file,files) {
        if(regex.exactMatch(file)) {
            comboDevType->addItem(regex.cap(1));
        }
    }
    comboDevType->setCurrentIndex(comboDevType->findText(resources.m_devType));

    if(files.isEmpty()) {
        labelMessage->setText(tr("No plugins found. I expect them in: %1").arg(XSTR(PLUGINDIR)));
    }
    else {
        labelMessage->setText("");
    }
}


void CDlgConfig::slotBrowserChanged(int idx)
{
    if(idx == CResources::eOther) {
        lineBrowserCmd->setEnabled(true);
    }
    else {
        lineBrowserCmd->setEnabled(false);
    }
}
