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

#ifndef CMEGAMENU_H
#define CMEGAMENU_H

#include <QLabel>
#include <QPointer>

class CCanvas;
class QLabel;
class QGridLayout;

/// the left hand context sensitive menu
class CMegaMenu : public QLabel
{
    Q_OBJECT
    public:
        CMegaMenu(CCanvas * canvas);
        virtual ~CMegaMenu();

        void keyPressEvent(QKeyEvent * e);

        void switchByKeyWord(const QString& key);

    protected:
        void mousePressEvent(QMouseEvent * e);

    private:
        struct func_key_state_t
        {
            const char * icon;
            QString name;
            void (CMegaMenu::*func)();
            QString tooltip;
        };

        void switchState(const func_key_state_t statedef[11]);

        void funcSwitchToMain();
        void funcSwitchToMap();
        void funcSwitchToWpt();

        void funcMoveArea();
        void funcCenterMap();

        void funcSelectArea();

        void funcNewWpt();
        void funcUploadWpt();
        void funcDownloadWpt();

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

        const func_key_state_t * current;

        static const func_key_state_t fsMain[];
        static const func_key_state_t fsMap[];
        static const func_key_state_t fsWpt[];
};

#endif //CMEGAMENU_H

