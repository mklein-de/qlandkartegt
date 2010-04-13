/**********************************************************************************************
    Copyright (C) 2007-2009 Oliver Eichler oliver.eichler@gmx.de

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

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QRegExp>
#include <gdal_priv.h>
#include <projects.h>

#include "CGetOpt.h"
#include "CAppOpts.h"

#ifdef __MINGW32__
#undef LP
#endif

#include "CMainWindow.h"
#include "CGarminTyp.h"	//TODO: this shall not be commented out when building a debug version with Visual Studio 2005

#include "config.h"

static void usage(std::ostream &s)
{
    s << "usage: qlandkartegt [-d | --debug]\n"
        "                    [-h | --help]\n"
        "                    [-m FD | --monitor=FD]\n"
        "                    [-n | --no-splash]\n"
        "                    [files...]\n"
        "\n"
        "The monitor function will read data from files if there is input on stream FD.\n"
        "For stdin use FD=0.\n\n";
}


static void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type)
    {
        case QtDebugMsg:
            if (qlOpts->debug)
            {
                puts(msg);
            }
            break;

        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", msg);
            break;

        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;

        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", msg);
            abort();
    }
}


CAppOpts *qlOpts;

static void processOptions()
{
    CGetOpt opts;                // uses qApp->argc() and qApp->argv()
    bool dValue;
    opts.addSwitch('d', "debug", &dValue);
    bool hValue;
    opts.addSwitch('h', "help", &hValue);
    QString mValue;
    opts.addOptionalOption('m', "monitor", &mValue, "0");
    bool nValue;
    opts.addSwitch('n', "no-splash", &nValue);
    QStringList args;
    opts.addOptionalArguments("files", &args);

    if (!opts.parse())
    {
        usage(std::cerr);
        exit(1);
    }

    if (hValue)
    {
        usage(std::cout);
        exit(0);
    }

    int m = -1;
    if (mValue != QString::null)
    {
        bool ok;
        m = mValue.toInt(&ok);
        if (!ok)
        {
            usage(std::cerr);
            exit(1);
        }
    }
    qlOpts = new CAppOpts(dValue,// bool debug
        m,                       // int monitor
        nValue,                  // bool nosplash
        args);                   // arguments
}


int main(int argc, char ** argv)
{

    {
        PJ * pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        PJ * pjGK    = pj_init_plus("+proj=tmerc +lat_0=0 +lon_0=12 +k=1 +x_0=4500000 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs +towgs84=606.0,23.0,413.0");

        //     double x = 12.09    * DEG_TO_RAD;
        //     double y = 49.0336  * DEG_TO_RAD;
        double x = 0.211011;
        double y = 0.855797;
        pj_transform(pjWGS84,pjGK,1,0,&x,&y,0);

        printf("------------ %f %f\n", x, y);
        char * ptr = pj_get_def(pjGK,0);
        printf("------------ %s\n",ptr);
        //         free(ptr);

        pj_free(pjWGS84);
        pj_free(pjGK);
    }

    QDir path(QDir::home().filePath(CONFIGDIR));
    if(!path.exists())
    {
        path.mkpath("./");
    }

    QString locale = QLocale::system().name();

#ifndef WIN32
    setenv("LC_NUMERIC","C",1);
#endif
    QApplication theApp(argc,argv);
    processOptions();
    qInstallMsgHandler(myMessageOutput);

#ifdef ENABLE_TRANSLATION
    {
        QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        QTranslator *qtTranslator = new QTranslator(0);
        if (qtTranslator->load(QLatin1String("qt_") + locale,resourceDir))
            theApp.installTranslator(qtTranslator);

        QStringList dirList;
#ifndef Q_WS_MAC
        dirList << ".";
        dirList << "src";
#ifndef Q_OS_WIN32
        dirList << QCoreApplication::applicationDirPath().replace(QRegExp("bin$"), "share/qlandkartegt/translations");
#else
        dirList << QCoreApplication::applicationDirPath();
#endif
#else
        dirList << QCoreApplication::applicationDirPath().replace(QRegExp("MacOS$"), "Resources");
#endif
        QTranslator *qlandkartegtTranslator = new QTranslator(0);
        qDebug() << dirList;
        foreach(QString dir, dirList)
        {
            QString transName = QLatin1String("qlandkartegt_") + locale;
            if (qlandkartegtTranslator->load( transName, dir))
            {
                theApp.installTranslator(qlandkartegtTranslator);
                qDebug() << "using file '"+ QDir(dir).canonicalPath() + "/" + transName + ".qm' for translations.";
                break;
            }
        }
    }
#endif

    {
        PJ * pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        PJ * pjGK    = pj_init_plus("+proj=tmerc +lat_0=0 +lon_0=12 +k=1 +x_0=4500000 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs +towgs84=606.0,23.0,413.0");

        //     double x = 12.09    * DEG_TO_RAD;
        //     double y = 49.0336  * DEG_TO_RAD;
        double x = 0.211011;
        double y = 0.855797;
        pj_transform(pjWGS84,pjGK,1,0,&x,&y,0);

        printf("------------ %f %f\n", x, y);
        char * ptr = pj_get_def(pjGK,0);
        printf("------------ %s\n",ptr);
        //         free(ptr);

        pj_free(pjWGS84);
        pj_free(pjGK);
    }

    GDALAllRegister();

    QCoreApplication::setApplicationName("QLandkarteGT");
    QCoreApplication::setOrganizationName("QLandkarteGT");
    QCoreApplication::setOrganizationDomain("qlandkarte.org");

#ifdef WIN32
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    qDebug() << QCoreApplication::applicationDirPath();
    qDebug() << QCoreApplication::libraryPaths();
#endif

    QSplashScreen *splash = 0;
    if (!qlOpts->nosplash)
    {
        splash = new QSplashScreen(QPixmap(":/pics/splash.png"));
#if defined(Q_WS_MAC)
        splash->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen);
#endif
        splash->show();
    }
    CMainWindow w;
    w.show();
    if (splash != 0)
    {
        splash->finish(&w);
        delete splash;
    }

    //     CGarminTyp typ("/home/oeichler/Desktop/teddy.typ",0);

    int res  = theApp.exec();

    GDALDestroyDriverManager();

    delete qlOpts;

    return res;
}
