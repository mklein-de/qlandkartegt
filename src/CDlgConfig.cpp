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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CDlgConfig.h"
#include "CResources.h"

CDlgConfig::CDlgConfig(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
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
    radioMetric->setChecked(resources.m_doMetric);
    radioImperial->setChecked(!resources.m_doMetric);
    spinUTCOffset->setValue(resources.m_offsetUTC > 24 ? 0: resources.m_offsetUTC);
    spinUTCOffsetFract->setValue(resources.m_offsetUTCfract);

    comboBrowser->setCurrentIndex(resources.m_eBrowser);
    lineBrowserCmd->setText(resources.cmdOther);

    comboDevice->addItem(tr(""),"");
    comboDevice->addItem(tr("QLandkarte M"), "QLandkarteM");

    connect(comboDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDeviceChanged(int)));
    comboDevice->setCurrentIndex(comboDevice->findData(resources.m_devKey));

    lineDevIPAddr->setText(resources.m_devIPAddress);
    lineDevIPPort->setText(QString::number(resources.m_devIPPort));
    lineDevSerialPort->setText(resources.m_devSerialPort);

//     if(resources.m_devKey == "QLandkarteM"){
//         groupDevice->setEnabled(true);
//     }

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
    resources.m_doMetric        = radioMetric->isChecked();
    resources.m_offsetUTC       = spinUTCOffset->value();
    resources.m_offsetUTCfract  = spinUTCOffsetFract->value();
    resources.setUTCOffset(resources.m_offsetUTC, resources.m_offsetUTCfract);

    resources.m_eBrowser        = (CResources::bowser_e)comboBrowser->currentIndex();
    resources.cmdOther          = lineBrowserCmd->text();

    resources.m_devKey          = comboDevice->itemData(comboDevice->currentIndex()).toString();
    resources.m_devIPAddress    = lineDevIPAddr->text();
    resources.m_devIPPort       = lineDevIPPort->text().toUShort();
    resources.m_devSerialPort   = lineDevSerialPort->text();

    QDialog::accept();
}


void CDlgConfig::slotCurrentDeviceChanged(int index)
{
    if(comboDevice->itemData(index) == "QLandkarteM"){
        groupDevice->setEnabled(true);
    }
    else{
        groupDevice->setEnabled(false);
    }
}
