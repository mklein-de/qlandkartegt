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
        void slotContextMenu(const QPoint&);

        void slotItemExpanded(QTreeWidgetItem * item);
        void slotItemClicked(QTreeWidgetItem * item, int column);
        void slotItemChanged(QTreeWidgetItem * item, int column);


    private:
        enum EntryType_e {eDirectory};
        enum ColumnType_e {eName = 0};
        enum UserRoles_e {eUserRoleKey = Qt::UserRole};

        void initDB();
        void migrateDB(int version);
        void queryChildrenFromDB(QTreeWidgetItem * parent, int levels);
        void setupTreeWidget();
        void addDirectory(QTreeWidgetItem * parent, const QString& name, const QString& comment);

        QTabWidget * tabbar;
        QTreeWidgetItem * itemDatabase;
        QTreeWidgetItem * itemWorkspace;

        QSqlDatabase db;

        QMenu * contextMenuDirectory;
        QAction * actAddDir;
        QAction * actDelDir;
};

#endif //CGEODB_H

