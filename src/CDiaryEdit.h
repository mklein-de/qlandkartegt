/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDiaryEdit.h

  Module:      

  Description:

  Created:     06/10/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDIARYEDIT_H
#define CDIARYEDIT_H

#include <QWidget>


class CDiaryEdit : public QWidget
{
    Q_OBJECT;
    public:
        CDiaryEdit(QWidget * parent);
        virtual ~CDiaryEdit();

//        QString getHtml();
//        void  setHtml(const QString& text);

        void collectData();

        bool isModified();

    public slots:
        void slotDocWizard();
//        void slotSave();

};

#endif //CDIARYEDIT_H

