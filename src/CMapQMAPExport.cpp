/**********************************************************************************************
    Copyright (C) 2011 Oliver Eichler oliver.eichler@gmx.de

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
#include "CMapDB.h"
#include "CMapSelectionRaster.h"
#include "CMapFile.h"

#include <QtGui>

// --------------------------------------------------------------------------------------------
IMapExportState::IMapExportState(CMapQMAPExport * parent)
: QObject(parent)
, gui(parent)
{

}

IMapExportState::~IMapExportState()
{

}

QString IMapExportState::getTempFilename()
{
    QTemporaryFile tmp;
    tmp.open();
    return tmp.fileName();
}

// --------------------------------------------------------------------------------------------
CMapExportStateCutFiles::CMapExportStateCutFiles(int levels, CMapQMAPExport * parent)
: IMapExportState(parent)
, levels(levels)
, jobIdx(0)
{
    gui->stdout(   "*************************************");
    gui->stdout(tr("Cut area from files..."));
    gui->stdout(   "-------------------------------------");
}

CMapExportStateCutFiles::~CMapExportStateCutFiles()
{
    qDebug() << "~CMapExportStateCutFiles()";
}

void CMapExportStateCutFiles::addJob(const job_t& job)
{
    jobs << job;
}

void CMapExportStateCutFiles::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        QStringList args;
        args << "-co" << "tiled=yes" << "-co" << "compress=LZW";
        args << "-expand" << "rgb";
        args << "-srcwin";

        args << QString::number(job.xoff) << QString::number(job.yoff);
        args << QString::number(job.width) << QString::number(job.height);
        args << job.srcFile;
        args << job.tarFile;

        gui->stdout(GDALTRANSLATE " " +  args.join(" ") + "\n");
        cmd.start(GDALTRANSLATE, args);

        jobIdx++;
    }
    else
    {
        CMapExportStateCombineFiles * nextState = new CMapExportStateCombineFiles(levels, gui);

        for(int level = 1; level <= levels; level++)
        {
            CMapExportStateCombineFiles::job_t job;
            job.level   = level;
            job.tarFile = getTempFilename();

            foreach(const job_t& j, jobs)
            {
                if(j.level == level)
                {
                    job.srcFile << j.tarFile;
                }
            }

            nextState->addJob(job);
        }

        gui->setNextState(nextState);
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateCombineFiles::CMapExportStateCombineFiles(int levels, CMapQMAPExport * parent)
: IMapExportState(parent)
, levels(levels)
, jobIdx(0)
{
    gui->stdout(   "*************************************");
    gui->stdout(tr("Combine files for each level..."));
    gui->stdout(   "-------------------------------------");
}

CMapExportStateCombineFiles::~CMapExportStateCombineFiles()
{
    qDebug() << "~CMapExportStateCombineFiles()";

    foreach(const job_t& job, jobs)
    {
        foreach(const QString& file, job.srcFile)
        {
            QFile::remove(file);
        }
    }

}

void CMapExportStateCombineFiles::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        if(job.srcFile.count() == 1)
        {
            QStringList args;
            args << tr("done");
            gui->stdout("copy " + job.srcFile[0] + " ->" + job.tarFile + "\n");
            QFile::rename(job.srcFile[0], job.tarFile);
            cmd.start("echo", args);
        }
        else
        {
            QStringList args;
            args << "-co" << "tiled=yes" << "-co" << "compress=LZW";
            args << job.srcFile;
            args << job.tarFile;

            gui->stdout(GDALWARP " " +  args.join(" ") + "\n");
            cmd.start(GDALWARP, args);
        }

        jobIdx++;
    }
    else
    {
        CMapExportStateReproject * nextState = new CMapExportStateReproject(levels, "EPSG:4326",gui);

        foreach(const job_t& j, jobs)
        {
            CMapExportStateReproject::job_t job;

            job.level   = j.level;
            job.srcFile = j.tarFile;
            job.tarFile = getTempFilename();

            nextState->addJob(job);
        }

        gui->setNextState(nextState);
    }
}

void CMapExportStateCombineFiles::addJob(const job_t& job)
{
    jobs << job;
}

// --------------------------------------------------------------------------------------------
CMapExportStateReproject::CMapExportStateReproject(int levels, const QString& proj, CMapQMAPExport * parent)
: IMapExportState(parent)
, levels(levels)
, jobIdx(0)
, proj(proj)
{
    gui->stdout(   "*************************************");
    gui->stdout(tr("Re-project files..."));
    gui->stdout(   "-------------------------------------");
}

CMapExportStateReproject::~CMapExportStateReproject()
{
    qDebug() << "~CMapExportStateReproject()";

    foreach(const job_t& job, jobs)
    {
        QFile::remove(job.srcFile);
    }

}

void CMapExportStateReproject::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        QString width, height;
        {
            CMapFile mapfile(job.srcFile, this);
            width   = QString("%1").arg(mapfile.xsize_px);
            height  = QString("%1").arg(mapfile.ysize_px);
        }

        QStringList args;
        args << "-t_srs" << proj;
        args << "-ts" << width << height;
        args << "-r" << "cubic";
        args << "-co" << "tiled=yes" << "-co" << "compress=jpeg";
        args << job.srcFile;
        args << job.tarFile;

        gui->stdout(GDALWARP " " +  args.join(" ") + "\n");
        cmd.start(GDALWARP, args);

        jobIdx++;
    }
    else
    {
        CMapExportStateOptimize * nextState = new CMapExportStateOptimize(gui);

        foreach(const job_t& j, jobs)
        {
            nextState->addJob(j.tarFile);
        }

        gui->setNextState(nextState);
    }
}

void CMapExportStateReproject::addJob(const job_t& job)
{
    jobs << job;
}

// --------------------------------------------------------------------------------------------
CMapExportStateOptimize::CMapExportStateOptimize(CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
{
    gui->stdout(   "*************************************");
    gui->stdout(tr("Optimize files..."));
    gui->stdout(   "-------------------------------------");
}

CMapExportStateOptimize::~CMapExportStateOptimize()
{
    qDebug() << "~CMapExportStateOptimize()";
}

void CMapExportStateOptimize::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        QString& job = jobs[jobIdx];

        QStringList args;
        args << "-r" << "cubic";
        args << "--config" << "COMPRESS_OVERVIEW" << "JPEG";
        args << job;
        args << "4" << "16";

        gui->stdout(GDALADDO " " +  args.join(" ") + "\n");
        cmd.start(GDALADDO, args);

        jobIdx++;
    }
    else
    {
        gui->stdout("---done---\n");
    }
}

void CMapExportStateOptimize::addJob(const QString& job)
{
    jobs << job;
}


// --------------------------------------------------------------------------------------------
CMapQMAPExport::CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent)
: QDialog(parent)
, mapsel(mapsel)
, has_map2jnx(false)
{
    setupUi(this);
    toolPath->setIcon(QPixmap(":/icons/iconFileLoad16x16.png"));

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotOutputPath()));
    connect(pushExport, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(radioJNX, SIGNAL(toggled(bool)), this, SLOT(slotBirdsEyeToggled(bool)));
    connect(radioGCM, SIGNAL(toggled(bool)), this, SLOT(slotGCMToggled(bool)));

    connect(&cmd, SIGNAL(readyReadStandardError()), this, SLOT(slotStderr()));
    connect(&cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(slotStdout()));
    connect(&cmd, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotFinished(int,QProcess::ExitStatus)));


    QSettings cfg;
    labelPath->setText(cfg.value("path/export","./").toString());

    IMap& map = CMapDB::self().getMap();
    linePrefix->setText(QString("%1_%2_%3").arg(QFileInfo(map.getFilename()).baseName()).arg(mapsel.lon1 * RAD_TO_DEG).arg(mapsel.lat1 * RAD_TO_DEG));
    lineDescription->setText(QFileInfo(map.getFilename()).baseName());

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

void CMapQMAPExport::slotStderr()
{
    QString str;
    textBrowser->setTextColor(Qt::red);

    str = cmd.readAllStandardError();

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
    str = cmd.readAllStandardOutput();

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


void CMapQMAPExport::stdout(const QString& str)
{
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(str);
}

void CMapQMAPExport::slotStart()
{

    /// @todo disable start button

    CMapDB::map_t& map = CMapDB::self().knownMaps[mapsel.mapkey];
    QDir srcPath = QFileInfo(map.filename).absolutePath();
    QSettings srcdef(map.filename, QSettings::IniFormat);

    int levels = srcdef.value("main/levels",0).toInt();

    CMapExportStateCutFiles * nextState = new CMapExportStateCutFiles(levels,this);

    // iterate over all map levels
    for(int level = 1; level <= levels; ++level)
    {
        // iterate over all files in a level
        QStringList filenames = srcdef.value(QString("level%1/files").arg(level),"").toString().split("|", QString::SkipEmptyParts);
        foreach(const QString& filename, filenames)
        {
            qDebug();
            CMapFile mapfile(srcPath.filePath(filename), this);
            if(!mapfile.ok)
            {
                QMessageBox::critical(0,tr("Error ..."), tr("Failed to read %1").arg(filename), QMessageBox::Abort,  QMessageBox::Abort);
                return QDialog::reject();
            }

            // transform the WGS84 points that define the selection into the map files projection system
            PJ * pjWGS84 = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");
            XY p1,p2;
            p1.u = mapsel.lon1;
            p1.v = mapsel.lat1;
            pj_transform(pjWGS84,mapfile.pj,1,0,&p1.u,&p1.v,0);

            p2.u = mapsel.lon2;
            p2.v = mapsel.lat2;
            pj_transform(pjWGS84,mapfile.pj,1,0,&p2.u,&p2.v,0);
            pj_free(pjWGS84);

            // cacluate the selected area on the map
            QRectF maparea(QPointF(mapfile.xref1, mapfile.yref1), QPointF(mapfile.xref2, mapfile.yref2));
            QRectF selarea(QPointF(p1.u, p1.v), QPointF(p2.u, p2.v));
            QRectF intersect = selarea.intersected(maparea);

            if(intersect.isValid())
            {
                CMapExportStateCutFiles::job_t job;

                job.level  = level;
                job.xoff   = (intersect.left()   - mapfile.xref1) / mapfile.xscale;
                job.yoff   = (intersect.bottom() - mapfile.yref1) / mapfile.yscale;
                job.width  =  intersect.width()  / mapfile.xscale;
                if(job.width == 0)
                {
                    // no empty files
                    continue;
                }
                job.height = -intersect.height() / mapfile.yscale;
                if (job.height == 0)
                {
                    // no empty files
                    continue;
                }

                job.tarFile = IMapExportState::getTempFilename();
                job.srcFile = mapfile.filename;

                qDebug() << job.level << job.srcFile << ">>" << job.tarFile;
                qDebug() << job.xoff << job.yoff << job.width << job.height;

                nextState->addJob(job);
            }
        }
    }

    setNextState(nextState);
}

void CMapQMAPExport::setNextState(IMapExportState * next)
{
    if(!state.isNull()) state->deleteLater();
    state = next;
    state->nextJob(cmd);
}

void CMapQMAPExport::slotFinished(int exitCode, QProcess::ExitStatus status)
{
    if(exitCode || status)
    {
        textBrowser->setTextColor(Qt::red);
        textBrowser->append(tr("--- failed ---\n"));

        delete state;
        return;
    }

    state->nextJob(cmd);
}
