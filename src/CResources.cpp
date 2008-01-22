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

#include <QtGui>


CResources * CResources::m_self = 0;

CResources::CResources(QObject * parent)
    : QObject(parent)
    , m_useHttpProxy(false)
    , m_httpProxyPort(8080)
    , m_eBrowser(eFirefox)
    , cmdFirefox("firefox \"%s\" &")
    , cmdKonqueror("kfmclient exec \"%s\"")
    , time_offset(0)

{
    m_self = this;

    QSettings cfg;

    QString family  = cfg.value("environment/mapfont/family","Arial").toString();
    int size        = cfg.value("environment/mapfont/size",8).toInt();
    m_mapfont = QFont(family,size);

    m_doMetric        = cfg.value("environment/doMetric",true).toBool();
    m_offsetUTC       = cfg.value("environment/UTCOffset",100).toInt();
    m_offsetUTCfract  = cfg.value("environment/UTCOffsetFract",0).toInt();

    if(m_offsetUTC == 100){
        QMessageBox::information(0,tr("Time offset ...")
                                ,tr("QLandkarte assumes that your hardware clock is "
                                    "set to UTC time. You must setup the correct "
                                    "time offset in 'Setup->Config'")
                                ,QMessageBox::Ok,QMessageBox::NoButton);
    }
    else{
        setUTCOffset(m_offsetUTC, m_offsetUTCfract);
    }

    m_useHttpProxy    = cfg.value("network/useProxy",m_useHttpProxy).toBool();
    m_httpProxy       = cfg.value("network/proxy/url",m_httpProxy).toString();
    m_httpProxyPort   = cfg.value("network/proxy/port",m_httpProxyPort).toUInt();

    m_eBrowser        = (bowser_e)cfg.value("network/browser",m_eBrowser).toInt();
    cmdOther          = cfg.value("network/browser/other","my command \"%s\"").toString();

    emit sigProxyChanged();

    m_device = new CDeviceTBDOE(this);
}

CResources::~CResources()
{
    QSettings cfg;

    cfg.setValue("environment/mapfont/family",m_mapfont.family());
    cfg.setValue("environment/mapfont/size",m_mapfont.pointSize());
    cfg.setValue("environment/doMetric",m_doMetric);
    cfg.setValue("environment/UTCOffset",m_offsetUTC);
    cfg.setValue("environment/UTCOffsetFract",m_offsetUTCfract);

    cfg.setValue("network/useProxy",m_useHttpProxy);
    cfg.setValue("network/proxy/url",m_httpProxy);
    cfg.setValue("network/proxy/port",m_httpProxyPort);

    cfg.setValue("network/browser",m_eBrowser);
    cfg.setValue("network/browser/other",cmdOther);

}

bool CResources::getHttpProxy(QString& url, quint16& port)
{
    url  = m_httpProxy;
    port = m_httpProxyPort;
    return m_useHttpProxy;
}

void CResources::setUTCOffset(int offset, int fract)
{
//     time_offset = TIME_OFFSET + offset * 3600 + fract * 60;
}
