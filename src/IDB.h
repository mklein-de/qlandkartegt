/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        IDB.h

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/
#ifndef IDB_H
#define IDB_H

#include <QObject>

class QToolBox;
class QWidget;

class IDB : public QObject
{
    Q_OBJECT
    public:
        IDB(QToolBox * tb, QObject * parent);
        virtual ~IDB();

        void gainFocus();

    protected:
        QToolBox *  toolbox;
        QWidget *   toolview;
};

#endif //IDB_H

