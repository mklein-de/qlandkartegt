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
#ifndef CMAPQMAPEXPORT_H
#define CMAPQMAPEXPORT_H

#include <QDialog>
#include <QProcess>
#include <QFile>
#include <QList>
#include <QPointer>
#include <QTemporaryFile>
#include "ui_IMapQMAPExport.h"

class CMapSelectionRaster;
class CMapQMAPExport;

class IMapExportState : public QObject
{
    Q_OBJECT;
    public:
        IMapExportState(CMapQMAPExport * parent);
        virtual ~IMapExportState();

        virtual void nextJob(QProcess& cmd) = 0;

        static QString getTempFilename();
    protected:
        CMapQMAPExport * gui;
};

class CMapExportStateCutFiles : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateCutFiles(int levels, CMapQMAPExport * parent);
        virtual ~CMapExportStateCutFiles();

        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString srcFile;
            QString tarFile;

            quint32 xoff;
            quint32 yoff;
            quint32 width;
            quint32 height;

            int level;
        };

        void addJob(const job_t& job);

    protected:


    private:
        const int levels;
        QList<job_t> jobs;
        int jobIdx;
};

class CMapExportStateCombineFiles : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateCombineFiles(int levels, CMapQMAPExport * parent);
        virtual ~CMapExportStateCombineFiles();

        void nextJob(QProcess& cmd);

        struct job_t
        {
            QStringList srcFile;
            QString tarFile;

            int level;
        };

        void addJob(const job_t& job);

    private:
        const int levels;
        QList<job_t> jobs;
        int jobIdx;

};

class CMapExportStateReproject : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateReproject(int levels, const QString& proj, CMapQMAPExport * parent);
        virtual ~CMapExportStateReproject();

        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString srcFile;
            QString tarFile;

            int level;
        };

        void addJob(const job_t& job);

    private:
        const int levels;
        QList<job_t> jobs;
        int jobIdx;
        QString proj;

};

class CMapExportStateOptimize : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateOptimize(CMapQMAPExport * parent);
        virtual ~CMapExportStateOptimize();

        void nextJob(QProcess& cmd);
        void addJob(const QString& job);

    private:
        QStringList jobs;
        int jobIdx;
};


class CMapQMAPExport : public QDialog, private Ui::IMapQMAPExport
{
    Q_OBJECT;
    public:
        CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent);
        virtual ~CMapQMAPExport();

        void stdout(const QString& str);

        void setNextState(IMapExportState * next);

    private slots:
        void slotBirdsEyeToggled(bool checked);
        void slotGCMToggled(bool checked);

        void slotStderr();
        void slotStdout();
        void slotFinished(int exitCode, QProcess::ExitStatus status);

        void slotStart();


    private:
        const CMapSelectionRaster& mapsel;

        bool has_map2jnx;
        QString path_map2jnx;
        QString path_map2gcm;

        QProcess cmd;

        QPointer<IMapExportState> state;

};

#endif //CMAPQMAPEXPORT_H

