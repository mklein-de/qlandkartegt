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

#include "CResources.h"
#include "CDeviceGarmin.h"
#include "CDeviceQLandkarteM.h"
#ifdef HS_MIKROKOPTER
#include "CDeviceMikrokopter.h"
#endif
#include "CDeviceNMEA.h"
#ifdef HAS_GPSD
#include "CDeviceGPSD.h"
#endif
#include "CLiveLogDB.h"
#include "CUnitMetric.h"
#include "CUnitNautic.h"
#include "CUnitImperial.h"
#include "CMapTDB.h"

#include <QtGui>

CResources * CResources::m_self = 0;

CResources::CResources(QObject * parent)
: QObject(parent)
, pathMaps("./")
, m_useHttpProxy(false)
, m_httpProxyPort(8080)
, m_eBrowser(eFirefox)
#ifdef WIN32
, cmdFirefox("start firefox \"%s\"")
#else
, cmdFirefox("firefox \"%s\" &")
#endif
, cmdKonqueror("kfmclient exec \"%s\"")
, time_offset(0)
, m_device(0)
, m_devIPPort(4242)
, m_flipMouseWheel(false)
{
    m_self = this;

    QSettings cfg;

    QString family  = cfg.value("environment/mapfont/family","Arial").toString();
    int size        = cfg.value("environment/mapfont/size",8).toInt();
    bool bold       = cfg.value("environment/mapfont/bold",false).toBool();
    bool italic     = cfg.value("environment/mapfont/italic",false).toBool();
    m_mapfont = QFont(family,size);
    m_mapfont.setBold(bold);
    m_mapfont.setItalic(italic);

    //m_doMetric        = cfg.value("environment/doMetric",true).toBool();
    m_flipMouseWheel  = cfg.value("environment/flipMouseWheel",m_flipMouseWheel).toBool();

    m_useHttpProxy    = cfg.value("network/useProxy",m_useHttpProxy).toBool();
    m_httpProxy       = cfg.value("network/proxy/url",m_httpProxy).toString();
    m_httpProxyPort   = cfg.value("network/proxy/port",m_httpProxyPort).toUInt();

    m_eBrowser        = (browser_e)cfg.value("network/browser",m_eBrowser).toInt();
    cmdOther          = cfg.value("network/browser/other","my command \"%s\"").toString();

    emit sigProxyChanged();

    m_devKey          = cfg.value("device/key",m_devKey).toString();
    m_devIPAddress    = cfg.value("device/ipAddr",m_devIPAddress).toString();
    m_devIPPort       = cfg.value("device/ipPort",m_devIPPort).toUInt();
    m_devSerialPort   = cfg.value("device/serialPort",m_devSerialPort).toString();
    m_devType         = cfg.value("device/type",m_devType).toString();
    m_devCharset      = cfg.value("device/charset",m_devCharset).toString();

    emit sigDeviceChanged();

    m_playSound       = cfg.value("device/playSound",m_playSound).toBool();
    IDevice::m_DownloadAllTrk   = cfg.value("device/dnlTrk",IDevice::m_DownloadAllTrk).toBool();
    IDevice::m_DownloadAllWpt   = cfg.value("device/dnlWpt",IDevice::m_DownloadAllWpt).toBool();
    IDevice::m_DownloadAllRte   = cfg.value("device/dnlRte",IDevice::m_DownloadAllRte).toBool();
    IDevice::m_UploadAllWpt     = cfg.value("device/uplWpt",IDevice::m_UploadAllWpt).toBool();
    IDevice::m_UploadAllTrk     = cfg.value("device/uplTrk",IDevice::m_UploadAllTrk).toBool();
    IDevice::m_UploadAllRte     = cfg.value("device/uplRte",IDevice::m_UploadAllRte).toBool();

    pathMaps        = cfg.value("path/maps",pathMaps).toString();

    QString unittype = cfg.value("environment/unittype","metric").toString();
    if(unittype == "metric")
        unit = new CUnitMetric(this);
    else if(unittype == "nautic")
    {
        unit = new CUnitNautic(this);
    }
    else if(unittype == "imperial")
    {
        unit = new CUnitImperial(this);
    }
    else
    {
        qWarning("Unknown unit type. Using 'metric'");
        unit = new CUnitMetric(this);
    }
}


CResources::~CResources()
{
    QSettings cfg;

    cfg.setValue("environment/mapfont/family",m_mapfont.family());
    cfg.setValue("environment/mapfont/size",m_mapfont.pointSize());
    cfg.setValue("environment/mapfont/bold",m_mapfont.bold());
    cfg.setValue("environment/mapfont/italic",m_mapfont.italic());

    cfg.setValue("environment/flipMouseWheel",m_flipMouseWheel);

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
    cfg.setValue("device/charset",m_devCharset);

    cfg.setValue("device/dnlTrk",IDevice::m_DownloadAllTrk);
    cfg.setValue("device/dnlWpt",IDevice::m_DownloadAllWpt);
    cfg.setValue("device/dnlRte",IDevice::m_DownloadAllRte);
    cfg.setValue("device/uplWpt",IDevice::m_UploadAllWpt);
    cfg.setValue("device/uplTrk",IDevice::m_UploadAllTrk);
    cfg.setValue("device/uplRte",IDevice::m_UploadAllRte);
    cfg.setValue("device/playSound",m_playSound);

    cfg.setValue("environment/unittype",unit->type);
}


bool CResources::getHttpProxy(QString& url, quint16& port)
{

    const char *proxy;

    // unless a manual proxy is configured, use the environment settings "HTTP_PROXY" or "http_proxy"
    if (!m_useHttpProxy && ((proxy = getenv("HTTP_PROXY")) || (proxy = getenv("http_proxy"))))
    {
        QString theProxy(proxy);
        QRegExp re("^http://([^:]+):(\\d+)$", Qt::CaseInsensitive);
        if (re.indexIn(theProxy) != -1)
        {
            qDebug() << "http proxy host" << re.cap(1) << "port" << re.cap(2);
            url = re.cap(1);
            port = re.cap(2).toInt();
            return true;
        }
    }

    url  = m_httpProxy;
    port = m_httpProxyPort;
    return m_useHttpProxy;
}


IDevice * CResources::device()
{
    // purge device if the key does not match
    if(m_device && (m_device->getDevKey() != m_devKey))
    {
        qDebug() << m_device->getDevKey() << m_devKey;
        delete m_device;
        m_device = 0;
    }

    // allocate new device
    if(!m_device)
    {
        if(m_devKey == "QLandkarteM")
        {
            //m_device = new CDeviceTBDOE(m_devIPAddress,m_devIPPort,this);
            m_device = new CDeviceQLandkarteM(m_devIPAddress,m_devIPPort,this);
        }
#ifdef HS_MIKROKOPTER
        if(m_devKey == "Mikrokopter")
        {
            m_device = new CDeviceMikrokopter(m_devSerialPort, this);
        }
#endif
        else if(m_devKey == "Garmin" && !m_devType.isEmpty())
        {
            m_device = new CDeviceGarmin(m_devType, m_devSerialPort, this);
        }
        else if(m_devKey == "NMEA")
        {
            m_device = new CDeviceNMEA(m_devSerialPort, this);
        }
#ifdef HAS_GPSD
        else if(m_devKey == "GPSD")
        {
            m_device = new CDeviceGPSD(this);
        }
#endif

        connect(m_device, SIGNAL(sigLiveLog(const CLiveLog&)), &CLiveLogDB::self(), SLOT(slotLiveLog(const CLiveLog&)));
    }

    // still no device?
    if(!m_device)
    {
        qWarning() << "no device";
        // TODO: would be nicer to open the setup dialog
        QMessageBox::critical(0,tr("No device."),tr("You have to select a device in Setup->Config->Device & Xfer"), QMessageBox::Abort, QMessageBox::Abort);
    }

    return m_device;
}


QString CResources::charset()
{
    if (m_devCharset.isNull() || m_devCharset.isEmpty())
        return "latin1";
    else
        return m_devCharset;
}


void CResources::openLink(const QString& link)
{
    QString cmd;
    if(m_eBrowser == eFirefox)
    {
        cmd.sprintf(cmdFirefox.toAscii(),link.toAscii().data());
    }
    else if(m_eBrowser == eKonqueror)
    {
        cmd.sprintf(cmdKonqueror.toAscii(),link.toAscii().data());
    }
    else if(m_eBrowser == eOther)
    {
        cmd.sprintf(cmdOther.toAscii(),link.toAscii().data());
    }
    else
    {
        return;
    }

    system(cmd.toAscii());

}
