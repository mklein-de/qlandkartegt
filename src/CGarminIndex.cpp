/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#include "CGarminIndex.h"
#include "CGarminTile.h"

#include <QtGui>
#include <QSqlQuery>
#include <QSqlError>


CGarminIndex::CGarminIndex(QObject * parent)
: QThread(parent)
{

}

CGarminIndex::~CGarminIndex()
{

}

bool CGarminIndex::indexCreated()
{

    return QFile::exists(dbName) && !isRunning();
}

void CGarminIndex::setDBName(const QString& name)
{
    QMutexLocker lock(&mutex);

    QDir path(QDir::home().filePath(".config/QLandkarteGT/"));
    dbName = path.filePath(name + ".db");
    qDebug() << "add" << dbName;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    db.open();
}

void CGarminIndex::create(const QStringList& files)
{
    if(isRunning()) return;
    imgFiles = files;

    QSqlDatabase db = QSqlDatabase::database(dbName);
    db.close();
    db.removeDatabase(dbName);

    QFile::remove(dbName);

    start();
}

void CGarminIndex::run()
{
    QMutexLocker lock(&mutex);

    const QRectF viewport(QPointF(-180 * DEG_TO_RAD, 90 * DEG_TO_RAD), QPointF(180 * DEG_TO_RAD, -90 * DEG_TO_RAD));
    const int size  = imgFiles.size();
    int cnt         = 0;
    QString filename;
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    db.open();

    QSqlQuery query(db);
    if(!query.exec("PRAGMA temp_store = MEMORY"))
    {
        qDebug() << query.lastError();
    }


    if(!query.exec("PRAGMA synchronous = OFF"))
    {
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE subfiles ("
                "id             INTEGER PRIMARY KEY,"
                "name           TEXT NOT NULL,"
                "filename       TEXT NOT NULL,"
                "rgn_offset     INT UNSIGNED NOT NULL,"
                "rgn_size       INT UNSIGNED NOT NULL,"
                "level          INT UNSIGNED NOT NULL"
                ")"))
    {
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE subdivs ("
                "id             INTEGER PRIMARY KEY,"
                "subfile        INT UNSIGNED NOT NULL,"
                "center_lon     INT NOT NULL,"
                "center_lat     INT NOT NULL,"
                "shift          INT NOT NULL"
                ")"))
    {
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE polylines ("
                "id             INTEGER PRIMARY KEY,"
                "type           INT UNSIGNED NOT NULL,"
                "subdiv         INT UNSIGNED NOT NULL,"
                "offset         INT UNSIGNED NOT NULL,"
                "label          TEXT"
                ")"))
    {
        qDebug() << query.lastError();
    }

    foreach(filename, imgFiles){
        CGarminTile tile(0);
        tile.readBasics(filename);
        emit sigProgress(tr("Create index... %1").arg(filename), cnt * 100 / size);

        tile.createIndex(db);
        ++cnt;
    }

    emit sigProgress(tr("Done"), 0);
    disconnect(this, 0, 0, 0);
}

void CGarminIndex::searchPolyline(const QString& text, QSet<QString>& result)
{
    qDebug() << "void CGarminIndex::searchPolyline(const QString& text)";
    QMutexLocker lock(&mutex);

    QSqlDatabase db = QSqlDatabase::database(dbName);
    QSqlQuery query(db);

    query.exec(QString("SELECT label FROM polylines WHERE label LIKE \"%1%\"").arg(text));

    while (query.next()) {
        result <<  query.value(0).toString();

//         if(result.size() > 20){
//             result << "...";
//             break;
//         }
    }
}
