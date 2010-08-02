/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CGEODB_H
#define CGEODB_H

#include <QWidget>
#include <QSqlDatabase>
#include "ui_IGeoToolWidget.h"

class QTabWidget;
class QTreeWidgetItem;
class QMenu;

class CGeoDB : public QWidget, private Ui::IGeoToolWidget
{
    Q_OBJECT;
    public:
        CGeoDB(QTabWidget * tb, QWidget * parent);
        virtual ~CGeoDB();

        void gainFocus();

    private slots:
        void slotAddDir();
        void slotDelDir();
        void slotEditDirComment();
        void slotContextMenu(const QPoint&);

        void slotItemExpanded(QTreeWidgetItem * item);
        void slotItemClicked(QTreeWidgetItem * item, int column);
        void slotItemChanged(QTreeWidgetItem * item, int column);

        void slotWptDBChanged();
        void slotTrkDBChanged();
        void slotRteDBChanged();
        void slotOvlDBChanged();

    private:
        enum EntryType_e {eFolder, eTypFolder, eWpt, eTrk, eRte, eOvl};
        enum ColumnType_e {eName = 0};
        enum UserRoles_e {eUserRoleKey = Qt::UserRole, eUserRoleQlKey = Qt::UserRole+1};

        void initDB();
        void migrateDB(int version);
        void queryChildrenFromDB(QTreeWidgetItem * parent, int levels);

        void setupTreeWidget();
        void addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment);
        void delFolder(QTreeWidgetItem * item, bool isTopLevel);

        QTabWidget * tabbar;
        QTreeWidgetItem * itemDatabase;
        QTreeWidgetItem * itemPaperbin;
        QTreeWidgetItem * itemWorkspace;

        QTreeWidgetItem * itemWksWpt;
        QTreeWidgetItem * itemWksTrk;
        QTreeWidgetItem * itemWksRte;
        QTreeWidgetItem * itemWksOvl;

        QSqlDatabase db;

        QMenu * contextMenuFolder;
        QAction * actAddDir;
        QAction * actDelDir;
        QAction * actEditDirComment;
};

#endif //CGEODB_H

