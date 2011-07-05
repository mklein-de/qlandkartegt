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
#include <gdal_priv.h>

CMapQMAPExport::CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent)
: QDialog(parent)
, mapsel(mapsel)
, file1(0)
, file2(0)
, has_map2jnx(false)
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
    connect(radioJNX, SIGNAL(toggled(bool)), this, SLOT(slotBirdsEyeToggled(bool)));
    connect(radioGCM, SIGNAL(toggled(bool)), this, SLOT(slotGCMToggled(bool)));

    connect(&cmd1, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd1, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd1, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished2(int,QProcess::ExitStatus)));

    connect(&cmd2, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd2, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd2, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished1(int,QProcess::ExitStatus)));

//    connect(&cmd3, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
//    connect(&cmd3, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
//    connect(&cmd3, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished1(int,QProcess::ExitStatus)));

    connect(&cmd4, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd4, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd4, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished4(int,QProcess::ExitStatus)));


#ifdef WIN32
    path_map2jnx = QCoreApplication::applicationDirPath()+QDir::separator()+"map2jnx.exe";
    QFile file_map2jnx(path_map2jnx);
    has_map2jnx = file_map2jnx.exists();
    path_map2gcm= QCoreApplication::applicationDirPath()+QDir::separator()+"map2gcm.exe";
#else
    QProcess proc1;
    proc1.start(MAP2JNX);
    proc1.waitForFinished();
    has_map2jnx = proc1.error() == QProcess::UnknownError;
    path_map2jnx = MAP2JNX;
#if defined(Q_WS_MAC)
    // MacOS X: application is stored in the bundle folder
    path_map2gcm = QString("%1/Resources/map2gcm").arg(QCoreApplication::applicationDirPath().replace(QRegExp("MacOS$"), ""));
#else
    path_map2gcm= "map2gcm";
#endif
#endif
    groupBirdsEye->hide();
    groupJPEG->hide();
    groupDevice->hide();

    spinJpegQuality->setValue(cfg.value("map/export/jnx/quality",75).toInt());
    comboJpegSubsampling->setCurrentIndex(comboJpegSubsampling->findText(cfg.value("map/export/jnx/subsampling","411").toString()));
    spinProductId->setValue(cfg.value("map/export/jnx/productid",0).toInt());
    lineProductName->setText(cfg.value("map/export/jnx/productname","BirdsEye").toString());
    lineCopyright->setText(cfg.value("map/export/jnx/copyright","None").toString());

    radioQLM->setChecked(cfg.value("map/export/qlm", true).toBool());
    radioGCM->setChecked(cfg.value("map/export/gcm", false).toBool());
    radioLOW->setChecked(cfg.value("map/export/low", false).toBool());

    if (has_map2jnx)
    {
        radioJNX->setChecked(cfg.value("map/export/jnx", false).toBool());
    }
    else
    {
        radioJNX->hide();
    }
}


CMapQMAPExport::~CMapQMAPExport()
{
    QSettings cfg;
    cfg.setValue("map/export/qlm", radioQLM->isChecked());
    cfg.setValue("map/export/gcm", radioGCM->isChecked());
    cfg.setValue("map/export/jnx", radioJNX->isChecked());
    cfg.setValue("map/export/low", radioLOW->isChecked());

    cfg.setValue("map/export/jnx/quality", spinJpegQuality->value());
    cfg.setValue("map/export/jnx/subsampling", comboJpegSubsampling->currentText());
    cfg.setValue("map/export/jnx/productid", spinProductId->value());
    cfg.setValue("map/export/jnx/productname",lineProductName->text());
    cfg.setValue("map/export/jnx/copyright",lineCopyright->text());
}

void CMapQMAPExport::slotBirdsEyeToggled(bool checked)
{
    if(checked)
    {
        groupBirdsEye->show();
        groupJPEG->show();
        groupDevice->show();

    }
    else
    {
        groupBirdsEye->hide();
        groupJPEG->hide();
        groupDevice->hide();
    }
}

void CMapQMAPExport::slotGCMToggled(bool checked)
{
    if(checked)
    {
        groupJPEG->show();
        groupDevice->show();
        labelTileSelection->hide();

    }
    else
    {
        groupJPEG->hide();
        groupDevice->hide();
        labelTileSelection->show();
    }
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
    QString str;
    textBrowser->setTextColor(Qt::red);
    if(sender() == &cmd1)
    {
        str = cmd1.readAllStandardError();
    }
    else if(sender() == &cmd2)
    {
        str = cmd2.readAllStandardError();
    }
//    else if(sender() == &cmd3)
//    {
//        str = cmd3.readAllStandardError();
//    }
    else if(sender() == &cmd4)
    {
        str = cmd4.readAllStandardError();
    }


#ifndef WIN32
    if(str[0] == '\r')
    {
        textBrowser->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
        textBrowser->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
        textBrowser->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
        textBrowser->textCursor().removeSelectedText();

        str = str.split("\r").last();
    }
#endif

    textBrowser->insertPlainText(str);
    textBrowser->verticalScrollBar()->setValue(textBrowser->verticalScrollBar()->maximum());

}


void CMapQMAPExport::slotStdout()
{
    QString str;
    textBrowser->setTextColor(Qt::blue);

    if(sender() == &cmd1)
    {
        str = cmd1.readAllStandardOutput();
    }
    else if(sender() == &cmd2)
    {
        str = cmd2.readAllStandardOutput();
    }
//    else if(sender() == &cmd3)
//    {
//        str = cmd3.readAllStandardOutput();
//    }
    else if(sender() == &cmd4)
    {
        str = cmd4.readAllStandardOutput();
    }

#ifndef WIN32
    if(str[0] == '\r')
    {
        textBrowser->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
        textBrowser->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
        textBrowser->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
        textBrowser->textCursor().removeSelectedText();

        str = str.split("\r").last();
    }
#endif

    textBrowser->insertPlainText(str);
    textBrowser->verticalScrollBar()->setValue(textBrowser->verticalScrollBar()->maximum());
}

void CMapQMAPExport::slotStart()
{
    if(radioQLM->isChecked())
    {
        startQLM();
    }
    else if(radioGCM->isChecked())
    {
        startQLM();
    }
    else if(radioJNX->isChecked())
    {
        startQLM();
    }
    else if(radioLOW->isChecked())
    {
        startQLM();
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
        QStringList tmpOutFiles;
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
            QRectF intersect = selarea.intersected(maparea);

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
                if(job.width==0)
                {
                    job.width=1;
                }
                job.height = -intersect.height() / mapfile->yscale;
                if (job.height==0)
                {
                    job.height=1;
                }

                quint64 div = (((job.width*job.height)/(1024*1024))*3);
//                                 qDebug() << "xoff: 0 <" << job.xoff;
//                                 qDebug() << "yoff: 0 <" << job.yoff;
//                                 qDebug() << "x2  :    " << (job.xoff + job.width)  << " <" << mapfile->xsize_px;
//                                 qDebug() << "y2  :    " << (job.yoff + job.height) << " <" << mapfile->ysize_px;
                if (radioLOW->isChecked() && (div > MAXENDURA))
                {
                    job_t tmpJob = job;

                    div = (quint64)sqrt(div/(MAXENDURA));
                    div += 1;

                    int tilesX = job.width/div;
                    int tilesY = job.height/div;
                    for (unsigned int divY=0; divY<div; divY++)
                    {
                        for (unsigned int divX=0; divX<div; divX++)
                        {

                            job.tarFilename = job.tarFilename.left(job.tarFilename.length()-4);
                            job.tarFilename = job.tarFilename+"_"+QString::number(divX*tilesX)+"_"+QString::number(divY*tilesY)+".tif";
                            job.xoff=tmpJob.xoff+divX*tilesX;
                            job.yoff=tmpJob.yoff+divY*tilesY;

                            if (divY == div-1)
                            {
                                job.height = tmpJob.height-divY*tilesY;
                            }
                            else
                            {
                                job.height=tilesY;
                            }

                            if (divX == div-1)
                            {
                                job.width = tmpJob.width-divX*tilesX;
                            }
                            else
                            {
                                job.width=tilesX;
                            }

                            jobs        << job;
                            tmpOutFiles    << tarPath.relativeFilePath(job.tarFilename);

                            tardef.setValue(QString("level%1/files").arg(level), tmpOutFiles.join("|"));
                            tardef.setValue(QString("level%1/zoomLevelMin").arg(level), srcdef.value(QString("level%1/zoomLevelMin").arg(level)));
                            tardef.setValue(QString("level%1/zoomLevelMax").arg(level), srcdef.value(QString("level%1/zoomLevelMax").arg(level)));
                            job.tarFilename=tmpJob.tarFilename;
                        }
                    }

                }
                else
                {
                    jobs        << job;
                    tmpOutFiles    << tarPath.relativeFilePath(job.tarFilename);
                    tardef.setValue(QString("level%1/files").arg(level), tmpOutFiles.join("|"));
                    tardef.setValue(QString("level%1/zoomLevelMin").arg(level), srcdef.value(QString("level%1/zoomLevelMin").arg(level)));
                    tardef.setValue(QString("level%1/zoomLevelMax").arg(level), srcdef.value(QString("level%1/zoomLevelMax").arg(level)));
                }
            }
            delete mapfile;
        }
    }

    outfiles.clear();
    slotFinished1(0,QProcess::NormalExit);
}

static bool tileIndexLessThan(const QPair<int, int> &i1, const QPair<int, int> &i2)
{
    if(i1.second < i2.second)
    {
        return true;
    }
    else if (i1.second == i2.second)
    {
        return i1.first < i2.first;
    }
    else{
        return false;
    }
}

void CMapQMAPExport::slotFinished1( int exitCode, QProcess::ExitStatus status)
{
    //qDebug() << exitCode << status;
    if(file1){delete file1; file1 = 0;}
    if(file2){delete file2; file2 = 0;}
    if(jobs.isEmpty())
    {
        if(has_map2jnx && radioJNX->isChecked())
        {
            QString prefix = linePrefix->text();
            QDir tarPath(labelPath->text());

            QStringList args;

            args << "-q" << QString::number(spinJpegQuality->value());
            args << "-s" << comboJpegSubsampling->currentText();
            args << "-p" << QString::number(spinProductId->value());
            args << "-m" << lineProductName->text();
            args << "-n" << lineDescription->text();
            args << "-c" << lineCopyright->text();
            args << "-z" << QString::number(spinZOrder->value());

            args += outfiles;

            args << tarPath.filePath(QString("%1.jnx").arg(prefix));
            textBrowser->setTextColor(Qt::black);
            textBrowser->append(path_map2jnx + " " +  args.join(" ") + "\n");

            cmd4.start(path_map2jnx, args);
            return;
        }
        else if(radioGCM->isChecked())
        {
            QString prefix = linePrefix->text();
            QDir tarPath(labelPath->text());

            QStringList args;

            args << "-q" << QString::number(spinJpegQuality->value());
            args << "-s" << comboJpegSubsampling->currentText();
            args << "-z" << QString::number(spinZOrder->value());

            QList< QPair<int, int> > keys = mapsel.selTiles.keys();
            if(keys.count())
            {
                QPair<int, int> key;
                file1 = new QTemporaryFile();
                file1->open();

                qSort(keys.begin(), keys.end(), tileIndexLessThan);
                key = keys.last();
                int xmax = key.first  + 1;

                QString index;
                foreach(key, keys)
                {
                    if(mapsel.selTiles[key] == false)
                    {
                        index += QString("%1 ").arg(key.second * xmax + key.first);
                    }
                }

                file1->write(index.toLatin1());
                file1->flush();
                file1->close();

                args << "-t" << file1->fileName();
            }


            args += outfiles;

            args << tarPath.filePath(QString("%1.kmz").arg(prefix));
            textBrowser->setTextColor(Qt::black);
            textBrowser->append(path_map2gcm + " " +  args.join(" ") + "\n");

            cmd4.start(path_map2gcm, args);
            return;
        }
        else
        {
            textBrowser->setTextColor(Qt::black);
            textBrowser->append(tr("--- finished ---\n"));
            return;
        }
    }

    file1 = new QTemporaryFile();
    file1->open();
    file2 = new QTemporaryFile();
    file2->open();

    job_t job = jobs.first();
    QStringList args;
    if(radioLOW->isChecked())
    {
        //qDebug() << "Make Lowrance 24bit";
        args << "-expand" << "rgb";
        //args << "-co" << "compress=NONE";
    }
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
    job_t job = jobs.takeFirst();
    QStringList args;
    QTemporaryFile * tmpFile = new QTemporaryFile();
    tmpFile->setFileName(job.tarFilename);
    if (tmpFile->exists())
    {
        tmpFile->remove();
    }
    outfiles << job.tarFilename;

    args << "-t_srs" << "EPSG:4326";
    args << "-ts"    << QString::number(job.width) << QString::number(job.height);
    args << "-co" << "tiled=yes";
    args << "-co" << "blockxsize=256";
    args << "-co" << "blockysize=256";
    if(radioLOW->isChecked())
    {
        args << "-co" << "compress=NONE";
    }
    else
    {
         args << "-co" << "compress=LZW";
    }
    args << file1->fileName();
    //args << file2->fileName();
    args << job.tarFilename;
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(GDALWARP " " +  args.join(" ") + "\n");

    cmd2.start(GDALWARP, args);
}


//void CMapQMAPExport::slotFinished3( int exitCode, QProcess::ExitStatus status)
//{
//    job_t job = jobs.takeFirst();

//    outfiles << job.tarFilename;

//    QStringList args;
//    args << "-co" << "tiled=yes";
//    args << "-co" << "blockxsize=256";
//    args << "-co" << "blockysize=256";
//    args << "-co" << "compress=LZW";
//    args << file2->fileName();
//    args << job.tarFilename;

//    textBrowser->setTextColor(Qt::black);
//    textBrowser->append(GDALTRANSLATE " " +  args.join(" ") + "\n");

//    cmd3.start(GDALTRANSLATE, args);
//}

void CMapQMAPExport::slotFinished4( int exitCode, QProcess::ExitStatus status)
{
    QString prefix = linePrefix->text();
    QDir tarPath(labelPath->text());
    QFile::remove(tarPath.filePath(QString("%1.qmap").arg(prefix)));

    for(int i = 0; i < outfiles.count(); i++)
    {
        QFile::remove(outfiles[i]);
    }

    if(file1){delete file1; file1 = 0;}
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(tr("--- finished ---\n"));
}

//bool CMapQMAPExport::isEnduraMap(job_t job)
//{
//    if (job.height*job.width*24 > 512*1024*1024)
//        return false;
//    return true;
//}
