/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#include "CMapQMAPExport.h"
#include "CMapSelectionRaster.h"
#include "CMapDB.h"
#include "CMapFile.h"
#include "GeoMath.h"

#include "config.h"


#include <QtGui>
#include <QtXml/QDomDocument>
#include <qzipwriter.h>

CMapQMAPExport::CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent)
: QDialog(parent)
, mapsel(mapsel)
, file1(0)
, file2(0)
{
    setupUi(this);
    toolPath->setIcon(QPixmap(":/icons/iconFileLoad16x16.png"));

    QSettings cfg;
    labelPath->setText(cfg.value("path/export","./").toString());

    IMap& map = CMapDB::self().getMap();
    linePrefix->setText(QString("%1_%2_%3").arg(QFileInfo(map.getFilename()).baseName()).arg(mapsel.lon1 * RAD_TO_DEG).arg(mapsel.lat1 * RAD_TO_DEG));
    lineDescription->setText(QFileInfo(map.getFilename()).baseName());

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotOutputPath()));
    connect(pushExport, SIGNAL(clicked()), this, SLOT(slotStart()));

    connect(&cmd1, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd1, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd1, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished2(int,QProcess::ExitStatus)));

    connect(&cmd2, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd2, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd2, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished3(int,QProcess::ExitStatus)));

    connect(&cmd3, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd3, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd3, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished1(int,QProcess::ExitStatus)));

    connect(&cmdKMZ1, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmdKMZ1, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmdKMZ1, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinishedKMZ2(int,QProcess::ExitStatus)));

    connect(&cmdKMZ2, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmdKMZ2, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmdKMZ2, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinishedKMZ3(int,QProcess::ExitStatus)));

    radioQLM->setChecked(cfg.value("map/export/qlm", true).toBool());
    radioGE->setChecked(cfg.value("map/export/ge", false).toBool());
    radioGCM->setChecked(cfg.value("map/export/gcm", false).toBool());
}


CMapQMAPExport::~CMapQMAPExport()
{
    QSettings cfg;
    cfg.setValue("map/export/qlm", radioQLM->isChecked());
    cfg.setValue("map/export/ge", radioGE->isChecked());
    cfg.setValue("map/export/gcm", radioGCM->isChecked());
}


void CMapQMAPExport::slotOutputPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select ouput path..."), labelPath->text(), FILE_DIALOG_FLAGS);
    if(path.isEmpty()) return;

    QSettings cfg;
    cfg.setValue("path/export", path);
    labelPath->setText(path);
}


void CMapQMAPExport::slotStderr()
{
    textBrowser->setTextColor(Qt::red);
    if(sender() == &cmd1)
    {
        textBrowser->append(cmd1.readAllStandardError());
    }
    else if(sender() == &cmd2)
    {
        textBrowser->append(cmd2.readAllStandardError());
    }
    else if(sender() == &cmd3)
    {
        textBrowser->append(cmd3.readAllStandardError());
    }
}


void CMapQMAPExport::slotStdout()
{
    textBrowser->setTextColor(Qt::blue);
    QString str;
    if(sender() == &cmd1)
    {
        str = cmd1.readAllStandardOutput();
    }
    else if(sender() == &cmd2)
    {
        str = cmd2.readAllStandardOutput();
    }
    else if(sender() == &cmd3)
    {
        str = cmd3.readAllStandardOutput();
    }

    textBrowser->insertPlainText(str);
    textBrowser->verticalScrollBar()->setValue(textBrowser->verticalScrollBar()->maximum());
}

void CMapQMAPExport::slotStart()
{
    if(radioQLM->isChecked())
    {
        startQLM();
    }
    else if(radioGE->isChecked())
    {
        startGE();
    }
    else if(radioGCM->isChecked())
    {
        startGCM();
    }
}

void CMapQMAPExport::startQLM()
{
    // get map summary
    CMapDB::map_t& map = CMapDB::self().knownMaps[mapsel.mapkey];

    QString prefix = linePrefix->text();

    QDir srcPath = QFileInfo(map.filename).absolutePath();
    QDir tarPath(labelPath->text());

    QSettings srcdef(map.filename, QSettings::IniFormat);
    QSettings tardef(tarPath.filePath(QString("%1.qmap").arg(prefix)), QSettings::IniFormat);

    // [description]
    // bottomright=" N43\xb0 43.980 E004\xb0 12.380 "
    // comment=Vergeze IGN
    // height=22075.3633436567
    // topleft=" N43\xb0 55.901 E004\xb0 14.722 "
    // width=3134.92507596005
    //
    // [home]
    // center=N43 47.589 E004 10.133
    // zoom=2
    tardef.beginGroup("description");
    QString pos;
    GPS_Math_Deg_To_Str(mapsel.lon1 * RAD_TO_DEG, mapsel.lat1 * RAD_TO_DEG, pos);
    tardef.setValue("topleft",pos);
    GPS_Math_Deg_To_Str(mapsel.lon2 * RAD_TO_DEG, mapsel.lat2 * RAD_TO_DEG, pos);
    tardef.setValue("bottomright",pos);
    tardef.setValue("comment",lineDescription->text());
    tardef.endGroup();

    tardef.beginGroup("home");

    float lon, lat;
    lon = mapsel.lon1 + (mapsel.lon2 - mapsel.lon1) / 2;
    lat = mapsel.lat1 + (mapsel.lat2 - mapsel.lat1) / 2;
    GPS_Math_Deg_To_Str(lon * RAD_TO_DEG, lat * RAD_TO_DEG, pos);
    tardef.setValue("center",pos);
    tardef.setValue("zoom",1);
    tardef.endGroup();

    int idx = 0;
    int levels = srcdef.value("main/levels",0).toInt();
    tardef.setValue("main/levels",levels);

    for(int level = 1; level <= levels; ++level)
    {
        QStringList outfiles;
        QStringList filenames = srcdef.value(QString("level%1/files").arg(level),"").toString().split("|", QString::SkipEmptyParts);
        QString filename;
        foreach(filename, filenames)
        {
            CMapFile * mapfile = new CMapFile(srcPath.filePath(filename), this);
            if(!mapfile->ok)
            {
                delete mapfile;
                QMessageBox::critical(0,tr("Error ..."), tr("Failed to read %1").arg(filename), QMessageBox::Abort,  QMessageBox::Abort);
                return QDialog::reject();
            }

            PJ * pjWGS84 = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");
            XY p1,p2;
            p1.u = mapsel.lon1;
            p1.v = mapsel.lat1;
            pj_transform(pjWGS84,mapfile->pj,1,0,&p1.u,&p1.v,0);

            p2.u = mapsel.lon2;
            p2.v = mapsel.lat2;
            pj_transform(pjWGS84,mapfile->pj,1,0,&p2.u,&p2.v,0);
            pj_free(pjWGS84);

            QRectF maparea(QPointF(mapfile->xref1, mapfile->yref1), QPointF(mapfile->xref2, mapfile->yref2));
            QRectF selarea(QPointF(p1.u, p1.v), QPointF(p2.u, p2.v));
            QRect  intersect = selarea.intersected(maparea).toRect();

            //             qDebug() << maparea << selarea << intersect;
            if(intersect.isValid())
            {
                job_t job;
                job.idx    = idx++;
                job.srcFilename = mapfile->filename;
                job.tarFilename = tarPath.filePath(QString("%1_%2.tif").arg(prefix).arg(job.idx));
                job.xoff   = (intersect.left()   - mapfile->xref1) / mapfile->xscale;
                job.yoff   = (intersect.bottom() - mapfile->yref1) / mapfile->yscale;
                job.width  =  intersect.width()  / mapfile->xscale;
                job.height = -intersect.height() / mapfile->yscale;

                //                 qDebug() << "xoff: 0 <" << job.xoff;
                //                 qDebug() << "yoff: 0 <" << job.yoff;
                //                 qDebug() << "x2  :    " << (job.xoff + job.width)  << " <" << mapfile->xsize_px;
                //                 qDebug() << "y2  :    " << (job.yoff + job.height) << " <" << mapfile->ysize_px;

                jobs        << job;
                outfiles    << tarPath.relativeFilePath(job.tarFilename);
            }
            tardef.setValue(QString("level%1/files").arg(level), outfiles.join("|"));
            tardef.setValue(QString("level%1/zoomLevelMin").arg(level), srcdef.value(QString("level%1/zoomLevelMin").arg(level)));
            tardef.setValue(QString("level%1/zoomLevelMax").arg(level), srcdef.value(QString("level%1/zoomLevelMax").arg(level)));
            delete mapfile;
        }
    }

    slotFinished1(0,QProcess::NormalExit);
}


void CMapQMAPExport::slotFinished1( int exitCode, QProcess::ExitStatus status)
{
    //     qDebug() << exitCode << status;
    if(file1){delete file1; file1 = 0;}
    if(file2){delete file2; file2 = 0;}
    if(jobs.isEmpty())
    {
        textBrowser->setTextColor(Qt::black);
        textBrowser->append(tr("--- finished ---\n"));
        return;
    }

    file1 = new QTemporaryFile();
    file1->open();
    file2 = new QTemporaryFile();
    file2->open();

    job_t job = jobs.first();
    QStringList args;
    args << "-srcwin";
    args << QString::number(job.xoff) << QString::number(job.yoff);
    args << QString::number(job.width) << QString::number(job.height);
    args << job.srcFilename;
    args << file1->fileName();

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALTRANSLATE " " +  args.join(" ") + "\n");

    cmd1.start(GDALTRANSLATE, args);

}


void CMapQMAPExport::slotFinished2( int exitCode, QProcess::ExitStatus status)
{
    job_t job = jobs.first();
    QStringList args;
    args << "-t_srs" << "EPSG:4326";
    args << "-ts"    << QString::number(job.width) << QString::number(job.height);
    args << file1->fileName();
    args << file2->fileName();

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALWARP " " +  args.join(" ") + "\n");

    cmd2.start(GDALWARP, args);
}


void CMapQMAPExport::slotFinished3( int exitCode, QProcess::ExitStatus status)
{
    job_t job = jobs.takeFirst();
    QStringList args;
    args << "-co" << "tiled=yes";
    args << "-co" << "blockxsize=256";
    args << "-co" << "blockysize=256";
    args << "-co" << "compress=LZW";
    args << file2->fileName();
    args << job.tarFilename;

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALTRANSLATE " " +  args.join(" ") + "\n");

    cmd3.start(GDALTRANSLATE, args);
}

void CMapQMAPExport::startGE()
{
    // get map summary
    CMapDB::map_t& map = CMapDB::self().knownMaps[mapsel.mapkey];

    QString prefix = linePrefix->text();

    QDir srcPath = QFileInfo(map.filename).absolutePath();
    QDir tarPath(labelPath->text());

    QSettings srcdef(map.filename, QSettings::IniFormat);

    int idx = 0;
    int level = 1;

    QStringList filenames = srcdef.value(QString("level%1/files").arg(level),"").toString().split("|", QString::SkipEmptyParts);
    QString filename;
    foreach(filename, filenames)
    {
        CMapFile * mapfile = new CMapFile(srcPath.filePath(filename), this);
        if(!mapfile->ok)
        {
            delete mapfile;
            QMessageBox::critical(0,tr("Error ..."), tr("Failed to read %1").arg(filename), QMessageBox::Abort,  QMessageBox::Abort);
            return QDialog::reject();
        }

        PJ * pjWGS84 = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");
        XY p1,p2;
        p1.u = mapsel.lon1;
        p1.v = mapsel.lat1;
        pj_transform(pjWGS84,mapfile->pj,1,0,&p1.u,&p1.v,0);

        p2.u = mapsel.lon2;
        p2.v = mapsel.lat2;
        pj_transform(pjWGS84,mapfile->pj,1,0,&p2.u,&p2.v,0);
        pj_free(pjWGS84);

        QRectF maparea(QPointF(mapfile->xref1, mapfile->yref1), QPointF(mapfile->xref2, mapfile->yref2));
        QRectF selarea(QPointF(p1.u, p1.v), QPointF(p2.u, p2.v));
        QRect  intersect = selarea.intersected(maparea).toRect();

        //             qDebug() << maparea << selarea << intersect;
        if(intersect.isValid())
        {
            job_t job;

            job.name    = QString("%1 %2").arg(lineDescription->text()).arg(job.idx);
            job.idx     = idx++;
            job.srcFilename = mapfile->filename;
            job.tarFilename = tarPath.filePath(QString("%1_%2.kmz").arg(prefix).arg(job.idx));
            job.xoff    = (intersect.left()   - mapfile->xref1) / mapfile->xscale;
            job.yoff    = (intersect.bottom() - mapfile->yref1) / mapfile->yscale;
            job.width   =  intersect.width()  / mapfile->xscale;
            job.height  = -intersect.height() / mapfile->yscale;

            job.p1.u    = mapsel.lon1;
            job.p1.v    = mapsel.lat1;
            job.p2.u    = mapsel.lon2;
            job.p2.v    = mapsel.lat2;

            jobs        << job;
        }
        delete mapfile;
    }

    slotFinishedKMZ1(0,QProcess::NormalExit);
}

void CMapQMAPExport::slotFinishedKMZ1( int exitCode, QProcess::ExitStatus status)
{
    if(file1){delete file1; file1 = 0;}
    if(file2){delete file2; file2 = 0;}
    if(jobs.isEmpty())
    {
        textBrowser->setTextColor(Qt::black);
        textBrowser->append(tr("--- finished ---\n"));
        return;
    }

    file1 = new QTemporaryFile();
    file1->open();
    file2 = new QTemporaryFile();
    file2->open();

    job_t job = jobs.first();
    QStringList args;
    args << "-srcwin";
    args << QString::number(job.xoff) << QString::number(job.yoff);
    args << QString::number(job.width) << QString::number(job.height);
    args << job.srcFilename;
    args << file1->fileName();

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALTRANSLATE " " +  args.join(" ") + "\n");

    cmdKMZ1.start(GDALTRANSLATE, args);

}

void CMapQMAPExport::slotFinishedKMZ2( int exitCode, QProcess::ExitStatus status)
{
    job_t job = jobs.first();

    QStringList args;
    args << "-t_srs" << "EPSG:3785";
    args << file1->fileName();
    args << file2->fileName();

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALWARP " " +  args.join(" ") + "\n");

    cmdKMZ2.start(GDALWARP, args);
}


void CMapQMAPExport::slotFinishedKMZ3( int exitCode, QProcess::ExitStatus status)
{
    job_t job = jobs.takeFirst();

    textBrowser->setTextColor(Qt::black);
    textBrowser->append(tr("Compress data to  %1\n").arg(job.tarFilename));

    QString str;
    QFile   zipfile(job.tarFilename);
    QImage  img(file2->fileName());
    QString mapfilename = QDir::temp().filePath("map.jpg");
    QFile   mapfile(mapfilename);
    QZipWriter zip(&zipfile);
    QDomDocument doc;
    QDomElement root    = doc.createElement("kml");

    doc.appendChild(root);
    root.setAttribute("xmlns","http://www.opengis.net/kml/2.2");

    QDomElement overlay = doc.createElement("GroundOverlay");
    root.appendChild(overlay);

    QDomElement name = doc.createElement("name");
    overlay.appendChild(name);
    name.appendChild(doc.createTextNode(job.name));

    QDomElement icon = doc.createElement("Icon");
    overlay.appendChild(icon);

    QDomElement href = doc.createElement("href");
    icon.appendChild(href);
    href.appendChild(doc.createTextNode("files/map.jpg"));

    QDomElement drawOrder = doc.createElement("drawOrder");
    icon.appendChild(drawOrder);
    drawOrder.appendChild(doc.createTextNode("50"));

    QDomElement latLonBox = doc.createElement("LatLonBox");
    overlay.appendChild(latLonBox);

    QDomElement north = doc.createElement("north");
    latLonBox.appendChild(north);
    str.sprintf("%1.8f", job.p1.v * RAD_TO_DEG);
    north.appendChild(doc.createTextNode(str));

    QDomElement south = doc.createElement("south");
    latLonBox.appendChild(south);
    str.sprintf("%1.8f", job.p2.v * RAD_TO_DEG);
    south.appendChild(doc.createTextNode(str));

    QDomElement east = doc.createElement("east");
    latLonBox.appendChild(east);
    str.sprintf("%1.8f", job.p2.u * RAD_TO_DEG);
    east.appendChild(doc.createTextNode(str));

    QDomElement west = doc.createElement("west");
    latLonBox.appendChild(west);
    str.sprintf("%1.8f", job.p1.u * RAD_TO_DEG);
    west.appendChild(doc.createTextNode(str));

    QDomElement rotation = doc.createElement("rotation");
    latLonBox.appendChild(rotation);
    rotation.appendChild(doc.createTextNode("0.0"));

    img.save(mapfilename);
    mapfile.open(QIODevice::ReadOnly);

    zipfile.open(QIODevice::WriteOnly);
    zip.addDirectory("files");
    zip.addFile("files/map.jpg", &mapfile);
    zip.addFile("doc.kml",QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n") + doc.toByteArray());

    zip.close();

    slotFinishedKMZ1(0,QProcess::NormalExit);
}

void CMapQMAPExport::startGCM()
{
    // get map summary
    CMapDB::map_t& mapdef = CMapDB::self().knownMaps[mapsel.mapkey];
    PJ * pjWGS84 = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");

    QString prefix = linePrefix->text();

    QDir srcPath = QFileInfo(mapdef.filename).absolutePath();
    QDir tarPath(labelPath->text());

    QSettings srcdef(mapdef.filename, QSettings::IniFormat);

    int idx = 0;
    int level = 1;

    QStringList filenames = srcdef.value(QString("level%1/files").arg(level),"").toString().split("|", QString::SkipEmptyParts);
    QString filename;
    foreach(filename, filenames)
    {
        CMapFile * mapfile = new CMapFile(srcPath.filePath(filename), this);
        if(!mapfile->ok)
        {
            delete mapfile;
            QMessageBox::critical(0,tr("Error ..."), tr("Failed to read %1").arg(filename), QMessageBox::Abort,  QMessageBox::Abort);
            return QDialog::reject();
        }

        XY p1,p2;
        p1.u = mapsel.lon1;
        p1.v = mapsel.lat1;
        pj_transform(pjWGS84,mapfile->pj,1,0,&p1.u,&p1.v,0);

        p2.u = mapsel.lon2;
        p2.v = mapsel.lat2;
        pj_transform(pjWGS84,mapfile->pj,1,0,&p2.u,&p2.v,0);

        double dU = 1024 * mapfile->xscale;
        double dV = 1024 * mapfile->yscale;

        for(double v = p1.v; v >= p2.v; v += dV)
        {
            for(double u = p1.u; u < p2.u; u += dU)
            {

                double dU1 = dU;
                double dV1 = dV;
                if(abs(p2.u - u) < abs(dU)) dU1 = (p2.u - u);
                if(abs(p2.v - v) < abs(dV)) dV1 = (p2.v - v);

                QRectF maparea(QPointF(mapfile->xref1, mapfile->yref1), QPointF(mapfile->xref2, mapfile->yref2));
                QRectF selarea(QPointF(u, v), QPointF(u + dU1, v + dV1));
                QRect  intersect = selarea.intersected(maparea).toRect();

                //             qDebug() << maparea << selarea << intersect;
                if(intersect.isValid())
                {
                    job_t job;
                    job.name    = QString("%1 %2").arg(lineDescription->text()).arg(job.idx);
                    job.idx     = idx++;
                    job.srcFilename = mapfile->filename;
                    job.tarFilename = tarPath.filePath(QString("%1_%2.kmz").arg(prefix).arg(job.idx));
                    job.xoff    = (intersect.left()   - mapfile->xref1) / mapfile->xscale;
                    job.yoff    = (intersect.bottom() - mapfile->yref1) / mapfile->yscale;
                    job.width   =  intersect.width()  / mapfile->xscale;
                    job.height  = -intersect.height() / mapfile->yscale;

                    job.p1.u    = u;
                    job.p1.v    = v;
                    job.p2.u    = u + dU1;
                    job.p2.v    = v + dV1;

                    pj_transform(mapfile->pj,pjWGS84,1,0,&job.p1.u,&job.p1.v,0);
                    pj_transform(mapfile->pj,pjWGS84,1,0,&job.p2.u,&job.p2.v,0);

                    jobs        << job;
                }
            }
        }
        delete mapfile;
    }

    pj_free(pjWGS84);

    slotFinishedKMZ1(0,QProcess::NormalExit);
}
