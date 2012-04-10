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


#include "CMainWindow.h"
#include "CCanvas.h"
#include "CDeviceTwoNav.h"
#include "CSettings.h"
#include "CGpx.h"
#include "CWptDB.h"
#include "GeoMath.h"
#include "CResources.h"

#include <QtGui>

CDeviceTwoNav::CDeviceTwoNav(QObject *parent)
: IDevice("TwoNav", parent)
{

}

CDeviceTwoNav::~CDeviceTwoNav()
{

}

bool CDeviceTwoNav::aquire(QDir& dir)
{
    SETTINGS;
    QString path = cfg.value("device/path","").toString();
    dir.setPath(path);

    pathRoot = "";
    pathData = "TwoNavData/Data";

    if(!dir.exists() || !dir.exists(pathData))
    {
        while(1)
        {
            pathRoot = "";
            pathData = "TwoNavData/Data";

            path = QFileDialog::getExistingDirectory(0, tr("Path to TwoNav device..."), dir.absolutePath());
            if(path.isEmpty())
            {
                return false;
            }
            dir.setPath(path);

            if(!dir.exists("TwoNavData/Data"))
            {
                QMessageBox::information(0,tr("Error..."), tr("I need a path with 'TwoNavData/Data' as subdirectory"),QMessageBox::Retry,QMessageBox::Retry);
                continue;
            }

            pathData = dir.absoluteFilePath("TwoNavData/Data");

            break;
        }
    }

    pathRoot = dir.absolutePath();
    cfg.setValue("device/path", pathRoot);
    return true;
}

void CDeviceTwoNav::uploadWpts(const QList<CWpt*>& wpts)
{
//    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload wapoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);

    QDir dir;
    if(!aquire(dir))
    {
        return;
    }

    dir.cd(pathData);

    QString pathDay = dir.absoluteFilePath(QString("Data_%1").arg(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd")));
    if(!dir.exists(pathDay))
    {
        dir.mkpath(pathDay);
    }

    dir.cd(pathDay);

    QFile file(dir.absoluteFilePath("Waypoints.wpt"));
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    out << "B  UTF-8" << endl;
    out << "G  WGS 84" << endl;
    out << "U  1" << endl;
    foreach(CWpt * wpt, wpts)
    {
        QString name    = wpt->getName();
        name    = name.replace(" ","_");

        QString comment = wpt->getComment();
        if(comment.isEmpty())
        {
            comment = wpt->getDescription();
        }
        IItem::removeHtml(comment);
        comment = comment.replace("\n","%0A%0D");

        QStringList list;
        list << "W";
        list << name.replace(" ", "_");
        list << "A";
        list << (wpt->lat > 0 ? QString("%1\272N") : QString("%1\272S")).arg(wpt->lat,0,'f');
        list << (wpt->lon > 0 ? QString("%1\272E") : QString("%1\272W")).arg(wpt->lon,0,'f');
        list << QDateTime::fromTime_t(wpt->timestamp).toString("dd-MMM-yyyy");
        list << QDateTime::fromTime_t(wpt->timestamp).toString("hh:mm:ss");
        list << QString("%1").arg(wpt->ele == WPT_NOFLOAT ? 0 : wpt->ele,0,'f');

        out << list.join(" ") << " ";
        out << comment << endl;

        list.clear();
        list << wpt->getIconString().replace(",","").replace(" ", "_");
        list << "0"; //test position
        list << "-1.0";
        list << "0";
        list << QString("%1").arg(CResources::self().wptTextColor().value());
        list << "1";
        list << "37"; // 1 Name 2 Beschreibung 4 Symbol 8 Höhe 16 URL 32 Radius
        list << wpt->link;
        list << QString("%1").arg(wpt->prx == WPT_NOFLOAT ? 0 : wpt->prx,0,'f');
        list << wpt->getKey();

        out << "w ";
        out << list.join(",");
        out << endl;

    }

    file.close();
    dir.cd(pathRoot);
    theMainWindow->getCanvas()->setFadingMessage(tr("Upload waypoints finished!"));
}

void CDeviceTwoNav::downloadWpts(QList<CWpt*>& wpts)
{
//    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download wapoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);

    QStringList files;
    QStringList subdirs;
    QDir dir;
    if(!aquire(dir))
    {
        return;
    }

    dir.cd(pathData);
    subdirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach(const QString& subdir, subdirs)
    {
        dir.cd(subdir);

        files = dir.entryList(QStringList("*gpx"));
        foreach(const QString& filename, files)
        {
            CGpx gpx(this, CGpx::eCleanExport);
            gpx.load(dir.absoluteFilePath(filename));
            CWptDB::self().loadGPX(gpx);
        }

        files = dir.entryList(QStringList("*wpt"));
        foreach(const QString& filename, files)
        {
            readWptFile(dir.absoluteFilePath(filename), wpts);
        }

        dir.cdUp();
    }

    dir.cd(pathRoot);
    theMainWindow->getCanvas()->setFadingMessage(tr("Download waypoints finished!"));
}

void CDeviceTwoNav::uploadTracks(const QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadTracks(QList<CTrack*>& trks)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::uploadRoutes(const QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadRoutes(QList<CRoute*>& rtes)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::uploadMap(const QList<IMapSelection*>& mss)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceTwoNav::downloadScreenshot(QImage& image)
{
    QMessageBox::information(0,tr("Error..."), tr("TwoNav: Download screenshots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

struct wpt_t
{
    wpt_t(): lon(0), lat(0), ele(0), prox(0){}
    QDateTime time;
    QString name;
    QString comment;
    QString symbol;
    QString key;
    QString url;

    float lon;
    float lat;
    float ele;
    float prox;

};


void CDeviceTwoNav::readWptFile(const QString& filename, QList<CWpt*>& wpts)
{
    wpt_t tmpwpt;
    QString line("start");
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);

    while(!line.isEmpty())
    {
        line = in.readLine();

        switch(line[0].toAscii())
        {
        case 'B':
        {
            QString name        = line.mid(1).simplified();
            QTextCodec * codec  = QTextCodec::codecForName(name.toAscii());
            if(codec)
            {
                in.setCodec(codec);
            }
            break;
        }
        case 'G':
        {
            QString name  = line.mid(1).simplified();
            if(name != "WGS 84")
            {
                QMessageBox::information(0,tr("Error..."), tr("Only support lon/lat WGS 84 format."),QMessageBox::Abort,QMessageBox::Abort);
                return;
            }
            break;
        }
        case 'U':
        {
            QString name  = line.mid(1).simplified();
            if(name != "1")
            {
                QMessageBox::information(0,tr("Error..."), tr("Only support lon/lat WGS 84 format."),QMessageBox::Abort,QMessageBox::Abort);
                return;
            }
            break;
        }
        case 'W':
        {
            tmpwpt = wpt_t();
            QStringList values = line.split(' ', QString::SkipEmptyParts);

            tmpwpt.name    = values[1];
            GPS_Math_Str_To_Deg(values[3].replace("\272","") + " " + values[4].replace("\272",""), tmpwpt.lon, tmpwpt.lat);
            tmpwpt.time    = QDateTime::fromString(values[5] + " " + values[6], "dd-MMM-yyyy hh:mm:ss");
            tmpwpt.time.setTimeSpec(Qt::UTC);
            tmpwpt.ele     = values[7].toFloat();

            if(values.size() > 7)
            {
                QStringList list = values.mid(8);
                tmpwpt.comment = list.join(" ");
            }

            break;
        }
        case 'w':
        {
            QStringList values = line.mid(1).simplified().split(',', QString::KeepEmptyParts);

            tmpwpt.symbol  = values[0];

            tmpwpt.url     = values[7];
            tmpwpt.prox    = values[8].toFloat();
            tmpwpt.key     = values[9];


            if(tmpwpt.prox == 0)
            {
                tmpwpt.prox = WPT_NOFLOAT;
            }
            if(tmpwpt.ele == 0)
            {
                tmpwpt.ele = WPT_NOFLOAT;
            }
            if(tmpwpt.key == "0")
            {
                tmpwpt.key.clear();
            }

            tmpwpt.name = tmpwpt.name.replace("_", " ");

            CWpt * wpt = new CWpt(&CWptDB::self());

            wpt->setName(tmpwpt.name);
            wpt->setComment(tmpwpt.comment);
            wpt->setIcon(tmpwpt.symbol);
            wpt->setKey(tmpwpt.key);
            wpt->lon        = tmpwpt.lon;
            wpt->lat        = tmpwpt.lat;
            wpt->ele        = tmpwpt.ele;
            wpt->prx        = tmpwpt.prox;
            wpt->urlname    = tmpwpt.url;
            wpt->timestamp  = tmpwpt.time.toTime_t();

            wpts << wpt;

            qDebug() << tmpwpt.name << tmpwpt.symbol << tmpwpt.time << tmpwpt.lon << tmpwpt.lat << tmpwpt.ele << tmpwpt.prox << tmpwpt.comment << tmpwpt.key;
            break;
        }
        }
    }

}
