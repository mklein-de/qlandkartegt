/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDiaryEdit.cpp

  Module:

  Description:

  Created:     06/10/2011

  (C) 2011 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDiaryEdit.h"
#include "CGeoDB.h"
#include "CDiary.h"
#include "CMainWindow.h"
#include "CTabWidget.h"

#include <QtGui>

class CDiaryEditLock
{
    public:
        CDiaryEditLock(CDiaryEdit * d) : d(d){d->isInternalEdit += 1;}
        ~CDiaryEditLock(){d->isInternalEdit -= 1;}
    private:
        CDiaryEdit * d;
};


CDiaryEdit::CDiaryEdit(CDiary& diary, QWidget * parent)
: QWidget(parent)
, isInternalEdit(0)
, diary(diary)
, modified(false)
{
    setupUi(this);

    toolSave->setIcon(QIcon(":/icons/save.png"));
    connect(toolSave, SIGNAL(clicked(bool)), this, SLOT(slotSave()));

    toolReload->setIcon(QIcon(":/icons/refresh.png"));
    connect(toolReload, SIGNAL(clicked(bool)), this, SLOT(slotReload()));

    toolPrint->setIcon(QIcon(":/icons/iconPrint22x22.png"));
    connect(toolPrint, SIGNAL(clicked(bool)), this, SLOT(slotPrintPreview()));


    slotReload();
}

CDiaryEdit::~CDiaryEdit()
{

}


void CDiaryEdit::collectData()
{

}

bool CDiaryEdit::isModified()
{

    return false;
}

void CDiaryEdit::slotSave()
{

}

void CDiaryEdit::slotReload()
{
    if(CGeoDB::self().getProjectDiaryData(diary.keyProjectGeoDB, diary))
    {
        modified = false;
    }

    textEdit->clear();
//    draw();
}

void CDiaryEdit::slotPrintPreview()
{

}

void CDiaryEdit::setTabTitle()
{
    CTabWidget * tab = theMainWindow->getCanvasTab();
    if(tab)
    {
        int idx = tab->indexOf(this);
        if(modified)
        {
            tab->setTabText(idx, tr("Diary - %1 *").arg(diary.getName()));
        }
        else
        {
            tab->setTabText(idx, tr("Diary - %1").arg(diary.getName()));
        }
    }

}

void CDiaryEdit::draw(QPaintDevice& dev, QTextDocument& doc)
{
    CDiaryEditLock lock(this);

    setTabTitle();



}
