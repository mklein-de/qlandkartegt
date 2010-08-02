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

#include "CGeoDB.h"
#include "config.h"
#include "WptIcons.h"

#include "CWptDB.h"
#include "CTrackDB.h"
#include "CRouteDB.h"
#include "COverlayDB.h"

#include <QtGui>
#include <QSqlQuery>
#include <QSqlError>


CGeoDB::CGeoDB(QTabWidget * tb, QWidget * parent)
    : QWidget(parent)
    , tabbar(tb)
{
    setupUi(this);
    setObjectName("GeoDB");

    tabbar->insertTab(eName,this, QIcon(":/icons/iconGeoDB16x16"),"");
    tabbar->setTabToolTip(tabbar->indexOf(this), tr("Manage your Geo Data Base"));

    itemWorkspace = new QTreeWidgetItem(treeWidget,eFolder);
    itemWorkspace->setText(eName, tr("Workspace"));
    itemWorkspace->setIcon(eName, QIcon(":/icons/iconGlobe16x16"));
    itemWorkspace->setToolTip(eName, tr("All items you see on the map."));
    itemWorkspace->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);

    itemWksWpt = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksWpt->setText(eName, tr("Waypoints"));
    itemWksWpt->setIcon(eName, QIcon(":/icons/iconWaypoint16x16"));
    itemWksWpt->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksWpt->setHidden(true);

    itemWksTrk = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksTrk->setText(eName, tr("Tracks"));
    itemWksTrk->setIcon(eName, QIcon(":/icons/iconTrack16x16"));
    itemWksTrk->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksTrk->setHidden(true);

    itemWksRte = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksRte->setText(eName, tr("Routes"));
    itemWksRte->setIcon(eName, QIcon(":/icons/iconRoute16x16"));
    itemWksRte->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksRte->setHidden(true);

    itemWksOvl = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksOvl->setText(eName, tr("Overlays"));
    itemWksOvl->setIcon(eName, QIcon(":/icons/iconOverlay16x16"));
    itemWksOvl->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksOvl->setHidden(true);

    itemPaperbin = new QTreeWidgetItem(treeWidget,eFolder);
    itemPaperbin->setText(eName, tr("Lost items"));
    itemPaperbin->setIcon(eName, QIcon(":/icons/iconDelete16x16"));
    itemPaperbin->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);

    itemDatabase = new QTreeWidgetItem(treeWidget,eFolder);
    itemDatabase->setText(eName, tr("Database"));
    itemDatabase->setIcon(eName, QIcon(":/icons/iconGeoDB16x16"));
    itemDatabase->setData(eName, eUserRoleDBKey, 1);
    itemDatabase->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemDatabase->setToolTip(eName, tr("All your data grouped by folders."));

    db = QSqlDatabase::addDatabase("QSQLITE","qlandkarte");
    db.setDatabaseName(QDir::home().filePath(CONFIGDIR "qlgt.db"));
    db.open();

    QSqlQuery query(db);

    if(!query.exec("SELECT version FROM versioninfo"))
    {
        initDB();
    }
    else if(query.next())
    {
        int version = query.value(0).toInt();
        if(version != DB_VERSION)
        {
            migrateDB(version);
        }
    }
    else
    {
        initDB();
    }

        contextMenuFolder = new QMenu(this);
    actEditDirComment = contextMenuFolder->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit comment"),this,SLOT(slotEditDirComment()));
    actAddDir = contextMenuFolder->addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("Add Folder"),this,SLOT(slotAddDir()));
    actDelDir = contextMenuFolder->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Del. Folder"),this,SLOT(slotDelDir()));


    setupTreeWidget();

    connect(treeWidget,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
    connect(treeWidget,SIGNAL(itemExpanded(QTreeWidgetItem *)),this,SLOT(slotItemExpanded(QTreeWidgetItem *)));
    connect(treeWidget,SIGNAL(itemClicked(QTreeWidgetItem *, int)),this,SLOT(slotItemClicked(QTreeWidgetItem *, int)));
    connect(treeWidget,SIGNAL(itemChanged(QTreeWidgetItem *, int)),this,SLOT(slotItemChanged(QTreeWidgetItem *, int)));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptDBChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotTrkDBChanged()));
    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotRteDBChanged()));
    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotOvlDBChanged()));

    slotWptDBChanged();
    slotTrkDBChanged();
    slotRteDBChanged();
    slotOvlDBChanged();

    itemWorkspace->setExpanded(true);
}

CGeoDB::~CGeoDB()
{

}


void CGeoDB::gainFocus()
{
    if(tabbar->currentWidget() != this)
    {
        tabbar->setCurrentWidget(this);
    }
}


void CGeoDB::initDB()
{
    qDebug() << "void CGeoDB::initDB()";
    QSqlQuery query(db);

    if(query.exec( "CREATE TABLE versioninfo ( version TEXT )"))
    {
        query.prepare( "INSERT INTO versioninfo (version) VALUES(:version)");
        query.bindValue(":version", DB_VERSION);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }
    }

    if(!query.exec( "CREATE TABLE folders ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date           DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           TEXT NOT NULL,"
        "name           TEXT NOT NULL,"
        "comment        TEXT"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }


    if(!query.exec( "CREATE TABLE items ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type           INTEGER,"
        "key            TEXT NOT NULL,"
        "date           DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           TEXT NOT NULL,"
        "name           TEXT NOT NULL,"
        "comment        TEXT,"
        "data           BLOB"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec("INSERT INTO folders (icon, name, comment) VALUES ('', 'database', '')"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE folder2folder ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "parent         INTEGER NOT NULL,"
        "child          INTEGER NOT NULL,"
        "FOREIGN KEY(parent) REFERENCES folders(id),"
        "FOREIGN KEY(child) REFERENCES folders(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE folder2item ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "parent         INTEGER NOT NULL,"
        "child          INTEGER NOT NULL,"
        "FOREIGN KEY(parent) REFERENCES folders(id),"
        "FOREIGN KEY(child) REFERENCES items(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

}

void CGeoDB::migrateDB(int version)
{
    qDebug() << "void CGeoDB::migrateDB(int version)" << version;
    QSqlQuery query(db);

    query.prepare( "UPDATE versioninfo set version=:version");
    query.bindValue(":version", version);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }


}

void CGeoDB::setupTreeWidget()
{
    QList<QTreeWidgetItem*> children = itemDatabase->takeChildren();
    QTreeWidgetItem * child;
    foreach(child, children)
    {
        delete child;
    }

    queryChildrenFromDB(itemDatabase, 2);
    itemDatabase->setExpanded(true);
}

void CGeoDB::slotWptDBChanged()
{

}

void CGeoDB::slotTrkDBChanged()
{

}

void CGeoDB::slotRteDBChanged()
{

}

void CGeoDB::slotOvlDBChanged()
{

}

void CGeoDB::slotAddDir()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    QString name = QInputDialog::getText(0, tr("Folder name..."), tr("Name of new folders"));
    if(name.isEmpty())
    {
        return;
    }

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Would you like to add a comment?"));

    addFolder(item, name, comment);
}

void CGeoDB::slotDelDir()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    QMessageBox::StandardButton but = QMessageBox::question(0, tr("Delete folders..."), tr("You are sure you want to delete '%1' and all items below?").arg(item->text(eName)), QMessageBox::Ok|QMessageBox::Abort);
    if(but == QMessageBox::Ok)
    {
        delFolder(item, true);
    }
}

void CGeoDB::slotEditDirComment()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    quint64 itemId = item->data(eName, eUserRoleDBKey).toULongLong();

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Edit comment?"), QLineEdit::Normal,item->toolTip(eName));
    if(comment.isEmpty())
    {
        return;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE folders SET comment=:comment WHERE id=:id");
    query.bindValue(":comment", comment);
    query.bindValue(":id", itemId);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    /// @todo fixit
//    int i;
//    const int size = itemDatabase->childCount();
//    for(i = 0; i < size; i++)
//    {
//        item = itemDatabase->child(i);
//        if(item->data(eName, eUserRoleDBKey).toULongLong() == itemId)
//        {
//            item->setToolTip(eName, comment);
//        }
//    }
}

void CGeoDB::slotContextMenu(const QPoint& pos)
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    QTreeWidgetItem * top  = item->parent();
    while(top && top->parent()) top = top->parent();
    if(top == 0) top = item;

    if(top == itemWorkspace)
    {

    }
    else if(top == itemPaperbin)
    {

    }
    else if(top == itemDatabase)
    {
        if(item->type() == eFolder)
        {
            if(item == itemDatabase)
            {
                actDelDir->setEnabled(false);
                actEditDirComment->setEnabled(false);
            }
            else
            {
                actDelDir->setEnabled(true);
                actEditDirComment->setEnabled(true);
            }

            QPoint p = treeWidget->mapToGlobal(pos);
            contextMenuFolder->exec(p);
        }
    }
}

void CGeoDB::slotItemExpanded(QTreeWidgetItem * item)
{
    if(item->childCount() == 0)
    {
        queryChildrenFromDB(item, 2);
    }
}

void CGeoDB::slotItemClicked(QTreeWidgetItem * item, int column)
{

}

void CGeoDB::slotItemChanged(QTreeWidgetItem * item, int column)
{
    quint64 itemId = item->data(eName, eUserRoleDBKey).toULongLong();
    QString itemText = item->text(eName);

    if(itemText.isEmpty())
    {
        return;
    }

    if(column == eName)
    {
        QSqlQuery query(db);
        query.prepare("UPDATE folders SET name=:name WHERE id=:id");
        query.bindValue(":name", itemText);
        query.bindValue(":id", itemId);

        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
            return;
        }
    }

    /// @todo fixit
//    int i;
//    const int size = itemDatabase->childCount();
//    for(i = 0; i < size; i++)
//    {
//        item = itemDatabase->child(i);
//        if(item->data(eName, eUserRoleDBKey).toULongLong() == itemId)
//        {
//            item->setText(eName, itemText);
//        }
//    }
}

void CGeoDB::queryChildrenFromDB(QTreeWidgetItem * parent, int levels)
{
    QSqlQuery query(db);
    quint64 parentId = parent->data(eName, eUserRoleDBKey).toULongLong();

    if(parentId == 0)
    {
        return;
    }

    query.prepare("SELECT child FROM folder2folder WHERE parent = :id");
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

        QTreeWidgetItem * item = new QTreeWidgetItem(parent, eFolder);
        item->setData(eName, eUserRoleDBKey, childId);
        item->setIcon(eName, QIcon(query2.value(0).toString()));
        item->setText(eName, query2.value(1).toString());
        item->setToolTip(eName, query2.value(2).toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if(parentId != 1)
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        if(levels)
        {
            queryChildrenFromDB(item, levels - 1);
        }
    }
}

void CGeoDB::addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment)
{

    QSqlQuery query(db);
    quint64 parentId = parent->data(eName, eUserRoleDBKey).toULongLong();
    quint64 childId = 0;

    query.prepare("INSERT INTO folders (icon, name, comment) VALUES (:icon, :name, :comment)");
    if(parentId == 1)
    {
        query.bindValue(":icon", ":/icons/iconFolderBlue16x16");
    }
    else
    {
        query.bindValue(":icon", ":/icons/iconFolderGreen16x16");
    }
    query.bindValue(":name", name);
    query.bindValue(":comment", comment);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    if(!query.exec("SELECT last_insert_rowid() from folders"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }
    query.next();
    childId = query.value(0).toULongLong();
    if(childId == 0)
    {
        qDebug() << "childId equals 0. bad.";
        return;
    }

    query.prepare("INSERT INTO folder2folder (parent, child) VALUES (:parent, :child)");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    QTreeWidgetItem * item = new QTreeWidgetItem(parent, eFolder);
    item->setData(eName, eUserRoleDBKey, childId);
    if(parentId == 1)
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderBlue16x16"));
    }
    else
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderGreen16x16"));
        item->setCheckState(0, Qt::Unchecked);
    }
    item->setText(eName, name);
    item->setToolTip(eName, comment);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    /// @todo fixit
//    int i;
//    const int size = itemDatabase->childCount();
//    for(i = 0; i < size; i++)
//    {
//        QTreeWidgetItem * item2 = itemDatabase->child(i);
//        if(item2->data(eName, eUserRoleDBKey).toULongLong() == parentId)
//        {
//            item2->addChild(item->clone());
//        }
//    }

}

void CGeoDB::delFolder(QTreeWidgetItem * item, bool isTopLevel)
{
    int i;
    const int size   = item->childCount();
    quint64 itemId   = item->data(eName, eUserRoleDBKey).toULongLong();
    quint64 parentId = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    QSqlQuery query(db);
    // delete this particular relation first
    query.prepare("DELETE FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", itemId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    // next query if the item is used as child in any other relation
    query.prepare("SELECT * FROM folder2folder WHERE child=:id");
    query.bindValue(":id", itemId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    // if there is no other relation delete the children, too.
    if(!query.next())
    {
        for(i = 0; i < size; i++)
        {
            delFolder(item->child(i), false);
        }

        // and remove the folder
        query.prepare("DELETE FROM folders WHERE id=:id");
        query.bindValue(":id", itemId);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }
    }

    if(isTopLevel)
    {
        /// @todo fixit
//        int i;
//        const int size = itemDatabase->childCount();
//        qDebug() << size;
//        for(i = 0; i < size; i++)
//        {
//            QTreeWidgetItem * item2 = itemDatabase->child(i);
//            qDebug() << item2->data(eName, eUserRoleDBKey).toULongLong() << itemId;
//            if(item2->data(eName, eUserRoleDBKey).toULongLong() == itemId)
//            {
//                item2->parent()->removeChild(item2);
//                delete item2;
//            }
//        }
    }
}

#if 0
void CGeoDB::setupTreeWidget()
{
    queryChildrenFromDB(itemDatabase, 2);
    itemDatabase->setExpanded(true);
}

void CGeoDB::queryChildrenFromDB(QTreeWidgetItem * parent, int levels)
{
    QSqlQuery query(db);

    QString key = parent->data(eName, eUserRoleKey).toString();
    QString cmd;

    if(key == "NULL")
    {
        cmd = "SELECT id, icon, name, comment FROM folders WHERE NOT parent";
    }
    else
    {
        cmd = "SELECT id, icon, name, comment FROM folders WHERE parent = " + key;
    }

    if(!query.exec(cmd))
    {
        qDebug() << query.lastError();
        return;
    }

    while(query.next())
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(parent, eFolder);
        item->setData(eName, eUserRoleKey, query.value(0).toULongLong());
        item->setIcon(eName, QIcon(query.value(1).toString()));
        item->setText(eName, query.value(2).toString());
        item->setToolTip(eName ,query.value(3).toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if(key != "NULL")
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        if(levels)
        {
            queryChildrenFromDB(item, levels - 1);
        }
    }
}



void CGeoDB::slotAddDir()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    QString name = QInputDialog::getText(0, tr("Folder name..."), tr("Name of new folders"));
    if(name.isEmpty())
    {
        return;
    }

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Would you like to add a comment?"));

    addFolder(item, name, comment);
}

void CGeoDB::slotDelDir()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    QMessageBox::StandardButton but = QMessageBox::question(0, tr("Delete folders..."), tr("You are sure you want to delete '%1' and all items below?").arg(item->text(eName)), QMessageBox::Ok|QMessageBox::Abort);

    if(but == QMessageBox::Ok)
    {
        delFolder(item, true);
    }
}

void CGeoDB::slotEditDirComment()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Edit comment?"), QLineEdit::Normal,item->toolTip(eName));
    if(comment.isEmpty())
    {
        return;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE folders SET comment=:comment WHERE id=:key");
    query.bindValue(":comment", comment);
    query.bindValue(":key", item->data(eName, eUserRoleKey).toULongLong());

    if(!query.exec())
    {
        qDebug() << "errmsg=" << query.lastError().text();
        return;
    }
    item->setToolTip(eName, comment);
}

void CGeoDB::slotContextMenu(const QPoint& pos)
{
    QTreeWidgetItem * item = treeWidget->currentItem();

    if(item->type() == eFolder)
    {

        if(item == itemDatabase || item == itemWorkspace || item == itemPaperbin)
        {
            actDelDir->setEnabled(false);
            actEditDirComment->setEnabled(false);
        }
        else
        {
            actDelDir->setEnabled(true);
            actEditDirComment->setEnabled(true);
        }

        if(item == itemWorkspace || item == itemPaperbin)
        {
            actAddDir->setEnabled(false);
        }
        else
        {
            actAddDir->setEnabled(true);
        }


        QPoint p = treeWidget->mapToGlobal(pos);
        contextMenuFolder->exec(p);
    }
}

void CGeoDB::slotItemExpanded(QTreeWidgetItem * item)
{
    if(item->childCount() == 0)
    {
        queryChildrenFromDB(item, 2);
    }
}

void CGeoDB::slotItemClicked(QTreeWidgetItem * item, int column)
{

}

void CGeoDB::slotItemChanged(QTreeWidgetItem * item, int column)
{
    quint64 key = item->data(eName, eUserRoleKey).toULongLong();
    if(column == eName)
    {
        QSqlQuery query(db);
        query.prepare("UPDATE folders SET name=:name WHERE id=:key");
        query.bindValue(":name", item->text(eName));
        query.bindValue(":key", key);

        if(!query.exec())
        {
            qDebug() << "errmsg=" << query.lastError().text();
            return;
        }
    }
}


void CGeoDB::slotWptDBChanged()
{
    QTreeWidgetItem * item;
    QList<QTreeWidgetItem *> children = itemWksWpt->takeChildren();
    foreach(item, children)
    {
        delete item;
    }
    children.clear();

    CWptDB::keys_t key;
    QList<CWptDB::keys_t> keys = CWptDB::self().keys();
    foreach(key, keys)
    {
        item = new QTreeWidgetItem(eWpt);
        item->setText(eName, key.name);
        item->setIcon(eName, getWptIconByName(key.icon));
        item->setToolTip(eName, key.comment);
        item->setData(eName, eUserRoleQlKey, key.key);
        children << item;
    }

    itemWksWpt->addChildren(children);
    itemWksWpt->setHidden(children.size() == 0);
    itemWksWpt->setExpanded(children.size() != 0);
}

void CGeoDB::slotTrkDBChanged()
{
    QTreeWidgetItem * item;
    QList<QTreeWidgetItem *> children = itemWksTrk->takeChildren();
    foreach(item, children)
    {
        delete item;
    }
    children.clear();

    CTrackDB::keys_t key;
    QList<CTrackDB::keys_t> keys = CTrackDB::self().keys();

    foreach(key, keys)
    {
        item = new QTreeWidgetItem(eTrk);
        item->setText(eName, key.name);
        item->setIcon(eName, key.icon);
        item->setToolTip(eName, key.comment);
        item->setData(eName, eUserRoleQlKey, key.key);
        children << item;
    }
    itemWksTrk->addChildren(children);
    itemWksTrk->setHidden(children.size() == 0);
    itemWksTrk->setExpanded(children.size() != 0);
}

void CGeoDB::slotRteDBChanged()
{

}

void CGeoDB::slotOvlDBChanged()
{
    QTreeWidgetItem * item;
    QList<QTreeWidgetItem *> children = itemWksOvl->takeChildren();
    foreach(item, children)
    {
        delete item;
    }
    children.clear();

    COverlayDB::keys_t key;
    QList<COverlayDB::keys_t> keys = COverlayDB::self().keys();

    foreach(key, keys)
    {
        item = new QTreeWidgetItem(eOvl);
        item->setText(eName, key.name);
        item->setIcon(eName, key.icon);
        item->setToolTip(eName, key.comment);
        item->setData(eName, eUserRoleQlKey, key.key);
        children << item;
    }
    itemWksOvl->addChildren(children);
    itemWksOvl->setHidden(children.size() == 0);
    itemWksOvl->setExpanded(children.size() != 0);
}


void CGeoDB::addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment)
{
    QSqlQuery query(db);

    QString parentKey = parent->data(eName, eUserRoleKey).toString();

    query.prepare("INSERT INTO folders (parent, icon, name, comment) VALUES (:parent, :icon, :name, :comment)");
    query.bindValue(":parent", parentKey);
    if(parentKey == "NULL")
    {
        query.bindValue(":icon", ":/icons/iconFolderBlue16x16");
    }
    else
    {
        query.bindValue(":icon", ":/icons/iconFolderGreen16x16");
    }
    query.bindValue(":name", name);
    query.bindValue(":comment", comment);

    if(!query.exec())
    {
        qDebug() << "errmsg=" << query.lastError().text();;
        return;
    }

    if(!query.exec("SELECT last_insert_rowid() from folders"))
    {
        qDebug() << "errmsg=" << query.lastError().text();
        return;
    }


    quint64 key = 0;
    while(query.next())
    {
        key = query.value(0).toULongLong();
    }
    if(key == 0)
    {
        qDebug() << "key equals 0. bad.";
    }


    QTreeWidgetItem * item = new QTreeWidgetItem(parent, eFolder);
    item->setText(eName, name);
    item->setToolTip(eName ,comment);
    if(parentKey == "NULL")
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderBlue16x16"));
    }
    else
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderGreen16x16"));
        item->setCheckState(0, Qt::Unchecked);
    }
    item->setData(eName, eUserRoleKey, key);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}


void CGeoDB::delFolder(QTreeWidgetItem * item, bool isTopLevel)
{
    int i;
    const int size = item->childCount();
    quint64 key = item->data(eName, eUserRoleKey).toULongLong();


    for(i = 0; i < size; i++)
    {
        delFolder(item->child(i), false);
    }


    QSqlQuery query(db);
    query.prepare("DELETE FROM folders WHERE id=:key");
    query.bindValue(":key", key);

    if(!query.exec())
    {
        qDebug() << "errmsg=" << query.lastError();
    }

    if(isTopLevel)
    {
        item->parent()->removeChild(item);
        delete item;
    }
}
#endif
