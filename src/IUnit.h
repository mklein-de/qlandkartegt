/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        IUnit.h

  Module:

  Description:

  Created:     07/08/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef IUNIT_H
#define IUNIT_H
#include <QObject>

class IUnit : public QObject
{
    Q_OBJECT;
    public:
        virtual ~IUnit();

        static IUnit& self(){return *m_self;}

        virtual void meter2elevation(float meter, QString& val, QString& unit) = 0;
        virtual void meter2distance(float meter, QString& val, QString& unit) = 0;

    protected:
        friend class CResources;
        IUnit(QObject * parent);
    private:
        static IUnit * m_self;
};

#endif //IUNIT_H

