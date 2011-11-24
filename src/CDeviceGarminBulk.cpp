/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDeviceGarminBulk.cpp

  Module:

  Description:

  Created:     11/23/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDeviceGarminBulk.h"
#include "CGpx.h"
#include "CWptDB.h"
#include "CTrackDB.h"

#include <QtGui>

CDeviceGarminBulk::CDeviceGarminBulk(QObject * parent)
: IDevice("Garmin Mass Storage", parent)
{

}

CDeviceGarminBulk::~CDeviceGarminBulk()
{

}

bool CDeviceGarminBulk::aquire(QDir& dir)
{
    QSettings cfg;
    QString path = cfg.value("device/path","").toString();
    dir.setPath(path);

    if(!dir.exists() || dir.absolutePath() != path || !dir.exists("GPX") || !dir.exists("JPEG"))
    {
        while(1)
        {
            path = QFileDialog::getExistingDirectory(0, "Path to Garmin device...", dir.absolutePath());
            if(path.isEmpty())
            {
                return false;
            }
            dir.setPath(path);

            if(!dir.exists("GPX"))
            {
                QMessageBox::critical(0, tr("Missing..."), tr("The selected path must have a subdirectory 'GPX'."), QMessageBox::Abort, QMessageBox::Abort);
                continue;
            }

            if(!dir.exists("JPEG"))
            {
                QMessageBox::critical(0, tr("Missing..."), tr("The selected path must have a subdirectory 'JPEG'."), QMessageBox::Abort, QMessageBox::Abort);
                continue;
            }

            break;
        }
    }
    cfg.setValue("device/path", path);
    return true;
}

void CDeviceGarminBulk::uploadWpts(const QList<CWpt*>& wpts)
{
    //QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
    QDir dir;
    if(!aquire(dir))
    {
        return;
    }


    dir.cd("JPEG");

    QStringList keys;
    foreach(CWpt* wpt, wpts)
    {
        keys << wpt->getKey();
        if(!wpt->images.isEmpty())
        {
            CWpt::image_t img = wpt->images.first();
            img.pixmap.save(dir.absoluteFilePath(wpt->getName() + ".jpg"));
            wpt->link = "Garmin/JPEG/" + wpt->getName() + ".jpg";
        }
    }

    dir.cdUp();
    dir.cd("GPX");

    CGpx gpx(this, CGpx::eCleanExport);
    CWptDB::self().saveGPX(gpx, keys);
    //2008-04-28T20:38:19Z
    QString filename = QString("WPT_%1.gpx").arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd"));
    gpx.save(dir.absoluteFilePath(filename));

    dir.cdUp();

}


void CDeviceGarminBulk::downloadWpts(QList<CWpt*>& /*wpts*/)
{
    //QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download waypoints is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
    QDir dir;
    if(!aquire(dir))
    {
        return;
    }

    dir.cd("GPX");

    QStringList files = dir.entryList(QStringList("*gpx"));
    foreach(const QString& filename, files)
    {
        CGpx gpx(this, CGpx::eCleanExport);
        gpx.load(dir.absoluteFilePath(filename));
        CWptDB::self().loadGPX(gpx);
    }

    dir.cdUp();
    dir.cd("JPEG");

    const QMap<QString,CWpt*>& wpts = CWptDB::self().getWpts();
    foreach(CWpt * wpt, wpts)
    {
        if(wpt->link.startsWith("Garmin/JPEG/"))
        {
            CWpt::image_t img;
            img.pixmap.load(dir.absoluteFilePath(wpt->link.mid(12)));
            if(!img.pixmap.isNull())
            {
                wpt->images << img;
            }
        }
    }
}


void CDeviceGarminBulk::uploadTracks(const QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadTracks(QList<CTrack*>& /*trks*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download tracks is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::uploadRoutes(const QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadRoutes(QList<CRoute*>& /*rtes*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download routes is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::uploadMap(const QList<IMapSelection*>& /*mss*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Upload maps is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}


void CDeviceGarminBulk::downloadScreenshot(QImage& /*image*/)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Download screenshots is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}

void CDeviceGarminBulk::setLiveLog(bool on)
{
    QMessageBox::information(0,tr("Error..."), tr("Garmin Mass Storage: Live log is not implemented."),QMessageBox::Abort,QMessageBox::Abort);
}
