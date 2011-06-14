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
#include "CDiaryDB.h"
#include "CMainWindow.h"
#include "CTabWidget.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "CTrack.h"
#include "CTrackDB.h"
#include "QTextHtmlExporter.h"
#include "CMainWindow.h"
#include "CCanvas.h"

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
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    toolSave->setIcon(QIcon(":/icons/save.png"));
    connect(toolSave, SIGNAL(clicked(bool)), this, SLOT(slotSave()));

    toolReload->setIcon(QIcon(":/icons/refresh.png"));
    connect(toolReload, SIGNAL(clicked(bool)), this, SLOT(slotReload()));

    toolPrint->setIcon(QIcon(":/icons/iconPrint22x22.png"));
    connect(toolPrint, SIGNAL(clicked(bool)), this, SLOT(slotPrintPreview()));

    connect(textEdit, SIGNAL(textChanged()), this, SLOT(setWindowModified()));
    connect(textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));

    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked(bool)), this, SLOT(close()));

}

CDiaryEdit::~CDiaryEdit()
{

}


bool CDiaryEdit::isModified()
{

    return modified;
}

void CDiaryEdit::resizeEvent(QResizeEvent * e)
{        
    QWidget::resizeEvent(e);

    CDiaryEditLock lock(this);
    textEdit->clear();
    draw(*textEdit->document());
}

void CDiaryEdit::setWindowModified()
{
    setWindowModified(true);
}

void CDiaryEdit::setWindowModified(bool yes)
{
    if(isInternalEdit || !yes) return;

    emit CDiaryDB::self().emitSigModified(diary.getKey());
    emit CDiaryDB::self().emitSigChanged();

    if(!modified)
    {
        modified = yes;
        setTabTitle();
    }

}

void CDiaryEdit::slotSave()
{
    collectData();

    if(!CGeoDB::self().setProjectDiaryData(diary.keyProjectGeoDB, diary))
    {
        QMessageBox::warning(0, tr("Failed..."), tr("Failed to save diary to database. Probably because it was not created from a database project."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    modified = false;

    setTabTitle();
}

void CDiaryEdit::slotReload()
{
    CDiaryEditLock lock(this);
    if(CGeoDB::self().getProjectDiaryData(diary.keyProjectGeoDB, diary))
    {
        modified = false;
    }

    setTabTitle();

    textEdit->clear();    
    draw(*textEdit->document());
}

void CDiaryEdit::slotPrintPreview()
{
    CDiaryEditLock lock(this);
    collectData();

    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Diary"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    QTextDocument doc;
    QSizeF pageSize = printer.pageRect(QPrinter::DevicePixel).size();
    doc.setPageSize(pageSize);
    draw(doc);

    QImage img;
    theMainWindow->getCanvas()->print(img, pageSize.toSize() - QSize(10,10));

    doc.rootFrame()->lastCursorPosition().insertImage(img);
    doc.print(&printer);

    textEdit->clear();
    draw(*textEdit->document());
}

void CDiaryEdit::setTabTitle()
{
    CDiaryEditLock lock(this);
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

static bool qSortWptLessTime(CWpt * p1, CWpt * p2)
{
    return p1->getTimestamp() < p2->getTimestamp();
}

static bool qSortTrkLessTime(CTrack * t1, CTrack * t2)
{
    return t1->getStartTimestamp() < t2->getStartTimestamp();
}

#define CHAR_PER_LINE 100

void CDiaryEdit::draw(QTextDocument& doc)
{
    CDiaryEditLock lock(this);
    QFontMetrics fm(QFont(font().family(),10));

    int cnt;
    int w = doc.textWidth();
    int pointSize = ((10 * w) / (CHAR_PER_LINE *  fm.width("X")));

    qDebug() << "pontSize" << pointSize;

    QFont f = textEdit->font();
    f.setPointSize(pointSize);
    textEdit->setFont(f);

    QTextCharFormat fmtCharHeading1;
    fmtCharHeading1.setFont(f);
    fmtCharHeading1.setFontWeight(QFont::Black);
    fmtCharHeading1.setFontPointSize(f.pointSize() + 8);

    QTextCharFormat fmtCharHeading2;
    fmtCharHeading2.setFont(f);
    fmtCharHeading2.setFontWeight(QFont::Black);
    fmtCharHeading2.setFontPointSize(f.pointSize() + 4);

    QTextCharFormat fmtCharStandard;
    fmtCharStandard.setFont(f);

    QTextCharFormat fmtCharHeader;
    fmtCharHeader.setFont(f);
    fmtCharHeader.setBackground(QColor("#c6e3c0"));
    fmtCharHeader.setFontWeight(QFont::Bold);

    QTextBlockFormat fmtBlockStandard;
    fmtBlockStandard.setTopMargin(10);
    fmtBlockStandard.setBottomMargin(10);

    QTextFrameFormat fmtFrameStandard;
    fmtFrameStandard.setTopMargin(5);
    fmtFrameStandard.setBottomMargin(5);
    fmtFrameStandard.setRightMargin(20);
    fmtFrameStandard.setWidth(w);
//    fmtFrameStandard.setBorder(1);

    QTextTableFormat fmtTableStandard;
    fmtTableStandard.setBorder(1);
    fmtTableStandard.setBorderBrush(Qt::black);
    fmtTableStandard.setCellPadding(4);
    fmtTableStandard.setCellSpacing(0);
    fmtTableStandard.setHeaderRowCount(1);
    fmtTableStandard.setTopMargin(10);
    fmtTableStandard.setBottomMargin(20);
    fmtTableStandard.setRightMargin(20);
    fmtTableStandard.setWidth(w);

    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::FixedLength, 32);
    constraints << QTextLength(QTextLength::VariableLength, 50);
    constraints << QTextLength(QTextLength::VariableLength, 100);
    fmtTableStandard.setColumnWidthConstraints(constraints);

    QTextCursor cursor = doc.rootFrame()->firstCursorPosition();

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


    if(!diary.getWpts().isEmpty())
    {
        QList<CWpt*>& wpts = diary.getWpts();
        cursor.insertText(tr("Waypoints"),fmtCharHeading2);

        QTextTable * table = cursor.insertTable(wpts.count()+1, eMax, fmtTableStandard);
        diary.tblWpt = table;
        table->cellAt(0,eSym).setFormat(fmtCharHeader);
        table->cellAt(0,eInfo).setFormat(fmtCharHeader);
        table->cellAt(0,eComment).setFormat(fmtCharHeader);

        table->cellAt(0,eInfo).firstCursorPosition().insertText(tr("Info"));
        table->cellAt(0,eComment).firstCursorPosition().insertText(tr("Comment"));

        cnt = 1;
        qSort(wpts.begin(), wpts.end(), qSortWptLessTime);

        foreach(CWpt * wpt, wpts)
        {
            table->cellAt(cnt,eSym).firstCursorPosition().insertImage(wpt->getIcon().toImage().scaledToWidth(16, Qt::SmoothTransformation));
            table->cellAt(cnt,eInfo).firstCursorPosition().insertText(wpt->getInfo(), fmtCharStandard);
            table->cellAt(cnt,eComment).firstCursorPosition().insertHtml(wpt->getComment());
            cnt++;
        }

        cursor.setPosition(table->lastPosition() + 1);
    }

    if(!diary.getTrks().isEmpty())
    {
        QList<CTrack*>& trks = diary.getTrks();
        cursor.insertText(tr("Tracks"),fmtCharHeading2);

        QTextTable * table = cursor.insertTable(trks.count()+1, eMax, fmtTableStandard);
        diary.tblTrk = table;
        table->cellAt(0,eSym).setFormat(fmtCharHeader);
        table->cellAt(0,eInfo).setFormat(fmtCharHeader);
        table->cellAt(0,eComment).setFormat(fmtCharHeader);

        table->cellAt(0,eInfo).firstCursorPosition().insertText(tr("Info"));
        table->cellAt(0,eComment).firstCursorPosition().insertText(tr("Comment"));

        cnt = 1;
        qSort(trks.begin(), trks.end(), qSortTrkLessTime);

        foreach(CTrack * trk, trks)
        {
            table->cellAt(cnt,eSym).firstCursorPosition().insertImage(trk->getIcon().toImage().scaledToWidth(16, Qt::SmoothTransformation));
            table->cellAt(cnt,eInfo).firstCursorPosition().insertText(trk->getInfo(), fmtCharStandard);
            table->cellAt(cnt,eComment).firstCursorPosition().insertHtml(trk->getComment());
            cnt++;
        }

        cursor.setPosition(table->lastPosition() + 1);
    }

}

//static QString toPlainText(const QTextTableCell& cell)
//{
//    QString str;
//    for (QTextFrame::iterator frm = cell.begin(); frm != cell.end(); ++frm)
//    {
//        const QTextBlock& blk = frm.currentBlock();
//        for(QTextBlock::iterator frgm = blk.begin(); frgm != blk.end(); ++frgm)
//        {
//            str += frgm.fragment().text() + "\n";
//        }
//    }

//    return str;
//}

//static QString toPlainText(const QTextFrame& frame)
//{
//    QString str;
//    for (QTextFrame::iterator frm = frame.begin(); frm != frame.end(); ++frm)
//    {
//        const QTextBlock& blk = frm.currentBlock();
//        for(QTextBlock::iterator frgm = blk.begin(); frgm != blk.end(); ++frgm)
//        {
//            str += frgm.fragment().text() + "\n";
//        }
//    }

//    return str;
//}

void CDiaryEdit::collectData()
{
    int cnt;
    if(!diary.diaryFrame.isNull())
    {
        QString comment = QLGT::QTextHtmlExporter(textEdit->document()).toHtml(*diary.diaryFrame);
        diary.setComment(comment.trimmed());
    }
    cnt = 1;
    QList<CWpt*>& wpts = diary.getWpts();
    foreach(CWpt* wpt, wpts)
    {
        wpt->setComment(QLGT::QTextHtmlExporter(textEdit->document()).toHtml(diary.tblWpt->cellAt(cnt, 2)));
        cnt++;
    }

    cnt = 1;
    QList<CTrack*>& trks = diary.getTrks();
    foreach(CTrack* trk, trks)
    {
        trk->setComment(QLGT::QTextHtmlExporter(textEdit->document()).toHtml(diary.tblTrk->cellAt(cnt, 2)));
        cnt++;
    }
}
