/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CDlgDelWpt.cpp

  Module:

  Description:

  Created:     01/06/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgDelWpt.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "WptIcons.h"

#include <QtGui>

CDlgDelWpt::CDlgDelWpt(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);

    const QMap<QString,CWpt*>& wpts         = CWptDB::self().getWpts();
    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();
    QSet<QString> types;

    while(wpt != wpts.end()){
        types << (*wpt)->icon;
        ++wpt;
    }

    QIcon icon;
    QString type;
    foreach(type, types){
        icon = getWptIconByName(type);
        new QListWidgetItem(icon, type, listWptByType);
    }

}

CDlgDelWpt::~CDlgDelWpt()
{

}

void CDlgDelWpt::accept()
{
    QStringList             keys;
    QStringList             types;
    QListWidgetItem *       item;
    QList<QListWidgetItem*> selTypes = listWptByType->selectedItems();
    foreach(item,selTypes) {
        types << item->text();
    }

    const QMap<QString,CWpt*>& wpts         = CWptDB::self().getWpts();
    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();

    while(wpt != wpts.end()){
        if(types.contains((*wpt)->icon)){
            keys << (*wpt)->key();
        }
        ++wpt;
    }

    CWptDB::self().delWpt(keys, true);

    QDialog::accept();
}
