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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CMapQMAPExport.h"
#include "CMapSelection.h"
#include "CMapDB.h"
#include "CMapFile.h"

#include <QtGui>

CMapQMAPExport::CMapQMAPExport(const CMapSelection& mapsel, QWidget * parent)
    : QDialog(parent)
    , mapsel(mapsel)
{
    setupUi(this);

    toolPath->setIcon(QPixmap(":/icons/iconFileLoad16x16"));

    QSettings cfg;
    labelPath->setText(cfg.value("path/export","./").toString());

    IMap& map = CMapDB::self().getMap();
    linePrefix->setText(QString("%1_%2_%3").arg(QFileInfo(map.getFilename()).baseName()).arg(mapsel.lon1 * RAD_TO_DEG).arg(mapsel.lat1 * RAD_TO_DEG));
    lineDescription->setText(QFileInfo(map.getFilename()).baseName());

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotOutputPath()));
    connect(pushExport, SIGNAL(clicked()), this, SLOT(slotStart()));

    connect(&cmd, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished(int,QProcess::ExitStatus)));

}

CMapQMAPExport::~CMapQMAPExport()
{

}

void CMapQMAPExport::slotOutputPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select ouput path..."), labelPath->text());
    if(path.isEmpty()) return;

    QSettings cfg;
    cfg.setValue("path/export", path);
    labelPath->setText(path);
}

void CMapQMAPExport::slotStderr()
{
    textBrowser->setTextColor(Qt::red);
    textBrowser->append(cmd.readAllStandardError());
}

void CMapQMAPExport::slotStdout()
{
    textBrowser->setTextColor(Qt::blue);
    QString str = cmd.readAllStandardOutput();

    textBrowser->insertPlainText(str);
    textBrowser->verticalScrollBar()->setValue(textBrowser->verticalScrollBar()->maximum());
}


void CMapQMAPExport::slotStart()
{
    // get map summary
    CMapDB::map_t& map = CMapDB::self().knownMaps[mapsel.mapkey];
    QSettings mapdef(map.filename,QSettings::IniFormat);

    QDir srcPath = QFileInfo(map.filename).absolutePath();
    QDir tarPath(labelPath->text());
    QString prefix = linePrefix->text();

    int idx = 0;
    int levels = mapdef.value("main/levels",0).toInt();
    for(int level = 1; level <= levels; ++level){
        QStringList filenames = mapdef.value(QString("level%1/files").arg(level),"").toString().split("|", QString::SkipEmptyParts);
        QString filename;
        foreach(filename, filenames){
            CMapFile * mapfile = new CMapFile(srcPath.filePath(filename), this);
            if(!mapfile->ok){
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
            if(intersect.isValid()){
                job_t job;
                job.idx    = idx++;
                job.srcFilename = mapfile->filename;
                job.tarFilename = tarPath.filePath(QString("%1%2.tif").arg(prefix).arg(job.idx));
                job.xoff   = (intersect.left()  - mapfile->xref1) / mapfile->xscale;
                job.yoff   = (intersect.top()   - mapfile->yref1) / mapfile->yscale;
                job.width  =  intersect.width()  / mapfile->xscale;
                job.height = -intersect.height() / mapfile->yscale;


                qDebug() << (job.xoff + job.width) << (job.yoff + job.height);
                qDebug() << mapfile->xsize_px << mapfile->ysize_px;


                jobs << job;
            }
            delete mapfile;
        }
    }

    job_t job = jobs.takeFirst();
    QStringList args;
    args << "-srcwin";
    args << QString::number(job.xoff) << QString::number(job.yoff);
    args << QString::number(job.width) << QString::number(job.width);
    args << "-co" << "tiled=yes";
    args << "-co" << "blockxsize=256";
    args << "-co" << "blockysize=256";
    args << "-co" << "compress=deflate";
    args << "-co" << "predictor=1";
    args << job.srcFilename;
    args << job.tarFilename;

    textBrowser->setTextColor(Qt::black);
    textBrowser->append("gdal_translate " +  args.join(" ") + "\n");

    cmd.start("gdal_translate", args);
}


void CMapQMAPExport::slotFinished( int exitCode, QProcess::ExitStatus status)
{
    qDebug() << exitCode << status;

    if(jobs.isEmpty()){
        textBrowser->setTextColor(Qt::black);
        textBrowser->append(tr("--- finished ---\n"));
        return;
    }
    job_t job = jobs.takeFirst();
    QStringList args;
    args << "-srcwin";
    args << QString::number(job.xoff) << QString::number(job.yoff);
    args << QString::number(job.width) << QString::number(job.width);
    args << "-co" << "tiled=yes";
    args << "-co" << "blockxsize=256";
    args << "-co" << "blockysize=256";
    args << "-co" << "compress=deflate";
    args << "-co" << "predictor=1";
    args << job.srcFilename;
    args << job.tarFilename;

    textBrowser->setTextColor(Qt::black);
    textBrowser->append("gdal_translate " +  args.join(" ") + "\n");

    cmd.start("gdal_translate", args);

}
