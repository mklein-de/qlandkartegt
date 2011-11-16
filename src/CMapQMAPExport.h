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

        virtual void explain() = 0;
        virtual void nextJob(QProcess& cmd) = 0;


        static QString getTempFilename();
    protected:
        CMapQMAPExport * gui;
    private:
        static quint32 tmpFileCnt;
};

class CMapExportStateCutFiles : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateCutFiles(CMapQMAPExport * parent);
        virtual ~CMapExportStateCutFiles();

        void explain();
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

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;
};

class CMapExportStateCombineFiles : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateCombineFiles(CMapQMAPExport * parent);
        virtual ~CMapExportStateCombineFiles();

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QStringList srcFile;
            QString tarFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;

};

class CMapExportStateConvColor : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateConvColor(CMapQMAPExport * parent);
        virtual ~CMapExportStateConvColor();

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString srcFile;
            QString tarFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;

};


class CMapExportStateReproject : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateReproject(const QString& proj, CMapQMAPExport * parent);
        virtual ~CMapExportStateReproject();

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString srcFile;
            QString tarFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
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

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QStringList overviews;
            QString srcFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;
};

class CMapExportStateGCM : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateGCM(const QString& app, CMapQMAPExport * parent);
        virtual ~CMapExportStateGCM();

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString jpegQuality;
            QString jpegSubSmpl;
            QString zOrder;
            QString tileFile;
            QStringList srcFile;
            QString tarFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;

        const QString app;

};

class CMapExportStateJNX : public IMapExportState
{
    Q_OBJECT;
    public:
        CMapExportStateJNX(const QString& app, CMapQMAPExport * parent);
        virtual ~CMapExportStateJNX();

        void explain();
        void nextJob(QProcess& cmd);

        struct job_t
        {
            QString jpegQuality;
            QString jpegSubSmpl;
            QString zOrder;
            QString productId;
            QString productName;
            QString description;
            QString copyright;
            QStringList srcFile;
            QString tarFile;
        };

        void addJob(const job_t& job){jobs << job;}
        const QList<job_t>& getJobs(){return jobs;}

    private:
        QList<job_t> jobs;
        int jobIdx;

        const QString app;

};


class CMapQMAPExport : public QDialog, private Ui::IMapQMAPExport
{
    Q_OBJECT;
    public:
        CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent);
        virtual ~CMapQMAPExport();

        void stdOut(const QString& str, bool gui = false);

        void setNextState();

    public slots:
        void slotFinished(int exitCode, QProcess::ExitStatus status);

    private slots:
        void slotBirdsEyeToggled(bool checked);
        void slotQLMToggled(bool checked);
        void slotGCMToggled(bool checked);
        void slotOutputPath();

        void slotStderr();
        void slotStdout();
        void slotStart();
        void slotCancel();
        void slotDetails();


    private:
        void progress(const QString& str);

        const CMapSelectionRaster& mapsel;

        bool has_map2jnx;
        QString path_map2jnx;
        QString path_map2gcm;

        QProcess cmd;

        QList<IMapExportState*> states;
        QPointer<IMapExportState> state;

        QString output;
};

#endif //CMAPQMAPEXPORT_H

