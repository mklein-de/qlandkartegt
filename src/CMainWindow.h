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

#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>

class QSplitter;
class QLabel;
class QToolBox;
class CCanvas;
class CMegaMenu;
class CWptDB;
class CSearchDB;
class CResources;

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        CMainWindow();
        virtual ~CMainWindow();

        void setPositionInfo(const QString& info);

    protected:
        void keyPressEvent(QKeyEvent * e);

    private slots:
        void slotLoadMapSet();
        void slotCopyright();
        void slotToolBoxChanged(int idx);

    private:
        void setupMenuBar();
        /// horizontal main splitter holding the canvas and the tool view
        QSplitter * mainSplitter;
        /// the vertical splitter holding the tool views
        QSplitter * leftSplitter;
        /// the vertical splitter holding the canvas and track info view
        QSplitter * rightSplitter;

        /// left hand context sensitive menu
        CMegaMenu * megaMenu;

        /// left hand tool box
        QToolBox  * toolbox;
        /// the map canvas
        CCanvas * canvas;
        /// coordinate label
        QLabel * statusCoord;

        /// root path of all maps
        QString pathMaps;
        QString mapFile;

        CResources * resources;

        /// the waypoint data base
        CWptDB * wptdb;

        CSearchDB * searchdb;


};

extern CMainWindow * theMainWindow;

#endif //CMAINWINDOW_H

