/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        IItem.h

  Module:

  Description:

  Created:     09/04/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef IITEM_H
#define IITEM_H

#include <QObject>
#include <QPixmap>

class IItem : public QObject
{
    Q_OBJECT;
    public:
        IItem(QObject * parent);
        virtual ~IItem();

        virtual void setName(const QString& str){name = str;}
        virtual QString getName(){return name;}

        virtual void setComment(const QString& str){comment = str;}
        virtual QString getComment(){return comment;}

        virtual QString getInfo()= 0;

        virtual void setIcon(const QString& str) = 0;
        virtual QPixmap getIcon(){return iconPixmap;}
        virtual QString getIconString(){return iconString;}

        virtual void setKey(const QString& str){key = str;}
        virtual QString getKey();

        virtual quint32 getTimestamp(){return timestamp;}

        static void resetKeyCnt(){keycnt = 0;}

    protected:

        quint32 timestamp;

        QString name;
        QString comment;
        QPixmap iconPixmap;
        QString iconString;

    private:
        virtual void genKey();
        static quint32 keycnt;
        QString key;

};

#endif //IITEM_H

