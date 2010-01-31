//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#ifndef CACTIONS_H_
#define CACTIONS_H_
#include <QObject>
#include <QString>
#include <QPixmap>
#include <QPointer>
class QAction;
class CMenus;
class CCanvas;
class CActions : public QObject
{
    Q_OBJECT
        public:
        CActions(QObject *parent);
        virtual ~CActions();
        QAction *getAction(const QString& actionObjectName);
        const QString& getMenuTitle() { return menuTitle; };
        const QPixmap & getMenuPixmap( ) {return menuPixmap;};
    private:
        void createAction(const QString& shortCut,const char * icon,const QString& name, const QString& setObjectName, const QString& tooltip);
        QObject *parent;
        CMenus *actionGroup;
        QPointer<CCanvas>  canvas;
        QString menuTitle;
        QPixmap menuPixmap;
        void setMenuTitle(const QString &menuTitle);
        void setMenuPixmap(const QPixmap & menuPixmap);
    public slots:
        void funcSwitchToMain();
        void funcSwitchToMap();
        void funcSwitchToMap3D();

        void funcSwitchToWpt();
        void funcSwitchToTrack();
        void funcSwitchToRoute();

        void funcSwitchToLiveLog();
        void funcSwitchToOverlay();
        void funcSwitchToMainMore();
        void funcClearAll();
        void funcUploadAll();
        void funcDownloadAll();

        void funcMoveArea();
        void funcZoomArea();
        void funcCenterMap();

        void funcSelectArea();
        void funcEditMap();
        void funcSearchMap();
        void funcUploadMap();

        void funcCloseMap3D();
        void funcMap3DMode();
        void funcMap3DZoomPlus();
        void funcMap3DZoomMinus();
        void funcMap3DLighting();

        void funcNewWpt();
        void funcEditWpt();
        void funcMoveWpt();
        void funcImageWpt();
        void funcUploadWpt();
        void funcDownloadWpt();

        void funcCombineTrack();
        void funcEditTrack();
        void funcCutTrack();
        void funcSelTrack();
        void funcUploadTrack();
        void funcDownloadTrack();
        void funcTrackPurgeSelection();

        void funcDeleteTrackSelection();

        void funcUploadRoute();
        void funcDownloadRoute();

        void funcLiveLog();
        void funcLockMap();
        void funcAddWpt();

        void funcText();
        void funcTextBox();
        void funcDistance();

        void funcDiary();
        void funcColorPicker();
        void funcWorldBasemap();

        void funcZoomIn();
        void funcZoomOut();
        void funcMoveLeft();
        void funcMoveRight();
        void funcMoveUp();
        void funcMoveDown();
        void funcCopyToClipboard();
        void funcPasteFromClipboard();
        void funcRedo();
        void funcUndo();
};
#endif                           /* CACTIONS_H_ */
