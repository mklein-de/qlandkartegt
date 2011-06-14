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
#include "ui_IDiaryEdit.h"

class CDiary;

class CDiaryEdit : public QWidget, private Ui::IDiaryEdit
{
    Q_OBJECT;
    public:
        CDiaryEdit(CDiary& diary, QWidget * parent);
        virtual ~CDiaryEdit();

        bool isModified();

        void collectData();

    public slots:
        void slotReload();

    private slots:
        void slotSave();
        void slotPrintPreview();
        void setWindowModified();
        void setWindowModified(bool yes);

    protected:
        void resizeEvent(QResizeEvent * e);

    private:
        friend class CDiaryEditLock;

        void draw(QTextDocument& doc);
        void setTabTitle();

        enum eTblCol{eSym, eInfo, eComment, eMax};

        int isInternalEdit;
        CDiary& diary;

        bool modified;
};

#endif //CDIARYEDIT_H

