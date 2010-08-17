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
        void slotFinished3( int exitCode, QProcess::ExitStatus status);

        void slotFinishedKMZ1( int exitCode, QProcess::ExitStatus status);
        void slotFinishedKMZ2( int exitCode, QProcess::ExitStatus status);
        void slotFinishedKMZ3( int exitCode, QProcess::ExitStatus status);

    private:
        void startQLM();
        void startGE();
        void startGCM();
        const CMapSelectionRaster& mapsel;

        QProcess cmd1;
        QProcess cmd2;
        QProcess cmd3;

        QProcess cmdKMZ1;
        QProcess cmdKMZ2;

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
};
#endif                           //CMAPQMAPEXPORT_H
