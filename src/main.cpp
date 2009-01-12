/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include <QtCore>
#include <QtGui>

#include <gdal_priv.h>
#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif

#include "CMainWindow.h"
// #include "CGarminTyp.h"

int main(int argc, char ** argv)
{
    GDALAllRegister();

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
    free(ptr);

    pj_free(pjWGS84);
    pj_free(pjGK);
    }

    QDir path(QDir::home().filePath(".config/QLandkarteGT/"));
    if(!path.exists()){
        path.mkpath("./");
    }

    QApplication theApp(argc,argv);

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
    free(ptr);

    pj_free(pjWGS84);
    pj_free(pjGK);
    }

    QCoreApplication::setApplicationName("QLandkarteGT");
    QCoreApplication::setOrganizationName("QLandkarteGT");

#ifdef WIN32
	QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
	qDebug() << QCoreApplication::applicationDirPath();
	qDebug() << QCoreApplication::libraryPaths();
#endif

    QSplashScreen *splash = new QSplashScreen(QPixmap(":/pics/splash.png"));
    splash->show();
    CMainWindow w;
    w.show();
    splash->finish(&w);
    delete splash;

//     CGarminTyp typ("/home/oeichler/Desktop/teddy.typ",0);

    int res  = theApp.exec();

    GDALDestroyDriverManager();
    return res;
}
