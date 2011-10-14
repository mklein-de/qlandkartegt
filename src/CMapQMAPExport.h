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
#ifndef CMAPQMAPEXPORT_H
#define CMAPQMAPEXPORT_H

#include <QDialog>
#include <QProcess>
#include "ui_IMapQMAPExport.h"
#include <projects.h>

#define MAX_MYNAV (1024*1024)

class CMapSelectionRaster;
class QTemporaryFile;


class CMapQMAPExport : public QDialog, private Ui::IMapQMAPExport
{
    Q_OBJECT;
    public:
        CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent);
        virtual ~CMapQMAPExport();

    private slots:
        void slotStart();
        void slotOutputPath();
        void slotStderr();
        void slotStdout();
        void slotFinished1( int exitCode, QProcess::ExitStatus status);
        void slotFinished2( int exitCode, QProcess::ExitStatus status);
//        void slotFinished3( int exitCode, QProcess::ExitStatus status);
        void slotFinished4( int exitCode, QProcess::ExitStatus status);
        void slotFinished5( int exitCode, QProcess::ExitStatus status);

        void slotBirdsEyeToggled(bool checked);
        void slotGCMToggled(bool checked);
    private:
        void startQLM();
        void startGCM();
        const CMapSelectionRaster& mapsel;


        QProcess cmd1;
        QProcess cmd2;
        QProcess cmd3;
        QProcess cmd4;
        QProcess cmd5;


        QTemporaryFile * file1;
        QTemporaryFile * file2;        

        struct job_t
        {
            QString name;
            QString srcFilename;
            QString tarFilename;
            quint32 xoff;
            quint32 yoff;
            quint32 width;
            quint32 height;

            XY p1;
            XY p2;

            int idx;
        };

        QList<job_t> jobs;
        QList<job_t> jobsLowrance;
        QList<job_t> tmpjobs;
        QStringList outfiles;

        bool has_map2jnx;
        QString path_map2jnx;
        QString path_map2gcm;
};
#endif                           //CMAPQMAPEXPORT_H
