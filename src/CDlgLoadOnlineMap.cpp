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

#include "CDlgLoadOnlineMap.h"
#include "CSettings.h"

#include <QtGui>
#include <QtNetwork>

#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif

const QString CDlgLoadOnlineMap::text =  QObject::tr(""
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'  'http://www.w3.org/TR/html4/loose.dtd'>"
"<html>"
"   <head>"
"       <title></title>"
"       <META HTTP-EQUIV='CACHE-CONTROL' CONTENT='NO-CACHE'>"
"       <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
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

CDlgLoadOnlineMap::CDlgLoadOnlineMap()
{
    setupUi(this);

#ifndef Q_OS_WIN32
    const char *envCache = getenv("QLGT_LEGEND");

    if (envCache)
    {
        tempDir = envCache;
    }
    else
    {
        struct passwd * userInfo = getpwuid(getuid());
        tempDir = QDir::homePath() + "/qlandkartegt-" + userInfo->pw_name + "/maps/";
    }
#else
    tempDir = QDir::homePath() + "/qlandkarteqt/cache/";
#endif
    tempDir.mkpath(tempDir.path());

    SETTINGS;
    QString wmsPath     = cfg.value("wmsMaps/path","").toString();
    if (!wmsPath.isEmpty())
    {
        tempDir.setPath(wmsPath);
    }

    wmsTargetPath->setText(tempDir.absolutePath());

    connect(&soapHttp, SIGNAL(responseReady(const QtSoapMessage &)),this, SLOT(slotWebServiceResponse(const QtSoapMessage &)));
    connect(wmsButtonPath, SIGNAL(clicked()),this,SLOT(slotTargetPath()));

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    getMapList();
}


void CDlgLoadOnlineMap::getMapList()
{
    QtSoapMessage request;
    request.setMethod(QtSoapQName("getwmsmaps", "urn:qlandkartegt"));
    request.addMethodArgument("folder", "", "");
    soapHttp.setHost("www.qlandkarte.org");
    soapHttp.submitRequest(request, "/webservice/qlandkartegt.php");
}

CDlgLoadOnlineMap::~CDlgLoadOnlineMap()
{
    QApplication::restoreOverrideCursor();
}


void CDlgLoadOnlineMap::accept()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<QListWidgetItem *> items = mapList->selectedItems();
    for (int i=0; i < items.count();i++)
    {
        QListWidgetItem *item = items.at(i);
        QtSoapMessage request;
        request.setMethod(QtSoapQName("getwmslink", "urn:qlandkartegt"));
        request.addMethodArgument("link", "", item->data(Qt::UserRole).toString());
        soapHttp.setHost("www.qlandkarte.org");
        soapHttp.submitRequest(request, "/webservice/qlandkartegt.php");
        selectedfile = "";
        selectedfile = selectedfile.prepend(tempDir.absolutePath()+"/");
        selectedfile += item->text() + ".xml";
        qDebug("Call Webservice Url %s",qPrintable(item->data(Qt::UserRole).toString()));
    }
}

bool CDlgLoadOnlineMap::saveToDisk(const QString &filename, QString data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data.toLocal8Bit());
    file.close();

    return true;
}

void CDlgLoadOnlineMap::slotWebServiceResponse(const QtSoapMessage &message)
{
    qDebug("Methode name response %s",qPrintable(message.method().name().name()));
    QString method(message.method().name().name());
    if (message.isFault())
    {
        qDebug("Error: %s", qPrintable(message.faultString().toString()));
    }
    else
    {
        if (method == "getwmsmapsResponse")
        {
                const QtSoapType &array = message.returnValue();
                QListWidgetItem *item;
                for (int i = 0; i < array.count(); ++i)
                {
                        const QtSoapType &map = array[i];
                        QString mapName(map["name"].toString().trimmed());
                        if (mapName.contains(QRegExp(".xml")))
                        {
                            mapName = mapName.replace(QRegExp(".xml"),"");
                            mapName = mapName.replace(QRegExp("_")," ");
                            item = new QListWidgetItem(QIcon(":/icons/iconWMS22x22.png"),mapName);
                            item->setData(Qt::UserRole,QUrl(map["link"].toString()));
                            mapList->addItem(item);
                        }
                }
                mapList->sortItems();
        }

        if (method == "getwmslinkResponse")
        {
            //qDebug("Event Download link triggered %s",qPrintable(message.returnValue().toString()));
            QString data(message.returnValue().toString());
            //data.replace(QRegExp("&amp;"), "&"); // This _must_ come first
            data.replace(QRegExp("&lt;"), "<");
            data.replace(QRegExp("&gt;"), ">");
            data.replace(QRegExp("&quot;"), "\"");

            qDebug("Write result to file %s",qPrintable(selectedfile));
            saveToDisk(selectedfile,data);
            QDialog::accept();
        }

        if (method == "getlastversionResponse")
        {

        }
    }
    QApplication::restoreOverrideCursor();
}

void CDlgLoadOnlineMap::slotTargetPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"), tempDir.absolutePath(), QFileDialog::ShowDirsOnly);
    if(!path.isEmpty())
    {
        tempDir.setPath(path);
        wmsTargetPath->setText(tempDir.absolutePath());
        SETTINGS;
        cfg.setValue("wmsMaps/path",path);
    }
}
