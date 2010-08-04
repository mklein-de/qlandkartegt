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
#include "CDlgSelGeoDBFolder.h"
#include "CGeoDB.h"

#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

CDlgSelGeoDBFolder::CDlgSelGeoDBFolder(QSqlDatabase& db, quint64& result)
    : db(db)
    , result(result)
{
    result = 0;
    setupUi(this);

    QTreeWidgetItem item;
    item.setData(CGeoDB::eName, CGeoDB::eUserRoleDBKey, 1);
    queryChildrenFromDB(&item);


    treeWidget->addTopLevelItems(item.takeChildren());
    treeWidget->expandAll();
}

CDlgSelGeoDBFolder::~CDlgSelGeoDBFolder()
{

}


void CDlgSelGeoDBFolder::accept()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    result = item->data(CGeoDB::eName, CGeoDB::eUserRoleDBKey).toULongLong();

    QDialog::accept();
}


void CDlgSelGeoDBFolder::queryChildrenFromDB(QTreeWidgetItem * parent)
{
    QSqlQuery query(db);
    quint64 parentId = parent ? parent->data(CGeoDB::eName, CGeoDB::eUserRoleDBKey).toULongLong() : 1;

    if(parentId == 0)
    {
        return;
    }

    query.prepare("SELECT t1.child FROM folder2folder AS t1, folders AS t2 WHERE t1.parent = :id AND t2.id = t1.child ORDER BY t2.name");
    query.bindValue(":id", parentId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();

        QSqlQuery query2(db);
        query2.prepare("SELECT icon, name, comment FROM folders WHERE id = :id");
        query2.bindValue(":id", childId);
        if(!query2.exec())
        {
            qDebug() << query2.lastQuery();
            qDebug() << query2.lastError();
            continue;
        }
        query2.next();

        QTreeWidgetItem * item = new QTreeWidgetItem(parent, CGeoDB::eFolder);
        item->setData(CGeoDB::eName, CGeoDB::eUserRoleDBKey, childId);
        item->setIcon(CGeoDB::eName, QIcon(query2.value(0).toString()));
        item->setText(CGeoDB::eName, query2.value(1).toString());
        item->setToolTip(CGeoDB::eName, query2.value(2).toString());

        queryChildrenFromDB(item);
    }

    treeWidget->header()->setResizeMode(CGeoDB::eName,QHeaderView::ResizeToContents);
}
