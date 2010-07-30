/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CGeoDB.h

  Module:      

  Description:

  Created:     07/29/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CGEODB_H
#define CGEODB_H

#include <QWidget>
#include "ui_IGeoToolWidget.h"

class QTabWidget;
class QTreeWidgetItem;

class CGeoDB : public QWidget, private Ui::IGeoToolWidget
{
    Q_OBJECT;
    public:
        CGeoDB(QTabWidget * tb, QWidget * parent);
        virtual ~CGeoDB();

        void gainFocus();

    private:
        QTabWidget * tabbar;
        QTreeWidgetItem * itemDatabase;
        QTreeWidgetItem * itemWorkspace;
};

#endif //CGEODB_H

