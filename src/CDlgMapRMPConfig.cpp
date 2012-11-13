/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgMapRMPConfig.cpp

  Module:      

  Description:

  Created:     11/13/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgMapRMPConfig.h"
#include "CMapRmp.h"

#include <QtGui>

#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif


const QString CDlgMapRMPConfig::text =  QObject::tr(""
                                                    "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'  'http://www.w3.org/TR/html4/loose.dtd'>"
                                                    "<html>"
                                                    "   <head>"
                                                    "       <title></title>"
                                                    "       <META HTTP-EQUIV='CACHE-CONTROL' CONTENT='NO-CACHE'>"
                                                    "       <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                                                    "       <style type='text/css'>"
                                                    "           p, li { white-space: pre-wrap;}"
                                                    "           td {padding-top: 3px;}"
                                                    "           th {background-color: darkBlue; color: white;}"
                                                    "       </style>"
                                                    "   </head>"
                                                    "   <body style=' font-family: sans-serif; font-size: 9pt; font-weight:400; font-style:normal;'>"
                                                    "       <p>${info}</p>"
                                                    "   </body>"
                                                    "</html>"
                                                    "");


CDlgMapRMPConfig::CDlgMapRMPConfig(CMapRmp *map)
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

CDlgMapRMPConfig::~CDlgMapRMPConfig()
{

}

