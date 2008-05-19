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
class QTabWidget;
class CCanvas;
class CMegaMenu;
class CMapDB;
class CWptDB;
class CTrackDB;
class CSearchDB;
class CDiaryDB;
class CResources;
class CTabWidget;
class CLiveLogDB;

class CMainWindow : public QMainWindow
{
    Q_OBJECT;
    public:
        CMainWindow();
        virtual ~CMainWindow();

        void setPositionInfo(const QString& info);

        CCanvas * getCanvas(){return canvas;}

        CTabWidget * getCanvasTab(){return canvasTab;}

        void setTempWidget(QWidget * w);

        void clear();
        void clearAll();

        const QString& getCurrentFilename(){return wksFile;}

    protected:
        void keyPressEvent(QKeyEvent * e);
        void closeEvent(QCloseEvent * e);

    private slots:
        void slotLoadMapSet();
        void slotCopyright();
        void slotToolBoxChanged(int idx);
        void slotConfig();
        void slotLoadData();
        void slotAddData();
        void slotSaveData();
        void slotPrint();
        void slotPrintPreview();
        void slotModified();
        void slotDataChanged();
        void slotOpenLink(const QString& link);

    private:
        void setupMenuBar();
        void loadData(QString& filename, const QString& filter);
        void setTitleBar();
        bool maybeSave();
        void saveData(const QString& filename, const QString& filter);
        /// horizontal main splitter holding the canvas and the tool view
        QSplitter * mainSplitter;
        /// the vertical splitter holding the tool views
        QSplitter * leftSplitter;
        /// the vertical splitter holding the canvas and track info view
        QSplitter * rightSplitter;

        /// left hand context sensitive menu
        CMegaMenu * megaMenu;

        /// left hand tool box
        QTabWidget  * tabbar;

        CTabWidget * canvasTab;

        /// the map canvas
        CCanvas * canvas;
        /// coordinate label
        QLabel * statusCoord;

        /// root path of geo data
        QString pathData;

        CResources * resources;

        /// the waypoint data base
        CWptDB * wptdb;
        /// the search database
        CSearchDB * searchdb;
        /// the map data base
        CMapDB * mapdb;
        /// the track data base
        CTrackDB * trackdb;
        /// diary database
        CDiaryDB * diarydb;

        CLiveLogDB * livelogdb;

        /// the current loaded geo data (workspace) file
        QString wksFile;
        /// true if any data has been changed
        bool modified;

        /// project summary
        QLabel * summary;

};

extern CMainWindow * theMainWindow;
#endif                           //CMAINWINDOW_H
