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

#include "CResources.h"
#include "CDeviceTBDOE.h"
#include "CDeviceGarmin.h"

#include <QtGui>

CResources * CResources::m_self = 0;

CResources::CResources(QObject * parent)
: QObject(parent)
, pathMaps("./")
, m_useHttpProxy(false)
, m_httpProxyPort(8080)
, m_eBrowser(eFirefox)
, cmdFirefox("firefox \"%s\" &")
, cmdKonqueror("kfmclient exec \"%s\"")
, time_offset(0)
, m_device(0)
, m_devIPPort(4242)
{
    m_self = this;

    QSettings cfg;

    QString family  = cfg.value("environment/mapfont/family","Arial").toString();
    int size        = cfg.value("environment/mapfont/size",8).toInt();
    m_mapfont = QFont(family,size);

    m_doMetric        = cfg.value("environment/doMetric",true).toBool();

    m_useHttpProxy    = cfg.value("network/useProxy",m_useHttpProxy).toBool();
    m_httpProxy       = cfg.value("network/proxy/url",m_httpProxy).toString();
    m_httpProxyPort   = cfg.value("network/proxy/port",m_httpProxyPort).toUInt();

    m_eBrowser        = (bowser_e)cfg.value("network/browser",m_eBrowser).toInt();
    cmdOther          = cfg.value("network/browser/other","my command \"%s\"").toString();

    emit sigProxyChanged();

    m_devKey          = cfg.value("device/key",m_devKey).toString();
    m_devIPAddress    = cfg.value("device/ipAddr",m_devIPAddress).toString();
    m_devIPPort       = cfg.value("device/ipPort",m_devIPPort).toUInt();
    m_devSerialPort   = cfg.value("device/serialPort",m_devSerialPort).toString();
    m_devType         = cfg.value("device/type",m_devType).toString();

    IDevice::m_DownloadAllTrk   = cfg.value("device/dnlTrk",IDevice::m_DownloadAllTrk).toBool();
    IDevice::m_DownloadAllWpt   = cfg.value("device/dnlWpt",IDevice::m_DownloadAllWpt).toBool();
    IDevice::m_UploadAllWpt     = cfg.value("device/uplWpt",IDevice::m_UploadAllWpt).toBool();

    pathMaps        = cfg.value("path/maps",pathMaps).toString();
}


CResources::~CResources()
{
    QSettings cfg;

    cfg.setValue("environment/mapfont/family",m_mapfont.family());
    cfg.setValue("environment/mapfont/size",m_mapfont.pointSize());
    cfg.setValue("environment/doMetric",m_doMetric);

    cfg.setValue("network/useProxy",m_useHttpProxy);
    cfg.setValue("network/proxy/url",m_httpProxy);
    cfg.setValue("network/proxy/port",m_httpProxyPort);

    cfg.setValue("network/browser",m_eBrowser);
    cfg.setValue("network/browser/other",cmdOther);

    cfg.setValue("device/key",m_devKey);
    cfg.setValue("device/ipAddr",m_devIPAddress);
    cfg.setValue("device/ipPort",m_devIPPort);
    cfg.setValue("device/serialPort",m_devSerialPort);
    cfg.setValue("device/serialPort",m_devSerialPort);
    cfg.setValue("device/type",m_devType);

    cfg.setValue("device/dnlTrk",IDevice::m_DownloadAllTrk);
    cfg.setValue("device/dnlWpt",IDevice::m_DownloadAllWpt);
    cfg.setValue("device/uplWpt",IDevice::m_UploadAllWpt);
}


bool CResources::getHttpProxy(QString& url, quint16& port)
{
    url  = m_httpProxy;
    port = m_httpProxyPort;
    return m_useHttpProxy;
}


IDevice * CResources::device()
{
    // purge device if the key does not match
    if(m_device && (m_device->getDevKey() != m_devKey)) {
        qDebug() << m_device->getDevKey() << m_devKey;
        delete m_device;
        m_device = 0;
    }

    // allocate new device
    if(!m_device) {
        if(m_devKey == "QLandkarteM" && !m_devIPAddress.isEmpty() && m_devIPPort) {
            m_device = new CDeviceTBDOE(m_devIPAddress,m_devIPPort,this);
        }
        else if(m_devKey == "Garmin" && !m_devType.isEmpty()) {
            m_device = new CDeviceGarmin(m_devType, m_devSerialPort, this);
        }
    }

    // still noe device?
    if(!m_device) {
        qWarning() << "no device";
        // TODO: would be nicer to open the setup dialog
        QMessageBox::critical(0,tr("No device."),tr("You have to select a device in Setup->Config->Device & Xfer"), QMessageBox::Abort, QMessageBox::Abort);
    }

    return m_device;
}

void CResources::openLink(const QString& link)
{
    QString cmd;
    if(m_eBrowser == eFirefox) {
        cmd.sprintf(cmdFirefox.toAscii(),link.toAscii().data());
    }
    else if(m_eBrowser == eKonqueror) {
        cmd.sprintf(cmdKonqueror.toAscii(),link.toAscii().data());
    }
    else if(m_eBrowser == eOther) {
        cmd.sprintf(cmdOther.toAscii(),link.toAscii().data());
    }
    else {
        return;
    }

    system(cmd.toAscii());

}
