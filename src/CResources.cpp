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

#include <QtGui>

CResources * CResources::m_self = 0;

CResources::CResources(QObject * parent)
    : QObject(parent)
    , m_useHttpProxy(false)
    , m_httpProxyPort(8080)

{
    m_self = this;

    QSettings cfg;

    m_useHttpProxy    = cfg.value("network/useProxy",m_useHttpProxy).toBool();
    m_httpProxy       = cfg.value("network/proxy/url",m_httpProxy).toString();
    m_httpProxyPort   = cfg.value("network/proxy/port",m_httpProxyPort).toUInt();

}

CResources::~CResources()
{
    QSettings cfg;

    cfg.setValue("network/useProxy",m_useHttpProxy);
    cfg.setValue("network/proxy/url",m_httpProxy);
    cfg.setValue("network/proxy/port",m_httpProxyPort);

}

bool CResources::getHttpProxy(QString& url, quint16& port)
{
    url  = m_httpProxy;
    port = m_httpProxyPort;
    return m_useHttpProxy;
}

