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

void CDiaryEdit::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);
    textEdit->clear();
    draw(*this, *textEdit->document());
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
    setTabTitle();
    draw(*this, *textEdit->document());
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
    QFontMetrics fm(QFont(font().family(),10));

    int w = doc.textWidth();
    int pointSize = ((10 * w) / (100 *  fm.width("X")));

    qDebug() << "pontSize" << pointSize;

    QFont f = textEdit->font();
    f.setPointSize(pointSize);

    QTextCharFormat fmtCharHeading1;
    fmtCharHeading1.setFont(f);
    fmtCharHeading1.setFontWeight(QFont::Black);
    fmtCharHeading1.setFontPointSize(f.pointSize() + 8);

    QTextCharFormat fmtCharStandard;
    fmtCharStandard.setFont(f);

    QTextBlockFormat fmtBlockStandard;
    fmtBlockStandard.setTopMargin(10);
    fmtBlockStandard.setBottomMargin(10);

    QTextFrameFormat fmtFrameStandard;
    fmtFrameStandard.setTopMargin(5);
    fmtFrameStandard.setBottomMargin(5);
    fmtFrameStandard.setWidth(w);
    fmtFrameStandard.setBorder(1);

    QTextCursor cursor = doc.find(QRegExp(".*"));

    cursor.insertText(diary.getName(), fmtCharHeading1);
    cursor.setCharFormat(fmtCharStandard);
    cursor.setBlockFormat(fmtBlockStandard);

    diary.diaryFrame = cursor.insertFrame(fmtFrameStandard);
    {
        QTextCursor cursor1(diary.diaryFrame);

        cursor1.setCharFormat(fmtCharStandard);
        cursor1.setBlockFormat(fmtBlockStandard);

        if(diary.getComment().isEmpty())
        {
            cursor1.insertText(tr("Add your own text here..."));
        }
        else
        {
            cursor1.insertHtml(diary.getComment());
        }
        cursor.setPosition(cursor1.position()+1);
    }

}


