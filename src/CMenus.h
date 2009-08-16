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

#ifndef CACTIONGROUPPROVIDER_H_
#define CACTIONGROUPPROVIDER_H_
#include <QList>
#include <QAction>
#include <QPointer>
#include <QHash>
#include <QSet>
#include <QFlags>

class CActions;
class QWidget;
class QLabel;

class CMenus: public QObject
{
    Q_OBJECT
        Q_ENUMS(ActionGroupName)
        public:
        CMenus(QObject * parent);
        virtual ~CMenus();
        enum ActionGroupName
        {
            NoMenu,
            Map3DMenu,
            WptMenu,
            LiveLogMenu,
            OverlayMenu,
            MainMoreMenu,
            TrackMenu,
            RouteMenu,
            MapMenu,
            MainMenu
        };

        enum MenuContextName
        {
            LeftSideMenu= 0x1,
            ContextMenu = 0x2,
            MenuBarMenu = 0x4
        };

        Q_DECLARE_FLAGS(MenuContextNames, MenuContextName)

            void addAction(ActionGroupName group, QAction *action, bool force = false);
        void addAction(ActionGroupName group, const QString& actionName, bool force = false);

        void removeAction(ActionGroupName group, QAction *action);
        void removeAction(QAction *action);
        void switchToActionGroup(ActionGroupName group);
        CActions* getActions() {return actions;};
        QList<QAction *> *getActiveActions(ActionGroupName group = NoMenu);
        bool addActionsToMenu(QMenu *menu, MenuContextNames names = QFlags<CMenus::MenuContextName>(ContextMenu), ActionGroupName groupName = NoMenu);
        bool addActionsToWidget(QLabel *menu);
        QList<QAction *> getActiveActionsList(QObject *menu, MenuContextNames names , ActionGroupName groupName = NoMenu);
        signals:
        void stateChanged();
    private:
        QHash<QAction *, QList<QKeySequence> > actionsShortcuts;
        QSet<QAction *>  excludedActionForMenuBarMenu;
        QSet<QAction *>  controlledActions;
        ActionGroupName activeGroup;
        QHash<ActionGroupName, QList<QAction *> *> actionGroupHash;
        CActions* actions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CMenus::MenuContextNames)
#endif                           /* CMenus_H_ */
