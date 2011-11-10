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
#include "config.h"

#include <QtGui>

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
    connect(radioQLM, SIGNAL(toggled(bool)), this, SLOT(slotQLMToggled(bool)));
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

    checkOverview2x->setChecked(cfg.value("map/export/over2x", true).toBool());
    checkOverview4x->setChecked(cfg.value("map/export/over4x", true).toBool());
    checkOverview8x->setChecked(cfg.value("map/export/over8x", true).toBool());
    checkOverview16x->setChecked(cfg.value("map/export/over16x", true).toBool());

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

    cfg.setValue("map/export/over2x",checkOverview2x->isChecked());
    cfg.setValue("map/export/over4x",checkOverview4x->isChecked());
    cfg.setValue("map/export/over8x",checkOverview8x->isChecked());
    cfg.setValue("map/export/over16x",checkOverview16x->isChecked());
}


void CMapQMAPExport::slotBirdsEyeToggled(bool checked)
{
    if(checked)
    {
        groupBirdsEye->show();
        groupJPEG->show();
        groupDevice->show();
        groupOptimization->hide();
    }
    else
    {
        groupBirdsEye->hide();
        groupJPEG->hide();
        groupDevice->hide();
        groupOptimization->hide();
    }
}

void CMapQMAPExport::slotGCMToggled(bool checked)
{
    if(checked)
    {
        groupJPEG->show();
        groupDevice->show();
        labelTileSelection->hide();
        groupOptimization->hide();

    }
    else
    {
        groupJPEG->hide();
        groupDevice->hide();
        labelTileSelection->show();
        groupOptimization->hide();
    }
}

void CMapQMAPExport::slotQLMToggled(bool checked)
{
    if(checked)
    {
        groupJPEG->hide();
        groupDevice->hide();
        labelTileSelection->show();
        groupOptimization->show();

    }
    else
    {
        groupJPEG->hide();
        groupDevice->hide();
        labelTileSelection->show();
        groupOptimization->hide();
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


void CMapQMAPExport::stdOut(const QString& str)
{
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(str);
}

void CMapQMAPExport::slotStart()
{
    if(!states.isEmpty() || !state.isNull()) return;

    qDebug() << "CMapQMAPExport::slotStart()";
    pushExport->setEnabled(false);

    CMapDB::map_t& map = CMapDB::self().knownMaps[mapsel.mapkey];
    QDir srcPath = QFileInfo(map.filename).absolutePath();
    QDir tarPath(labelPath->text());
    QString prefix = linePrefix->text();
    QSettings srcdef(map.filename, QSettings::IniFormat);

    int levels = srcdef.value("main/levels",0).toInt();

    // *********************************************
    // 1. step: cut defined areas from all map files
    // ---------------------------------------------
    CMapExportStateCutFiles * state1 = new CMapExportStateCutFiles(this);
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

                state1->addJob(job);
            }
        }
    }
    states << state1;

    // *********************************************
    // 2. step: combine files on a level
    // ---------------------------------------------
    CMapExportStateCombineFiles * state2 = new CMapExportStateCombineFiles(this);
    for(int level = 1; level <= levels; level++)
    {
        CMapExportStateCombineFiles::job_t job;
        job.tarFile = IMapExportState::getTempFilename();
        foreach(const CMapExportStateCutFiles::job_t& j, state1->getJobs())
        {
            if(j.level == level)
            {
                job.srcFile << j.tarFile;
            }
        }

        if(!job.srcFile.isEmpty())
        {
            state2->addJob(job);
        }
    }
    states << state2;

    // *********************************************
    // 3. step: move 8 bit files into rgb color space
    // ---------------------------------------------
    CMapExportStateConvColor * state3 = new CMapExportStateConvColor(this);
    foreach(const CMapExportStateCombineFiles::job_t& j, state2->getJobs())
    {
        CMapExportStateConvColor::job_t job;
        job.srcFile = j.tarFile;
        job.tarFile = IMapExportState::getTempFilename();
        state3->addJob(job);
    }
    states << state3;

    // *********************************************
    // 4. step: reproject files
    // ---------------------------------------------
    int cnt = 0;
    CMapExportStateReproject * state4 = new CMapExportStateReproject("EPSG:4326", this);
    foreach(const CMapExportStateConvColor::job_t& j, state3->getJobs())
    {
        CMapExportStateReproject::job_t job;
        job.srcFile = j.tarFile;

        if(radioQLM->isChecked())
        {
            job.tarFile = tarPath.filePath(QString("%1_%2.tif").arg(prefix).arg(cnt++));
            QFile::remove(job.tarFile);
        }
        else
        {
            job.tarFile = IMapExportState::getTempFilename();
        }
        state4->addJob(job);
    }
    states << state4;


    if(radioQLM->isChecked())
    {
        // *********************************************
        // 5. step: add overview levels
        // ---------------------------------------------

        QStringList overviews;
        if(checkOverview2x->isChecked())  overviews << "2";
        if(checkOverview4x->isChecked())  overviews << "4";
        if(checkOverview8x->isChecked())  overviews << "8";
        if(checkOverview16x->isChecked()) overviews << "16";

        CMapExportStateOptimize * state5 = new CMapExportStateOptimize(this);
        foreach(const CMapExportStateReproject::job_t& j, state4->getJobs())
        {
            CMapExportStateOptimize::job_t job;
            job.overviews   = overviews;
            job.srcFile     = j.tarFile;
            state5->addJob(job);
        }
        states << state5;
    }
    else if(radioGCM->isChecked())
    {
        // *********************************************
        // 5. step: convert Geotiff to Garmin Custom Map
        // ---------------------------------------------
        QString tileFile;
        QList< QPair<int, int> > keys = mapsel.selTiles.keys();
        if(keys.count())
        {
            QPair<int, int> key;

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

            QFile file(IMapExportState::getTempFilename());
            file.open(QIODevice::WriteOnly);
            file.write(index.toLatin1());
            file.flush();
            file.close();
            tileFile = file.fileName();
        }


        CMapExportStateGCM * state5 = new CMapExportStateGCM(path_map2gcm, this);

        CMapExportStateGCM::job_t job;

        job.jpegQuality = QString::number(spinJpegQuality->value());
        job.jpegSubSmpl = comboJpegSubsampling->currentText();
        job.zOrder      = QString::number(spinZOrder->value());
        job.tileFile    = tileFile;

        foreach(const CMapExportStateReproject::job_t& j, state4->getJobs())
        {
            job.srcFile << j.tarFile;

        }
        job.tarFile     = tarPath.filePath(QString("%1.kmz").arg(prefix));

        state5->addJob(job);
        states << state5;
    }
    else if(radioJNX->isChecked())
    {
        // *********************************************
        // 5. step: convert Geotiff to Garmin JNX Map
        // ---------------------------------------------
        CMapExportStateJNX * state5 = new CMapExportStateJNX(path_map2jnx, this);

        CMapExportStateJNX::job_t job;

        job.jpegQuality = QString::number(spinJpegQuality->value());
        job.jpegSubSmpl = comboJpegSubsampling->currentText();
        job.zOrder      = QString::number(spinZOrder->value());
        job.productId   = QString::number(spinProductId->value());
        job.productName = lineProductName->text();
        job.description = lineDescription->text();
        job.copyright   = lineCopyright->text();

        foreach(const CMapExportStateReproject::job_t& j, state4->getJobs())
        {
            job.srcFile << j.tarFile;

        }
        job.tarFile     = tarPath.filePath(QString("%1.jnx").arg(prefix));

        state5->addJob(job);
        states << state5;
    }

    // start the statemachine
    setNextState();
}

void CMapQMAPExport::setNextState()
{
    if(!state.isNull()) state->deleteLater();

    if(states.isEmpty())
    {
        stdOut(tr("--- done ---\n"));
        pushExport->setEnabled(true);
    }
    else
    {
        state = states.takeFirst();
        state->explain();
        state->nextJob(cmd);
    }
}

void CMapQMAPExport::slotFinished(int exitCode, QProcess::ExitStatus status)
{
    if(exitCode || status)
    {
        textBrowser->setTextColor(Qt::red);
        textBrowser->append(tr("--- failed ---\n"));

        qDeleteAll(states);
        states.clear();
        state->deleteLater();

        pushExport->setEnabled(true);
        return;
    }

    QApplication::processEvents();
    state->nextJob(cmd);
}

void CMapQMAPExport::slotOutputPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select ouput path..."), labelPath->text(), FILE_DIALOG_FLAGS);
    if(path.isEmpty()) return;

    QSettings cfg;
    cfg.setValue("path/export", path);
    labelPath->setText(path);
}



// --------------------------------------------------------------------------------------------

quint32 IMapExportState::tmpFileCnt = 0;

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
    QTemporaryFile * tmp = new QTemporaryFile(QDir::temp().absoluteFilePath(QString("qlgt_%1.XXXXXX.tif").arg(tmpFileCnt++)));
    tmp->open();
    QString fn =  tmp->fileName();
    tmp->close();
    delete tmp;

    return fn;
}

// --------------------------------------------------------------------------------------------
CMapExportStateCutFiles::CMapExportStateCutFiles(CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
{
}

CMapExportStateCutFiles::~CMapExportStateCutFiles()
{
    qDebug() << "~CMapExportStateCutFiles()";
}

void CMapExportStateCutFiles::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Cut area from files..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateCutFiles::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        bool isRgb;
        {
            CMapFile mapfile(job.srcFile, this);
            isRgb = mapfile.rasterBandCount != 1;
        }

        QStringList args;
        args << "-co" << "tiled=yes" << "-co" << "compress=LZW";

        if(!isRgb)
        {
            args << "-expand" << "rgba";
        }

        args << "-srcwin";
        args << QString::number(job.xoff) << QString::number(job.yoff);
        args << QString::number(job.width) << QString::number(job.height);
        args << job.srcFile;
        args << job.tarFile;

        jobIdx++;

        gui->stdOut(GDALTRANSLATE " " +  args.join(" ") + "\n");
        cmd.start(GDALTRANSLATE, args);

    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateCombineFiles::CMapExportStateCombineFiles(CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
{

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

void CMapExportStateCombineFiles::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Combine files for each level..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateCombineFiles::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        if(job.srcFile.count() == 1)
        {
            gui->stdOut("copy " + job.srcFile[0] + " ->" + job.tarFile + "\n");
            QFile::rename(job.srcFile[0], job.tarFile);
            jobIdx++;

            gui->slotFinished(0, QProcess::NormalExit);
            return;
        }

        QStringList args;
        args << "-co" << "tiled=yes" << "-co" << "compress=LZW";
        args << job.srcFile;
        args << job.tarFile;

        jobIdx++;

        gui->stdOut(GDALWARP " " +  args.join(" ") + "\n");
        cmd.start(GDALWARP, args);
    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateConvColor::CMapExportStateConvColor(CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
{

}

CMapExportStateConvColor::~CMapExportStateConvColor()
{
    qDebug() << "~CMapExportStateConvColor()";

    foreach(const job_t& job, jobs)
    {
        QFile::remove(job.srcFile);
    }

}

void CMapExportStateConvColor::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Reduce color bands to 3 (RGB)..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateConvColor::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        QStringList args;

        args << "-b" << "1";
        args << "-b" << "2";
        args << "-b" << "3";

        args << "-co" << "tiled=yes" << "-co" << "compress=jpeg";

        args << job.srcFile;
        args << job.tarFile;

        jobIdx++;

        gui->stdOut(GDALTRANSLATE " " +  args.join(" ") + "\n");
        cmd.start(GDALTRANSLATE, args);

    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateReproject::CMapExportStateReproject(const QString& proj, CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
, proj(proj)
{

}

CMapExportStateReproject::~CMapExportStateReproject()
{
    qDebug() << "~CMapExportStateReproject()";

    foreach(const job_t& job, jobs)
    {
        QFile::remove(job.srcFile);
    }

}

void CMapExportStateReproject::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Re-project files..."));
    gui->stdOut(   "-------------------------------------");
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

        jobIdx++;

        gui->stdOut(GDALWARP " " +  args.join(" ") + "\n");
        cmd.start(GDALWARP, args);


    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateOptimize::CMapExportStateOptimize(CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
{

}

CMapExportStateOptimize::~CMapExportStateOptimize()
{
    qDebug() << "~CMapExportStateOptimize()";
}

void CMapExportStateOptimize::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Optimize files..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateOptimize::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        if(job.overviews.isEmpty())
        {
            gui->stdOut(tr("nothing to do\n"));
            jobIdx++;

            gui->slotFinished(0, QProcess::NormalExit);
            return;
        }

        QStringList args;
        args << "-r" << "cubic";
        args << "--config" << "COMPRESS_OVERVIEW" << "JPEG";
        args << job.srcFile;
        args += job.overviews;

        jobIdx++;

        gui->stdOut(GDALADDO " " +  args.join(" ") + "\n");
        cmd.start(GDALADDO, args);
    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateGCM::CMapExportStateGCM(const QString& app, CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
, app(app)
{

}

CMapExportStateGCM::~CMapExportStateGCM()
{
    qDebug() << "~CMapExportStateGCM()";

    foreach(const job_t& job, jobs)
    {
        QFile::remove(job.tileFile);
        foreach(const QString& file, job.srcFile)
        {
            QFile::remove(file);
        }
    }
}

void CMapExportStateGCM::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Create Garmin Custom Map..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateGCM::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        QStringList args;
        args << "-q" << job.jpegQuality;
        args << "-s" << job.jpegSubSmpl;
        args << "-z" << job.zOrder;
        args << "-t" << job.tileFile;

        args += job.srcFile;
        args << job.tarFile;

        jobIdx++;

        gui->stdOut(app + " " +  args.join(" ") + "\n");
        cmd.start(app, args);

    }
    else
    {
        gui->setNextState();
    }
}

// --------------------------------------------------------------------------------------------
CMapExportStateJNX::CMapExportStateJNX(const QString& app, CMapQMAPExport * parent)
: IMapExportState(parent)
, jobIdx(0)
, app(app)
{

}

CMapExportStateJNX::~CMapExportStateJNX()
{
    qDebug() << "~CMapExportStateJNX()";

    foreach(const job_t& job, jobs)
    {
        foreach(const QString& file, job.srcFile)
        {
            QFile::remove(file);
        }
    }
}

void CMapExportStateJNX::explain()
{
    gui->stdOut(   "*************************************");
    gui->stdOut(tr("Create Garmin JNX Map..."));
    gui->stdOut(   "-------------------------------------");
}

void CMapExportStateJNX::nextJob(QProcess& cmd)
{
    if(jobIdx < jobs.count())
    {
        job_t& job = jobs[jobIdx];

        QStringList args;
        args << "-q" << job.jpegQuality;
        args << "-s" << job.jpegSubSmpl;
        args << "-p" << job.productId;
        args << "-m" << job.productName;
        args << "-n" << job.description;
        args << "-c" << job.copyright;
        args << "-z" << job.zOrder;
        args += job.srcFile;
        args << job.tarFile;

        jobIdx++;

        gui->stdOut(app + " " +  args.join(" ") + "\n");
        cmd.start(app, args);
    }
    else
    {
        gui->setNextState();
    }
}
