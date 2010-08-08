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
#include "CWpt.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CRouteDB.h"
#include "COverlayDB.h"
#include "COverlayText.h"
#include "COverlayTextBox.h"
#include "COverlayDistance.h"
#include "CDlgSelGeoDBFolder.h"

#include "CQlb.h"

#include <QtGui>
#include <QSqlQuery>
#include <QSqlError>


#define QUERY_EXEC(cmd) \
if(!query.exec())\
{\
    qDebug() << query.lastQuery();\
    qDebug() << query.lastError();\
    cmd;\
}\

class CGeoDBInternalEditLock
{
    public:
        CGeoDBInternalEditLock(CGeoDB * db) : db(db){db->isInternalEdit += 1;}
        ~CGeoDBInternalEditLock(){db->isInternalEdit -= 1;}
    private:
        CGeoDB * db;
};

CGeoDB::CGeoDB(QTabWidget * tb, QWidget * parent)
    : QWidget(parent)
    , tabbar(tb)
    , isInternalEdit(0)
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

    itemLostFound = new QTreeWidgetItem(treeWidget,eFolder);
    itemLostFound->setText(eName, tr("Lost & Found"));
    itemLostFound->setIcon(eName, QIcon(":/icons/iconDelete16x16"));
    itemLostFound->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemLostFound->setToolTip(eName, tr("All items that lost their parent folder as you deleted it."));

    itemDatabase = new QTreeWidgetItem(treeWidget,eFolder);
    itemDatabase->setText(eName, tr("Database"));
    itemDatabase->setIcon(eName, QIcon(":/icons/iconGeoDB16x16"));
    itemDatabase->setData(eName, eUserRoleDBKey, 1);
    itemDatabase->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemDatabase->setToolTip(eName, tr("All your data grouped by folders."));

    timeoutCheckState = new QTimer(this);
    timeoutCheckState->setSingleShot(true);
    connect(timeoutCheckState, SIGNAL(timeout()), this, SLOT(slotTimeoutCheckState()));

    db = QSqlDatabase::addDatabase("QSQLITE","qlandkarte");
    db.setDatabaseName(QDir::home().filePath(CONFIGDIR "qlgt.db"));
    db.open();

    QSqlQuery query(db);

    if(!query.exec("PRAGMA locking_mode=EXCLUSIVE")) {
      return;
    }

    if(!query.exec("PRAGMA temp_store=MEMORY")) {
      return;
    }

    if(!query.exec("PRAGMA default_cache_size=50")) {
      return;
    }

    if(!query.exec("PRAGMA page_size=8192")) {
      return;
    }


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

    contextMenuFolder   = new QMenu(this);
    actEditDirComment   = contextMenuFolder->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit comment"),this,SLOT(slotEditFolder()));
    actAddDir           = contextMenuFolder->addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("New folder"),this,SLOT(slotAddFolder()));
    actDelDir           = contextMenuFolder->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Delete"),this,SLOT(slotDelFolder()));
    actCopyDir          = contextMenuFolder->addAction(QPixmap(":/icons/editcopy"), tr("Copy..."), this, SLOT(slotCopyFolder()));
    actMoveDir          = contextMenuFolder->addAction(QPixmap(":/icons/iconWptMove16x16"), tr("Move..."), this, SLOT(slotMoveFolder()));

    contextMenuItem     = new QMenu(this);
    actCopyItem         = contextMenuItem->addAction(QPixmap(":/icons/editcopy"), tr("Copy..."), this, SLOT(slotCopyItems()));
    actMoveItem         = contextMenuItem->addAction(QPixmap(":/icons/iconWptMove16x16"), tr("Move..."), this, SLOT(slotMoveItems()));
    actDelItem          = contextMenuItem->addAction(QPixmap(":/icons/iconDelete16x16"), tr("Delete"), this, SLOT(slotDelItems()));

    contextMenuWks      = new QMenu(this);
    actAddToDB          = contextMenuWks->addAction(QPixmap(":/icons/iconAdd16x16"), tr("Add to database..."), this, SLOT(slotAddItems()));
    actSaveToDB         = contextMenuWks->addAction(QPixmap(":/icons/iconFileSave16x16"), tr("Save to database..."), this, SLOT(slotSaveItems()));

    contextMenuLost     = new QMenu(this);
    actMoveLost         = contextMenuLost->addAction(QPixmap(":/icons/iconWptMove16x16"), tr("Move..."), this, SLOT(slotMoveLost()));
    actDelLost          = contextMenuLost->addAction(QPixmap(":/icons/iconDelete16x16"), tr("Delete"), this, SLOT(slotDelLost()));

    setupTreeWidget();

    connect(treeWidget,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
    connect(treeWidget,SIGNAL(itemExpanded(QTreeWidgetItem *)),this,SLOT(slotItemExpanded(QTreeWidgetItem *)));
    connect(treeWidget,SIGNAL(itemChanged(QTreeWidgetItem *, int)),this,SLOT(slotItemChanged(QTreeWidgetItem *, int)));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptDBChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotTrkDBChanged()));
    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotRteDBChanged()));
    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotOvlDBChanged()));

    slotWptDBChanged();
    slotTrkDBChanged();
    slotRteDBChanged();
    slotOvlDBChanged();

    connect(&CWptDB::self(), SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(&CTrackDB::self(), SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(&CRouteDB::self(), SIGNAL(sigModified()), this, SLOT(slotModified()));
    connect(&COverlayDB::self(), SIGNAL(sigModified()), this, SLOT(slotModified()));

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
        QUERY_EXEC();
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
        "data           BLOB NOT NULL"
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
    QUERY_EXEC();
}

void CGeoDB::setupTreeWidget()
{
    CGeoDBInternalEditLock lock(this);

    QList<QTreeWidgetItem*> children = itemDatabase->takeChildren();
    QTreeWidgetItem * child;
    foreach(child, children)
    {
        delete child;
    }

    queryChildrenFromDB(itemDatabase, 2);
    itemDatabase->setExpanded(true);

    updateLostFound();
}

void CGeoDB::slotWptDBChanged()
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    qDeleteAll(itemWksWpt->takeChildren());

    CWptDB& wptdb = CWptDB::self();
    CWptDB::keys_t key;
    QList<CWptDB::keys_t> keys = wptdb.keys();

    foreach(key, keys)
    {

        CWpt * wpt = wptdb.getWptByKey(key.key);
        if(!wpt || wpt->sticky)
        {
            continue;
        }

        query.prepare("SELECT id FROM items WHERE key=:key");
        query.bindValue(":key", key.key);
        QUERY_EXEC();

        QTreeWidgetItem * item = new QTreeWidgetItem(itemWksWpt, eWpt);
        if(query.next())
        {
            item->setData(eName, eUserRoleDBKey, query.value(0));
            item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
        }
        item->setData(eName, eUserRoleQLKey, key.key);
        item->setIcon(eName, getWptIconByName(key.icon));
        item->setText(eName, key.name);
        item->setToolTip(eName, key.comment);
    }

    itemWksWpt->setHidden(itemWksWpt->childCount() == 0);

}

void CGeoDB::slotTrkDBChanged()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);
    qDeleteAll(itemWksTrk->takeChildren());

    CTrackDB& trkdb = CTrackDB::self();
    CTrackDB::keys_t key;
    QList<CTrackDB::keys_t> keys = trkdb.keys();

    foreach(key, keys)
    {
        query.prepare("SELECT id FROM items WHERE key=:key");
        query.bindValue(":key", key.key);
        QUERY_EXEC();

        QTreeWidgetItem * item = new QTreeWidgetItem(itemWksTrk, eTrk);
        if(query.next())
        {
            item->setData(eName, eUserRoleDBKey, query.value(0));
            item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
        }
        item->setData(eName, eUserRoleQLKey, key.key);
        item->setIcon(eName, key.icon);
        item->setText(eName, key.name);
        item->setToolTip(eName, key.comment);
    }

    itemWksTrk->setHidden(itemWksTrk->childCount() == 0);

}

void CGeoDB::slotRteDBChanged()
{
    CGeoDBInternalEditLock lock(this);
}

void CGeoDB::slotOvlDBChanged()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);
    qDeleteAll(itemWksOvl->takeChildren());

    COverlayDB& ovldb = COverlayDB::self();
    COverlayDB::keys_t key;
    QList<COverlayDB::keys_t> keys = ovldb.keys();

    foreach(key, keys)
    {
        query.prepare("SELECT id FROM items WHERE key=:key");
        query.bindValue(":key", key.key);
        QUERY_EXEC();

        QTreeWidgetItem * item = new QTreeWidgetItem(itemWksOvl, eTrk);
        if(query.next())
        {
            item->setData(eName, eUserRoleDBKey, query.value(0));
            item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
        }
        item->setData(eName, eUserRoleQLKey, key.key);
        item->setIcon(eName, key.icon);
        item->setText(eName, key.name);
        item->setToolTip(eName, key.comment);
    }

    itemWksOvl->setHidden(itemWksOvl->childCount() == 0);

}

void CGeoDB::slotAddFolder()
{
    CGeoDBInternalEditLock lock(this);

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

void CGeoDB::slotDelFolder()
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item = treeWidget->currentItem();
    QMessageBox::StandardButton but = QMessageBox::question(0, tr("Delete folders..."), tr("You are sure you want to delete '%1' and all items below?").arg(item->text(eName)), QMessageBox::Ok|QMessageBox::Abort);
    if(but == QMessageBox::Ok)
    {
        delFolder(item, true);
    }

    updateLostFound();
}

void CGeoDB::slotEditFolder()
{
    CGeoDBInternalEditLock lock(this);

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
    QUERY_EXEC(return);

    updateFolderById(itemId);
}

void CGeoDB::slotMoveFolder()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    quint64 parentId1 = 0, parentId2 = 0, childId = 0;
    CDlgSelGeoDBFolder dlg(db, parentId2, true);

    dlg.exec();

    if(parentId2 == 0)
    {
        return;
    }

    QTreeWidgetItem * item  = treeWidget->currentItem();;
    childId     = item->data(eName, eUserRoleDBKey).toULongLong();
    parentId1   = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    query.prepare("DELETE FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId1);
    query.bindValue(":child", childId);
    QUERY_EXEC(return);

    // create link folder <-> item
    query.prepare("SELECT id FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId2);
    query.bindValue(":child", childId);
    QUERY_EXEC();

    if(!query.next())
    {
        query.prepare("INSERT INTO folder2folder (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId2);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId2, item);
    }

    delFolderById(parentId1, childId);
}

void CGeoDB::slotCopyFolder()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    quint64 parentId = 0, childId = 0;
    CDlgSelGeoDBFolder dlg(db, parentId, true);

    dlg.exec();

    if(parentId == 0)
    {
        return;
    }

    QTreeWidgetItem * item = treeWidget->currentItem();
    childId = item->data(eName, eUserRoleDBKey).toULongLong();

    // create link folder <-> item
    query.prepare("SELECT id FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);
    QUERY_EXEC();

    if(!query.next())
    {
        query.prepare("INSERT INTO folder2folder (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId, item);
    }
}

void CGeoDB::slotContextMenu(const QPoint& pos)
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    QTreeWidgetItem * top  = item->parent();
    while(top && top->parent()) top = top->parent();
    if(top == 0) top = item;

    if(top == itemWorkspace)
    {
        QPoint p = treeWidget->mapToGlobal(pos);
        contextMenuWks->exec(p);
    }
    else if(top == itemLostFound)
    {
        QPoint p = treeWidget->mapToGlobal(pos);
        contextMenuLost->exec(p);
    }
    else if(top == itemDatabase)
    {
        if(item->type() == eFolder)
        {
            if(item == itemDatabase)
            {
                actDelDir->setVisible(false);
                actEditDirComment->setVisible(false);
                actMoveDir->setVisible(false);
                actCopyDir->setVisible(false);
            }
            else
            {
                actDelDir->setVisible(true);
                actEditDirComment->setVisible(true);

                quint64 parentId = item->parent()->data(eName, eUserRoleDBKey).toULongLong();
                if(parentId == 1)
                {
                    actMoveDir->setVisible(false);
                    actCopyDir->setVisible(false);
                }
                else
                {
                    actMoveDir->setVisible(true);
                    actCopyDir->setVisible(true);

                }
            }

            QPoint p = treeWidget->mapToGlobal(pos);
            contextMenuFolder->exec(p);
        }
        else
        {
            QPoint p = treeWidget->mapToGlobal(pos);
            contextMenuItem->exec(p);
        }
    }
}

void CGeoDB::slotItemExpanded(QTreeWidgetItem * item)
{
    CGeoDBInternalEditLock lock(this);

    const int size = item->childCount();
    if(size == 0)
    {
        queryChildrenFromDB(item, 2);
    }
    else
    {
        for(int i = 0; i< size; i++)
        {
            QTreeWidgetItem  * child = item->child(i);
            if(child->type() == eFolder && child->childCount() == 0)
            {
                queryChildrenFromDB(child, 1);
            }
        }
    }
}


void CGeoDB::slotItemChanged(QTreeWidgetItem * item, int column)
{
    if(isInternalEdit != 0)
    {
        return;
    }


    CGeoDBInternalEditLock lock(this);
    qDebug() << "void CGeoDB::slotItemChanged(QTreeWidgetItem * item, int column)" << column;

    QSqlQuery query(db);

    if(item->checkState(eName) == Qt::Checked)
    {
        if(item->type() == eFolder)
        {
            moveChildrenToWks(item->data(eName, eUserRoleDBKey).toULongLong());
        }
        else
        {
            query.prepare("SELECT data FROM items WHERE id=:id");
            query.bindValue(":id", item->data(eName, eUserRoleDBKey));
            QUERY_EXEC(return);
            if(query.next())
            {
                QByteArray array = query.value(0).toByteArray();
                QBuffer buffer(&array);
                CQlb qlb(this);
                qlb.load(&buffer);

                CWptDB::self().loadQLB(qlb);
                CTrackDB::self().loadQLB(qlb);
                CRouteDB::self().loadQLB(qlb);
                COverlayDB::self().loadQLB(qlb);

            }
        }

        timeoutCheckState->start(1000);
    }
    else
    {
        quint64 itemId = item->data(eName, eUserRoleDBKey).toULongLong();
        QString itemText = item->text(eName);

        if(itemText.isEmpty() || item->type() != eFolder)
        {
            return;
        }

        if(column == eName)
        {
            query.prepare("UPDATE folders SET name=:name WHERE id=:id");
            query.bindValue(":name", itemText);
            query.bindValue(":id", itemId);
            QUERY_EXEC(return);
        }

        updateFolderById(itemId);
    }
}

void CGeoDB::moveChildrenToWks(quint64 parentId)
{
    QSqlQuery query(db);

    query.prepare("SELECT t1.data, t1.id FROM items AS t1, folder2item AS t2 WHERE t2.parent=:parent AND t1.id = t2.child");
    query.bindValue(":parent", parentId);
    QUERY_EXEC(return);
    while(query.next())
    {
        checkItemByid(query.value(1).toULongLong());

        QByteArray array = query.value(0).toByteArray();
        QBuffer buffer(&array);
        CQlb qlb(this);
        qlb.load(&buffer);

        CWptDB::self().loadQLB(qlb);
        CTrackDB::self().loadQLB(qlb);
        CRouteDB::self().loadQLB(qlb);
        COverlayDB::self().loadQLB(qlb);
    }

    query.prepare("SELECT child FROM folder2folder WHERE parent=:parent");
    query.bindValue(":parent", parentId);
    QUERY_EXEC(return);
    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();
        checkItemByid(childId);
        moveChildrenToWks(childId);
    }
}

void CGeoDB::queryChildrenFromDB(QTreeWidgetItem * parent, int levels)
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    const quint64 parentId = parent->data(eName, eUserRoleDBKey).toULongLong();

    if(parentId == 0)
    {
        return;
    }

    // folders 1st
    query.prepare("SELECT t1.child FROM folder2folder AS t1, folders AS t2 WHERE t1.parent = :id AND t2.id = t1.child ORDER BY t2.name");
    query.bindValue(":id", parentId);
    QUERY_EXEC(return);
    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();

        QSqlQuery query2(db);
        query2.prepare("SELECT icon, name, comment FROM folders WHERE id = :id ORDER BY name");
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

    // items 2nd
    query.prepare("SELECT t1.child FROM folder2item AS t1, items AS t2 WHERE t1.parent = :id AND t2.id = t1.child ORDER BY t2.type, t2.name");
    query.bindValue(":id", parentId);
    QUERY_EXEC(return);
    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();

        QSqlQuery query2(db);
        query2.prepare("SELECT type, icon, name, comment FROM items WHERE id = :id");
        query2.bindValue(":id", childId);
        if(!query2.exec())
        {
            qDebug() << query2.lastQuery();
            qDebug() << query2.lastError();
            continue;
        }
        query2.next();

        QTreeWidgetItem * item = new QTreeWidgetItem(parent, query2.value(0).toInt());
        item->setData(eName, eUserRoleDBKey, childId);
        if(query2.value(0).toInt() == eWpt)
        {
            item->setIcon(eName, getWptIconByName(query2.value(1).toString()));
        }
        else if(query2.value(0).toInt() == eTrk)
        {
            QPixmap pixmap(16,16);
            pixmap.fill(query2.value(1).toString());
            item->setIcon(eName, pixmap);
        }
        else
        {
            item->setIcon(eName, QIcon(query2.value(1).toString()));
        }
        item->setText(eName, query2.value(2).toString());
        item->setToolTip(eName, query2.value(3).toString());

        if(parentId != 1)
        {
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    treeWidget->header()->setResizeMode(eName,QHeaderView::ResizeToContents);
}

void CGeoDB::addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment)
{
    CGeoDBInternalEditLock lock(this);

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
    QUERY_EXEC(return);
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
    QUERY_EXEC(return);

    QTreeWidgetItem * item = new QTreeWidgetItem(eFolder);
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

    addFolderById(parentId, item);

    // delete item as it will be cloned by addFolderById() and never used directly
    delete item;
}

void CGeoDB::delFolder(QTreeWidgetItem * item, bool isTopLevel)
{
    CGeoDBInternalEditLock lock(this);

    int i;
    const int size   = item->childCount();
    quint64 itemId   = item->data(eName, eUserRoleDBKey).toULongLong();
    quint64 parentId = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    QSqlQuery query(db);
    // delete this particular relation first
    query.prepare("DELETE FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", itemId);
    QUERY_EXEC();

    // next query if the item is used as child in any other relation
    query.prepare("SELECT * FROM folder2folder WHERE child=:id");
    query.bindValue(":id", itemId);
    QUERY_EXEC();
    // if there is no other relation delete the children, too.
    if(!query.next())
    {
        for(i = 0; i < size; i++)
        {
            delFolder(item->child(i), false);
        }

        // remove the child items relations
        query.prepare("DELETE FROM folder2item WHERE parent=:id");
        query.bindValue(":id", itemId);
        QUERY_EXEC();

        // and remove the folder
        query.prepare("DELETE FROM folders WHERE id=:id");
        query.bindValue(":id", itemId);
        QUERY_EXEC();
    }

    if(isTopLevel)
    {
        delFolderById(parentId, itemId);
    }

}

void CGeoDB::addFolderById(quint64 parentId, QTreeWidgetItem * child)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }

        if(item->data(eName, eUserRoleDBKey).toULongLong() == parentId)
        {
            QTreeWidgetItem * clone = new QTreeWidgetItem(child->type());
            clone->setIcon(eName, child->icon(eName));
            clone->setIcon(eName, child->icon(eName));
            clone->setData(eName, eUserRoleDBKey, child->data(eName, eUserRoleDBKey));
            clone->setData(eName, eUserRoleQLKey, child->data(eName, eUserRoleQLKey));
            clone->setText(eName, child->text(eName));
            clone->setToolTip(eName, child->toolTip(eName));

            if(parentId != 1)
            {
                clone->setCheckState(0, Qt::Unchecked);
            }

            if(clone->type() == eFolder)
            {
                item->insertChild(0,clone);
            }
            else{
                item->addChild(clone);
            }

            queryChildrenFromDB(clone,1);
            item->sortChildren(eName, Qt::AscendingOrder);
        }
    }
}

void CGeoDB::delFolderById(quint64 parentId, quint64 childId)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);
    QList<QTreeWidgetItem*> itemsToDelete;

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == parentId)
        {
            int i;
            const int size = item->childCount();
            for(i = 0; i < size; i++)
            {
                if(item->child(i)->type() != eFolder)
                {
                    continue;
                }
                if(item->child(i)->data(eName, eUserRoleDBKey).toULongLong() == childId)
                {
                    // just collect the items, do not delete now to prevent crash
                    itemsToDelete << item->takeChild(i);
                    break;
                }
            }
        }
    }

    // now it's save to delete all items
    qDeleteAll(itemsToDelete);
}

void CGeoDB::updateFolderById(quint64 id)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    QSqlQuery query(db);
    query.prepare("SELECT icon, name, comment FROM folders WHERE id=:id");
    query.bindValue(":id", id);
    QUERY_EXEC();
    query.next();

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == id)
        {
            item->setIcon(eName, QIcon(query.value(0).toString()));
            item->setText(eName, query.value(1).toString());
            item->setToolTip(eName, query.value(2).toString());
        }
    }
}

void CGeoDB::updateItemById(quint64 id)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    QSqlQuery query(db);
    query.prepare("SELECT icon, name, comment FROM items WHERE id=:id");
    query.bindValue(":id", id);
    QUERY_EXEC();
    query.next();

    foreach(item, items)
    {
        if(item->type() == eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == id)
        {
            switch(item->type())
            {
                case eWpt:
                {
                    item->setIcon(eName, getWptIconByName(query.value(0).toString()));
                    break;
                }
                case eTrk:
                {
                    QPixmap pixmap(16,16);
                    pixmap.fill(query.value(0).toString());
                    item->setIcon(eName, pixmap);
                    break;
                }
                case eRte:
                {

                    break;
                }
                case eOvl:
                {
                    item->setIcon(eName, QIcon(query.value(0).toString()));
                    break;
                }
            }


            item->setText(eName, query.value(1).toString());
            item->setToolTip(eName, query.value(2).toString());
        }
    }
}


void CGeoDB::delItemById(quint64 parentId, quint64 childId)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);
    QList<QTreeWidgetItem*> itemsToDelete;

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == parentId)
        {
            int i;
            const int size = item->childCount();
            for(i = 0; i < size; i++)
            {
                if(item->child(i)->type() == eFolder)
                {
                    continue;
                }
                if(item->child(i)->data(eName, eUserRoleDBKey).toULongLong() == childId)
                {
                    // just collect the items, do not delete now to prevent crash
                    itemsToDelete << item->takeChild(i);
                    break;
                }
            }
        }
    }

    // now it's save to delete all items
    qDeleteAll(itemsToDelete);
}

void CGeoDB::checkItemByid(quint64 id)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    foreach(item, items)
    {
        if(item->data(eName, eUserRoleDBKey).toULongLong() == id)
        {
            item->setCheckState(eName, Qt::Checked);
        }
    }
}

void CGeoDB::slotAddItems()
{
    CGeoDBInternalEditLock lock(this);


    quint64 parentId;
    CDlgSelGeoDBFolder dlg(db, parentId);

    dlg.exec();

    if(parentId == 0)
    {
        return;
    }


    int size, i;
    QTreeWidgetItem * item;

    bool addAll = treeWidget->currentItem() == itemWorkspace;

    //////////// add waypoints ////////////
    size = itemWksWpt->childCount();
    for(i = 0; i < size; i++)
    {
        item = itemWksWpt->child(i);
        if(!item->isSelected() && !addAll)
        {
            continue;
        }

        addWptToDB(parentId, item);
    }
    //////////// add tracks ////////////
    size = itemWksTrk->childCount();
    for(i = 0; i < size; i++)
    {
        item = itemWksTrk->child(i);
        if(!item->isSelected() && !addAll)
        {
            continue;
        }

        addTrkToDB(parentId, item);
    }
    //////////// add overlays ////////////
    size = itemWksOvl->childCount();
    for(i = 0; i < size; i++)
    {
        item = itemWksOvl->child(i);
        if(!item->isSelected() && !addAll)
        {
            continue;
        }

        addOvlToDB(parentId, item);
    }
}

void CGeoDB::addWptToDB(quint64 parentId, QTreeWidgetItem * item)
{
    quint64 childId;
    QSqlQuery query;
    // test for item with qlandkarte key
    QString key = item->data(eName, eUserRoleQLKey).toString();
    query.prepare("SELECT id FROM items WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC();

    if(query.next())
    {
        childId = query.value(0).toULongLong();
    }
    else
    {
        // insert item
        QString key = item->data(eName, eUserRoleQLKey).toString();
        CWpt * wpt  = CWptDB::self().getWptByKey(key);


        QBuffer buffer;
        CQlb qlb(this);
        qlb << *wpt;
        qlb.save(&buffer);

        // add item to database
        query.prepare("INSERT INTO items (type, key, date, icon, name, comment, data) "
                      "VALUES (:type, :key, :date, :icon, :name, :comment, :data)");

        query.bindValue(":type", eWpt);
        query.bindValue(":key", wpt->key());
        query.bindValue(":date", QDateTime::fromTime_t(wpt->timestamp).toString("yyyy-MM-dd hh-mm-ss"));
        query.bindValue(":icon", wpt->icon);
        query.bindValue(":name", wpt->name);
        query.bindValue(":comment", wpt->comment);
        query.bindValue(":data", buffer.data());
        QUERY_EXEC();

        if(!query.exec("SELECT last_insert_rowid() from items"))
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
    }
    item->setData(eName, eUserRoleDBKey, childId);
    item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
    // create link folder <-> item
    query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);
    QUERY_EXEC();

    if(!query.next() && parentId != 0)
    {
        query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId, item);
    }

}

void CGeoDB::addTrkToDB(quint64 parentId, QTreeWidgetItem * item)
{
    quint64 childId;
    QSqlQuery query;
    // test for item with qlandkarte key
    QString key = item->data(eName, eUserRoleQLKey).toString();
    query.prepare("SELECT id FROM items WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC();

    if(query.next())
    {
        childId = query.value(0).toULongLong();
    }
    else
    {
        // insert item
        QString key = item->data(eName, eUserRoleQLKey).toString();
        CTrack * trk  = CTrackDB::self().getTrackByKey(key);


        QBuffer buffer;
        CQlb qlb(this);
        qlb << *trk;
        qlb.save(&buffer);

        // add item to database
        query.prepare("INSERT INTO items (type, key, date, icon, name, comment, data) "
                      "VALUES (:type, :key, :date, :icon, :name, :comment, :data)");

        query.bindValue(":type", eTrk);
        query.bindValue(":key", trk->key());
        query.bindValue(":date", trk->getStartTimestamp().toString("yyyy-MM-dd hh-mm-ss"));
        query.bindValue(":icon", trk->getColor());
        query.bindValue(":name", trk->name);
        query.bindValue(":comment", trk->getComment());
        query.bindValue(":data", buffer.data());
        QUERY_EXEC();

        if(!query.exec("SELECT last_insert_rowid() from items"))
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
    }
    item->setData(eName, eUserRoleDBKey, childId);
    item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
    // create link folder <-> item
    query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);
    QUERY_EXEC();

    if(!query.next() && parentId != 0)
    {
        query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId, item);
    }
}

void CGeoDB::addOvlToDB(quint64 parentId, QTreeWidgetItem * item)
{
    quint64 childId;
    QSqlQuery query;
    // test for item with qlandkarte key
    QString key = item->data(eName, eUserRoleQLKey).toString();
    query.prepare("SELECT id FROM items WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC();

    if(query.next())
    {
        childId = query.value(0).toULongLong();
    }
    else
    {
        // insert item
        QString key = item->data(eName, eUserRoleQLKey).toString();
        IOverlay * ovl  = COverlayDB::self().getOverlayByKey(key);


        QBuffer buffer;
        CQlb qlb(this);
        qlb << *ovl;
        qlb.save(&buffer);

        // add item to database
        query.prepare("INSERT INTO items (type, key, date, icon, name, comment, data) "
                      "VALUES (:type, :key, :date, :icon, :name, :comment, :data)");

        query.bindValue(":type", eOvl);
        query.bindValue(":key", ovl->key());

        qDebug() <<"type" << ovl->type;
        if(ovl->type == "Text")
        {
            COverlayText * _ovl_ = qobject_cast<COverlayText*>(ovl);
            query.bindValue(":icon", ":/icons/iconText16x16");
            query.bindValue(":name", tr("Static text"));
            query.bindValue(":comment", _ovl_->getInfo());

        }
        else if(ovl->type == "TextBox")
        {
            COverlayTextBox * _ovl_ = qobject_cast<COverlayTextBox*>(ovl);
            query.bindValue(":icon", ":/icons/iconTextBox16x16");
            query.bindValue(":name", tr("Geo ref. text"));
            query.bindValue(":comment", _ovl_->getInfo());

        }
        else if(ovl->type == "Distance")
        {
            COverlayDistance * _ovl_ = qobject_cast<COverlayDistance*>(ovl);
            query.bindValue(":icon", ":/icons/iconDistance16x16");
            query.bindValue(":name", _ovl_->getName());
            query.bindValue(":comment", _ovl_->getInfo());

        }

        query.bindValue(":data", buffer.data());
        QUERY_EXEC();

        if(!query.exec("SELECT last_insert_rowid() from items"))
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
    }
    item->setData(eName, eUserRoleDBKey, childId);
    item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
    // create link folder <-> item
    query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);
    QUERY_EXEC();

    if(!query.next() && parentId != 0)
    {
        query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId, item);
    }

}


void CGeoDB::slotSaveItems()
{
    int i;
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    bool saveAll = treeWidget->currentItem() == itemWorkspace;
    bool savedAll = true;
    bool addUnknownItems = false;

    QList<QTreeWidgetItem*> items;
    for(i = 0; i < itemWksWpt->childCount(); i++)
    {
        items << itemWksWpt->child(i);
    }
    for(i = 0; i < itemWksTrk->childCount(); i++)
    {
        items << itemWksTrk->child(i);
    }
    for(i = 0; i < itemWksRte->childCount(); i++)
    {
        items << itemWksRte->child(i);
    }
    for(i = 0; i < itemWksOvl->childCount(); i++)
    {
        items << itemWksOvl->child(i);
    }


    const int size = items.size();
    QTreeWidgetItem * item;
    for(i = 0; i < size; i++)
    {
        item = items[i];
        if(item->data(eName, eUserRoleDBKey).toULongLong() == 0)
        {

            addUnknownItems = (item->isSelected()||saveAll);

        }
        else
        {
            if(!item->isSelected())
            {
                savedAll = false;
            }
        }
    }

    if(addUnknownItems)
    {
        QMessageBox::Button res = QMessageBox::question(0, tr("Unknown items..."),
                                                        tr("There are items that haven't been added to the database. Add them to Lost & Found?"),
                                                        QMessageBox::Ok|QMessageBox::Ignore, QMessageBox::Ignore
                                                        );
        addUnknownItems = res == QMessageBox::Ok;
    }

    for(i = 0; i < size; i++)
    {
        QString icon, name, comment;

        item = items[i];
        if(!item->isSelected() && !saveAll)
        {
            continue;
        }

        quint64 childId = item->data(eName, eUserRoleDBKey).toULongLong();
        if(childId == 0 && addUnknownItems)
        {
            // add to database
            switch(item->type())
            {
                case eWpt:
                {
                    addWptToDB(0, item);
                    break;
                }
                case eTrk:
                {
                    addTrkToDB(0, item);
                    break;
                }
                case eRte:
                {

                    break;
                }
                case eOvl:
                {
                    addOvlToDB(0, item);
                    break;
                }
            }

        }
        else
        {
            // update database
            QString key = item->data(eName, eUserRoleQLKey).toString();
            QBuffer buffer;
            CQlb qlb(this);

            switch(item->type())
            {
                case eWpt:
                {
                    CWpt * wpt = CWptDB::self().getWptByKey(key);
                    icon = wpt->icon;
                    name = wpt->name;
                    comment = wpt->comment;
                    qlb << *wpt;
                    break;
                }
                case eTrk:
                {
                    CTrack * trk = CTrackDB::self().getTrackByKey(key);
                    icon = trk->getColor().name();
                    name = trk->getName();
                    comment = trk->getComment();
                    qlb << *trk;
                    break;
                }
                case eRte:
                {

                    break;
                }
                case eOvl:
                {
                    IOverlay * ovl = COverlayDB::self().getOverlayByKey(key);
                    if(ovl->type == "Text")
                    {
                        COverlayText * _ovl_ = qobject_cast<COverlayText*>(ovl);
                        icon = ":/icons/iconText16x16";
                        name = tr("Static text");
                        comment = _ovl_->getInfo();
                    }
                    else if(ovl->type == "TextBox")
                    {
                        COverlayTextBox * _ovl_ = qobject_cast<COverlayTextBox*>(ovl);
                        icon = ":/icons/iconTextBox16x16";
                        name = tr("Geo ref. text");
                        comment = _ovl_->getInfo();
                    }
                    else if(ovl->type == "Distance")
                    {
                        COverlayDistance * _ovl_ = qobject_cast<COverlayDistance*>(ovl);
                        icon = ":/icons/iconDistance16x16";
                        name = _ovl_->getName();
                        comment = _ovl_->getInfo();
                    }
                    qlb << *ovl;
                    break;
                }
            }
            qlb.save(&buffer);

            query.prepare("UPDATE items SET icon=:icon, name=:name, comment=:comment, data=:data WHERE id=:id");
            query.bindValue(":icon", icon);
            query.bindValue(":name", name);
            query.bindValue(":comment", comment);
            query.bindValue(":data", buffer.data());
            query.bindValue(":id", childId);
            QUERY_EXEC(continue);

            updateItemById(childId);

        }

    }

    if(savedAll || saveAll)
    {
        itemWorkspace->setText(eName, tr("Workspace"));
    }

    updateLostFound();
}

void CGeoDB::updateLostFound()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    qDeleteAll(itemLostFound->takeChildren());

    query.prepare("SELECT type, id, icon, name, comment FROM items AS t1 WHERE NOT EXISTS(SELECT * FROM folder2item WHERE child=t1.id) ORDER BY t1.name");
    QUERY_EXEC(return);
    QList<QTreeWidgetItem*> items;
    while(query.next())
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(query.value(0).toInt());
        item->setData(eName, eUserRoleDBKey, query.value(1).toULongLong());
        if(item->type() == eWpt)
        {
            item->setIcon(eName, getWptIconByName(query.value(2).toString()));
        }
        else if(item->type() == eTrk)
        {
            QPixmap pixmap(16,16);
            pixmap.fill(query.value(2).toString());
            item->setIcon(eName, pixmap);
        }
        else
        {
            item->setIcon(eName, QIcon(query.value(2).toString()));
        }
        item->setText(eName, query.value(3).toString());
        item->setToolTip(eName, query.value(4).toString());

        items << item;
    }
    if(items.size())
    {
        itemLostFound->setText(eName, tr("Lost & Found (%1)").arg(items.size()));
    }
    else
    {
        itemLostFound->setText(eName, tr("Lost & Found"));
    }

    itemLostFound->addChildren(items);

}

void CGeoDB::slotMoveLost()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    quint64 parentId, childId = 0;
    CDlgSelGeoDBFolder dlg(db, parentId);

    dlg.exec();

    if(parentId == 0)
    {
        return;
    }

    bool moveAll = treeWidget->currentItem() == itemLostFound;

    QTreeWidgetItem * item;
    const int size = itemLostFound->childCount();
    for(int i = 0; i < size; i++)
    {
        item = itemLostFound->child(i);
        if(!item->isSelected() && !moveAll)
        {
            continue;
        }

        childId = item->data(eName, eUserRoleDBKey).toULongLong();

        query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(return);
        // update tree widget
        addFolderById(parentId, item);
    }

    updateLostFound();
}

void CGeoDB::slotDelLost()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    bool delAll = treeWidget->currentItem() == itemLostFound;

    QTreeWidgetItem * item;
    const int size = itemLostFound->childCount();
    for(int i = 0; i < size; i++)
    {
        item = itemLostFound->child(i);
        if(!item->isSelected() && !delAll)
        {
            continue;
        }

        query.prepare("DELETE FROM items WHERE id=:id");
        query.bindValue(":id",item->data(eName, eUserRoleDBKey));
        QUERY_EXEC(continue);
    }

    updateLostFound();
}

void CGeoDB::slotTimeoutCheckState()
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    foreach(item, items)
    {
        if(item->checkState(eName) == Qt::Checked)
        {
            item->setCheckState(eName, Qt::Unchecked);
        }
    }
}

void CGeoDB::slotDelItems()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);
    QTreeWidgetItem* item         = treeWidget->currentItem();
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();

    quint64 parentId = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    foreach(item, items)
    {
        if(item->type() == eFolder)
        {
            continue;
        }
        quint64 childId = item->data(eName, eUserRoleDBKey).toULongLong();

        query.prepare("DELETE FROM folder2item WHERE parent=:parent AND child=:child");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC(continue);

        delItemById(parentId, childId);
    }

    updateLostFound();
}

void CGeoDB::slotMoveItems()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    quint64 parentId1 = 0, parentId2 = 0;
    CDlgSelGeoDBFolder dlg(db, parentId2);

    dlg.exec();

    if(parentId2 == 0)
    {
        return;
    }

    QTreeWidgetItem * item        = treeWidget->currentItem();;
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();

    parentId1 = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    foreach(item, items)
    {
        if(item->type() == eFolder)
        {
            continue;
        }
        quint64 childId = item->data(eName, eUserRoleDBKey).toULongLong();

        query.prepare("DELETE FROM folder2item WHERE parent=:parent AND child=:child");
        query.bindValue(":parent", parentId1);
        query.bindValue(":child", childId);
        QUERY_EXEC(continue);

        // create link folder <-> item
        query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
        query.bindValue(":parent", parentId2);
        query.bindValue(":child", childId);
        QUERY_EXEC();

        if(!query.next())
        {
            query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
            query.bindValue(":parent", parentId2);
            query.bindValue(":child", childId);
            QUERY_EXEC(return);
            // update tree widget
            addFolderById(parentId2, item);
        }

        delItemById(parentId1, childId);
    }
}

void CGeoDB::slotCopyItems()
{
    CGeoDBInternalEditLock lock(this);
    QSqlQuery query(db);

    quint64 parentId = 0;
    CDlgSelGeoDBFolder dlg(db, parentId);

    dlg.exec();

    if(parentId == 0)
    {
        return;
    }

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();

    foreach(item, items)
    {
        if(item->type() == eFolder)
        {
            continue;
        }
        quint64 childId = item->data(eName, eUserRoleDBKey).toULongLong();

        // create link folder <-> item
        query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        QUERY_EXEC();

        if(!query.next())
        {
            query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
            query.bindValue(":parent", parentId);
            query.bindValue(":child", childId);
            QUERY_EXEC(return);
            // update tree widget
            addFolderById(parentId, item);
        }
    }
}

void CGeoDB::slotModified()
{
    CGeoDBInternalEditLock lock(this);
//    itemWorkspace->setText(eName, tr("Workspace (*)"));
}
