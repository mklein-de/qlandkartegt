/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgMapJNXConfig.h"
#include "CMapJnx.h"

#include <QtGui>

#ifndef WIN32
#include <pwd.h>
#endif

const QString CDlgMapJNXConfig::text =  QObject::tr(""
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>"
"<html>"
"   <head>"
"       <META HTTP-EQUIV='CACHE-CONTROL'' CONTENT='NO-CACHE'>"
"       <meta http-equiv='Content-Typ' content='text/html; charset=utf-8'>"
"       <style type='text/css'>"
"           p, li { white-space: pre-wrap; }"
"           td {padding-top: 10px;}"
"           th {background-color: lightBlue;}"
"       </style>"
"   </head>"
"   <body style=' font-family:'Sans'; font-size:9pt; font-weight:400; font-style:normal;'>"
"       <p>${info}</p>"
"   </body>"
"</html>"
"");

CDlgMapJNXConfig::CDlgMapJNXConfig(CMapJnx * map)
: map(map)
{
    setupUi(this);

    QDir tempDir;
#ifndef Q_OS_WIN32
    const char *envCache = getenv("QLGT_LEGEND");

    if (envCache)
    {
        tempDir = envCache;
    }
    else
    {
        struct passwd * userInfo = getpwuid(getuid());
        tempDir = QDir::tempPath() + "/qlandkarteqt-" + userInfo->pw_name + "/legend/";
    }
#else
    tempDir = QDir::tempPath() + "/qlandkarteqt/cache/";
#endif
    tempDir.mkpath(tempDir.path());

    QString cpytext = text;
    cpytext = cpytext.replace("${info}", map->getMapInfo());

    webView->page()->settings()->clearMemoryCaches();

    QFile tmp(tempDir.filePath("legend.html"));
    tmp.open(QIODevice::WriteOnly);
    tmp.write(cpytext.toUtf8());
    tmp.close();

    //webView->load(QUrl::fromLocalFile(tmp.fileName()));
    webView->setHtml(cpytext);

    connect(this, SIGNAL(accepted()), SLOT(deleteLater()));
    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));
}


CDlgMapJNXConfig::~CDlgMapJNXConfig()
{
    qDebug() << "CDlgMapJNXConfig::~CDlgMapJNXConfig()";
}


void CDlgMapJNXConfig::accept()
{

    QDialog::accept();
}
