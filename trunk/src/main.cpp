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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include <QtCore>
#include <QtGui>

#include <gdal_priv.h>

#include "CMainWindow.h"

int main(int argc, char ** argv)
{
    GDALAllRegister();

    QApplication theApp(argc,argv);
    QCoreApplication::setApplicationName("QLandkarteGT");
    QCoreApplication::setOrganizationName("QLandkarteGT");

    QSplashScreen *splash = new QSplashScreen(QPixmap(":/pics/splash.png"));
    splash->show();
    CMainWindow w;
    w.show();
    splash->finish(&w);
    delete splash;

    int res  = theApp.exec();
    return res;
}
