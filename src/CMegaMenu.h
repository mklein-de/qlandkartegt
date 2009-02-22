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

#ifndef CMEGAMENU_H
#define CMEGAMENU_H

#include <QLabel>
#include <QPointer>
#include <QVector>

class CCanvas;
class QLabel;
class QGridLayout;

/// the left hand context sensitive menu
class CMegaMenu : public QLabel
{
    Q_OBJECT;
    public:
        virtual ~CMegaMenu();

        static CMegaMenu& self(){return *m_self;}

        void keyPressEvent(QKeyEvent * e);

        void switchByKeyWord(const QString& key);

    protected slots:
        void slotEnable(){setEnabled(true);}

    protected:
        void mousePressEvent(QMouseEvent * e);

    private:
        friend class CMainWindow;
        CMegaMenu(CCanvas * canvas);

        struct func_key_state_t
        {
            func_key_state_t()
            {
                icon = 0;
                name = tr("-");
                func = 0;
                tooltip = tr("-");
            }
/*            func_key_state_t(const func_key_state_t& f)
            {
                icon = f.icon;
                name = f.name;
                func = f.func;
                tooltip = f.tooltip;
            }*/
            func_key_state_t(const char * icon,const QString& name, void (CMegaMenu::*func)(), const QString& tooltip)
                : icon(icon), name(name), func(func), tooltip(tooltip){}
            const char * icon;
            QString name;
            void (CMegaMenu::*func)();
            QString tooltip;
        };

        void switchState(const QVector<func_key_state_t>& statedef);

        void funcSwitchToMain();
        void funcSwitchToMap();
#ifdef PLOT_3D
        void funcSwitchToMap3D();
#endif
        void funcSwitchToWpt();
        void funcSwitchToTrack();

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

#ifdef PLOT_3D
        void funcCloseMap3D();
        void funcMap3DMode();
        void funcMap3DZoomPlus();
        void funcMap3DZoomMinus();
        void funcMap3DLighting();
#endif

        void funcNewWpt();
        void funcEditWpt();
        void funcMoveWpt();
        void funcUploadWpt();
        void funcDownloadWpt();

        void funcCombineTrack();
        void funcEditTrack();
        void funcCutTrack();
        void funcSelTrack();
        void funcUploadTrack();
        void funcDownloadTrack();

        void funcLiveLog();
        void funcLockMap();
        void funcAddWpt();

        void funcText();
        void funcTextBox();
        void funcDistance();

        void funcDiary();
        void funcColorPicker();

        static CMegaMenu * m_self;

        QPointer<CCanvas>  canvas;

        QLabel * menuTitle;

        QGridLayout * layout;
        QLabel * keyEsc;
        QLabel * keyF1;
        QLabel * keyF2;
        QLabel * keyF3;
        QLabel * keyF4;
        QLabel * keyF5;
        QLabel * keyF6;
        QLabel * keyF7;
        QLabel * keyF8;
        QLabel * keyF9;
        QLabel * keyF10;

        QLabel * icons[11];

        QLabel * names[11];

        QVector<func_key_state_t>& current;

        QVector<func_key_state_t> fsMain;
        QVector<func_key_state_t> fsMap;
        QVector<func_key_state_t> fsMap3D;
        QVector<func_key_state_t> fsWpt;
        QVector<func_key_state_t> fsTrack;
        QVector<func_key_state_t> fsLiveLog;
        QVector<func_key_state_t> fsOverlay;
        QVector<func_key_state_t> fsMainMore;

//         static const func_key_state_t fsMain[];
//         static const func_key_state_t fsMap[];
//         static const func_key_state_t fsMap3D[];
//         static const func_key_state_t fsWpt[];
//         static const func_key_state_t fsTrack[];
//         static const func_key_state_t fsLiveLog[];
//         static const func_key_state_t fsOverlay[];
//         static const func_key_state_t fsMainMore[];
};
#endif                           //CMEGAMENU_H
