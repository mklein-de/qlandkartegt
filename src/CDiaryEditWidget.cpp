/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/
/****************************************************************************
 **
 ** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License version 2.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of
 ** this file.  Please review the following information to ensure GNU
 ** General Public Licensing requirements will be met:
 ** http://trolltech.com/products/qt/licenses/licensing/opensource/
 **
 ** If you are unsure which license is appropriate for your use, please
 ** review the following information:
 ** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
 ** or contact the sales department at sales@trolltech.com.
 **
 ** In addition, as a special exception, Trolltech gives you certain
 ** additional rights. These rights are described in the Trolltech GPL
 ** Exception version 1.0, which can be found at
 ** http://www.trolltech.com/products/qt/gplexception/ and in the file
 ** GPL_EXCEPTION.txt in this package.
 **
 ** In addition, as a special exception, Trolltech, as the sole copyright
 ** holder for Qt Designer, grants users of the Qt/Eclipse Integration
 ** plug-in the right for the Qt/Eclipse Integration to link to
 ** functionality provided by Qt Designer and its related libraries.
 **
 ** Trolltech reserves all rights not expressly granted herein.
 **
 ** Trolltech ASA (c) 2007
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/
#include "CDiaryEditWidget.h"
#include "CDiaryDB.h"
#include "CWptDB.h"
#include "CWptToolWidget.h"
#include "CTrackDB.h"
#include "CMainWindow.h"
#include "CWpt.h"
#include "CTrack.h"
#include "WptIcons.h"
#include "GeoMath.h"
#include "IUnit.h"
#include "CGeoDB.h"
#include "CTabWidget.h"


#include <QtGui>
#include <QTextHtmlExporter.h>

class CDiaryInternalEditLock
{
    public:
        CDiaryInternalEditLock(CDiaryEditWidget * d) : d(d){d->isInternalEdit += 1;}
        ~CDiaryInternalEditLock(){d->isInternalEdit -= 1;}
    private:
        CDiaryEditWidget * d;
};


CDiaryEditWidget::CDiaryEditWidget(CDiary * diary, QWidget * parent, bool embedded)
: QWidget(parent)
, embedded(embedded)
, modified(false)
, isInternalEdit(0)
, diary(diary)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    actionTextBold = new QAction(QIcon(":/icons/textbold.png"), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    actionTextBold->setCheckable(true);
    toolBold->setDefaultAction(actionTextBold);

    actionTextItalic = new QAction(QIcon(":/icons/textitalic.png"), tr("&Italic"), this);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    actionTextItalic->setCheckable(true);
    toolItalic->setDefaultAction(actionTextItalic);

    actionTextUnderline = new QAction(QIcon(":/icons/textunder.png"), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    actionTextUnderline->setCheckable(true);
    toolUnder->setDefaultAction(actionTextUnderline);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction *)), this, SLOT(textAlign(QAction *)));

    actionAlignLeft = new QAction(QIcon(":/icons/textleft.png"), tr("&Left"), grp);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    toolLeft->setDefaultAction(actionAlignLeft);
    actionAlignCenter = new QAction(QIcon(":/icons/textcenter.png"), tr("C&enter"), grp);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    toolCenter->setDefaultAction(actionAlignCenter);
    actionAlignRight = new QAction(QIcon(":/icons/textright.png"), tr("&Right"), grp);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    toolRight->setDefaultAction(actionAlignRight);
    actionAlignJustify = new QAction(QIcon(":/icons/textjustify.png"), tr("&Justify"), grp);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    toolBlock->setDefaultAction(actionAlignJustify);

    QPixmap pix(24, 24);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    toolColor->setDefaultAction(actionTextColor);

    comboStyle->addItem("standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    connect(comboStyle, SIGNAL(activated(int)), this, SLOT(textStyle(int)));

    connect(comboFont, SIGNAL(activated(const QString &)), this, SLOT(textFamily(const QString &)));

    comboSize->setObjectName("comboSize");
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(const QString &)), this, SLOT(textSize(const QString &)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font().pointSize())));

    connect(textEdit, SIGNAL(textChanged()), this, SLOT(setWindowModified()));
    connect(textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
    connect(textEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this, SLOT(currentCharFormatChanged(const QTextCharFormat &)));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

    textEdit->setFocus();

    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    QAction *a;
    a = actionUndo = new QAction(QIcon(":/icons/editundo.png"), tr("&Undo"), this);
    a->setShortcut(QKeySequence::Undo);
    toolUndo->setDefaultAction(a);

    a = actionRedo = new QAction(QIcon(":/icons/editredo.png"), tr("&Redo"), this);
    a->setShortcut(QKeySequence::Redo);
    toolRedo->setDefaultAction(a);

    a = actionCut = new QAction(QIcon(":/icons/editcut.png"), tr("Cu&t"), this);
    a->setShortcut(QKeySequence::Cut);
    toolCut->setDefaultAction(a);

    a = actionCopy = new QAction(QIcon(":/icons/editcopy.png"), tr("&Copy"), this);
    a->setShortcut(QKeySequence::Copy);
    toolCopy->setDefaultAction(a);

    a = actionPaste = new QAction(QIcon(":/icons/editpaste.png"), tr("&Paste"), this);
    a->setShortcut(QKeySequence::Paste);
    toolPaste->setDefaultAction(a);

    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

    connect(textEdit->document(), SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));

    connect(actionUndo, SIGNAL(triggered()), textEdit, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), textEdit, SLOT(redo()));

    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    connect(actionCut, SIGNAL(triggered()), textEdit, SLOT(cut()));
    connect(actionCopy, SIGNAL(triggered()), textEdit, SLOT(copy()));
    connect(actionPaste, SIGNAL(triggered()), textEdit, SLOT(paste()));

    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));

    if(!embedded)
    {
        toolSave->setIcon(QIcon(":/icons/save.png"));
        connect(toolSave, SIGNAL(clicked(bool)), this, SLOT(slotSave()));

        toolReload->setIcon(QIcon(":/icons/refresh.png"));
        connect(toolReload, SIGNAL(clicked(bool)), this, SLOT(slotDocWizard()));

        toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
        connect(toolExit, SIGNAL(clicked(bool)), this, SLOT(close()));
    }
    else
    {
        toolReload->hide();
        toolExit->hide();
        toolSave->hide();
    }

    QFontMetrics fm(textEdit->font());

    fmtTextHeading1.setFont(textEdit->font());
    fmtTextHeading1.setFontWeight(QFont::Black);
    fmtTextHeading1.setFontPointSize(fmtTextHeading1.fontPointSize() + 8);

    fmtTextHeading2.setFont(textEdit->font());
    fmtTextHeading2.setFontWeight(QFont::Black);
    fmtTextHeading2.setFontPointSize(fmtTextHeading2.fontPointSize() + 4);

    fmtTextStandard.setFont(textEdit->font());
    fmtTextBold = fmtTextStandard;
    fmtTextBold.setFontWeight(QFont::Bold);

    blockHeading1.setTopMargin(5);
    blockHeading1.setBottomMargin(20);

    blockHeading2.setTopMargin(5);
    blockHeading2.setBottomMargin(10);

    blockStandard.setTopMargin(10);
    blockStandard.setBottomMargin(10);

    frameStandard.setTopMargin(5);
    frameStandard.setBottomMargin(5);
    frameStandard.setWidth( 80 * fm.width("X"));
//    frameStandard.setBorder(1);
//    frameStandard.setBorderBrush(Qt::blue);
}


CDiaryEditWidget::~CDiaryEditWidget()
{

}

void CDiaryEditWidget::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}


void CDiaryEditWidget::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}


void CDiaryEditWidget::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}


void CDiaryEditWidget::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}


void CDiaryEditWidget::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}


void CDiaryEditWidget::textSize(const QString &p)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(p.toFloat());
    mergeFormatOnWordOrSelection(fmt);
}


void CDiaryEditWidget::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();

    if (styleIndex != 0)
    {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex)
        {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList())
        {
            listFmt = cursor.currentList()->format();
        }
        else
        {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    }
    else
    {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}


void CDiaryEditWidget::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}


void CDiaryEditWidget::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}


void CDiaryEditWidget::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}


void CDiaryEditWidget::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}


void CDiaryEditWidget::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
    {
        actionAlignLeft->setChecked(true);
    }
    else if (a & Qt::AlignHCenter)
    {
        actionAlignCenter->setChecked(true);
    }
    else if (a & Qt::AlignRight)
    {
        actionAlignRight->setChecked(true);
    }
    else if (a & Qt::AlignJustify)
    {
        actionAlignJustify->setChecked(true);
    }
}


void CDiaryEditWidget::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}


void CDiaryEditWidget::cursorPositionChanged()
{
    alignmentChanged(textEdit->alignment());
}

void CDiaryEditWidget::setWindowModified()
{
    setWindowModified(true);
}

void CDiaryEditWidget::setWindowModified(bool yes)
{
    if(isInternalEdit || !yes) return;

    if(!embedded)
    {
        emit CDiaryDB::self().sigModified();
        emit CDiaryDB::self().sigChanged();
    }

    if(!modified)
    {
        CTabWidget * tab = theMainWindow->getCanvasTab();
        if(tab)
        {
            int idx         = tab->indexOf(this);
            tab->setTabText(idx, tr("Diary - %1 *").arg(diary->getName()));
        }
    }

    modified = yes;

}


void CDiaryEditWidget::clipboardDataChanged()
{
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}


static bool qSortWptLessTime(CWpt * p1, CWpt * p2)
{
    return p1->getTimestamp() < p2->getTimestamp();
}

static bool qSortTrkLessTime(CTrack * t1, CTrack * t2)
{
    return t1->getStartTimestamp() < t2->getStartTimestamp();
}

void CDiaryEditWidget::slotDocWizard()
{

    CDiaryInternalEditLock lock(this);

    if(diary == 0) return;


    int cnt;

    diary->clear();
    if(!CGeoDB::self().getProjectDiaryData(diary->keyProjectGeoDB, *diary))
    {
        return;
    }

    CTabWidget * tab = theMainWindow->getCanvasTab();
    if(tab)
    {
        int idx = tab->indexOf(this);
        if(modified)
        {
            tab->setTabText(idx, tr("Diary - %1 *").arg(diary->getName()));
        }
        else
        {
            tab->setTabText(idx, tr("Diary - %1").arg(diary->getName()));
        }
    }

    textEdit->clear();
    QTextCursor cursor = textEdit->textCursor();

    cursor.insertText(diary->getName(), fmtTextHeading1);

    diary->diaryFrame = cursor.insertFrame(frameStandard);
    {
        QTextCursor cursor1(diary->diaryFrame);
        if(diary->getComment().isEmpty())
        {
            cursor1.insertText(tr("Add your own text here..."), fmtTextStandard);
        }
        else
        {
            cursor1.insertHtml(diary->getComment());
        }
        cursor.setPosition(cursor1.position()+1);
    }

    if(!diary->getWpts().isEmpty())
    {
        QList<CWpt*>& wpts = diary->getWpts();
        cursor.insertText(tr("Waypoints"),fmtTextHeading2);

        QTextTableFormat fmtTbl;
        fmtTbl.setBorder(1);
        fmtTbl.setBorderBrush(Qt::black);
        fmtTbl.setCellPadding(4);
        fmtTbl.setCellSpacing(0);
        fmtTbl.setHeaderRowCount(1);
        fmtTbl.setTopMargin(10);
        fmtTbl.setBottomMargin(20);

        QTextTable * table = cursor.insertTable(wpts.count()+1, 6, fmtTbl);
        diary->tblWpt = table;

        QTextCharFormat fmtHd;
        fmtHd.setBackground(QColor("#c6e3c0"));
        table->cellAt(0,0).setFormat(fmtHd);
        table->cellAt(0,1).setFormat(fmtHd);
        table->cellAt(0,2).setFormat(fmtHd);
        table->cellAt(0,3).setFormat(fmtHd);
        table->cellAt(0,4).setFormat(fmtHd);
        table->cellAt(0,5).setFormat(fmtHd);

        table->cellAt(0,1).firstCursorPosition().insertText(tr("Time"), fmtTextBold);
        table->cellAt(0,2).firstCursorPosition().insertText(tr("Pos."), fmtTextBold);
        table->cellAt(0,3).firstCursorPosition().insertText(tr("Name"), fmtTextBold);
        table->cellAt(0,4).firstCursorPosition().insertText(tr("Elevation"), fmtTextBold);
        table->cellAt(0,5).firstCursorPosition().insertText(tr("Comment"), fmtTextBold);

        cnt = 1;
        qSort(wpts.begin(), wpts.end(), qSortWptLessTime);

        foreach(CWpt * wpt, wpts)
        {
            QString pos;
            GPS_Math_Deg_To_Str(wpt->lon, wpt->lat, pos);

            table->cellAt(cnt,0).firstCursorPosition().insertImage(wpt->getIcon().toImage().scaledToWidth(16, Qt::SmoothTransformation));
            table->cellAt(cnt,1).firstCursorPosition().insertText(QDateTime::fromTime_t(wpt->getTimestamp()).toString());
            table->cellAt(cnt,2).firstCursorPosition().insertText(pos);
            table->cellAt(cnt,3).firstCursorPosition().insertText(wpt->getName());
            if(wpt->ele != WPT_NOFLOAT)
            {
                QString val, unit;
                IUnit::self().meter2elevation(wpt->ele, val, unit);
                table->cellAt(cnt,4).firstCursorPosition().insertText(val + unit);
            }
            else
            {
                table->cellAt(cnt,4).firstCursorPosition().insertText(tr("n/a"));
            }

            table->cellAt(cnt,5).firstCursorPosition().insertText(wpt->getComment());
            cnt++;
        }

        cursor.setPosition(table->lastPosition() + 1);

    }

    if(!diary->getRtes().isEmpty())
    {

    }

    if(!diary->getTrks().isEmpty())
    {
        QList<CTrack*>& trks = diary->getTrks();
        cursor.insertText(tr("Tracks"),fmtTextHeading2);

        QTextTableFormat fmtTbl;
        fmtTbl.setBorder(1);
        fmtTbl.setBorderBrush(Qt::black);
        fmtTbl.setCellPadding(4);
        fmtTbl.setCellSpacing(0);
        fmtTbl.setHeaderRowCount(1);
        fmtTbl.setTopMargin(10);
        fmtTbl.setBottomMargin(20);

        QTextTable * table = cursor.insertTable(trks.count()+1, 3, fmtTbl);
        diary->tblTrk = table;

        QTextCharFormat fmtHd;
        fmtHd.setBackground(QColor("#c6e3c0"));
        table->cellAt(0,0).setFormat(fmtHd);
        table->cellAt(0,1).setFormat(fmtHd);
        table->cellAt(0,2).setFormat(fmtHd);

        table->cellAt(0,1).firstCursorPosition().insertText(tr("Info"), fmtTextBold);
        table->cellAt(0,2).firstCursorPosition().insertText(tr("Comment"), fmtTextBold);

        cnt = 1;
        qSort(trks.begin(), trks.end(), qSortTrkLessTime);

        foreach(CTrack * trk, trks)
        {
            table->cellAt(cnt,0).firstCursorPosition().insertImage(trk->getIcon().toImage().scaledToWidth(16, Qt::SmoothTransformation));
            table->cellAt(cnt,1).firstCursorPosition().insertText(trk->getInfo());
            table->cellAt(cnt,2).firstCursorPosition().insertText(trk->getComment());
            cnt++;
        }

        cursor.setPosition(table->lastPosition() + 1);
    }

}


QString toPlainText(const QTextTableCell& cell)
{
    QString str;
    for (QTextFrame::iterator frm = cell.begin(); frm != cell.end(); ++frm)
    {
        const QTextBlock& blk = frm.currentBlock();
        for(QTextBlock::iterator frgm = blk.begin(); frgm != blk.end(); ++frgm)
        {
            str += frgm.fragment().text() + "\n";
        }
    }

    return str;
}

void CDiaryEditWidget::slotSave()
{
    quint32 cnt = 1;
    if(!diary->diaryFrame.isNull())
    {
        diary->setComment(QLGT::QTextHtmlExporter(textEdit->document()).toHtml(*diary->diaryFrame));
    }

    QList<CWpt*>& wpts = diary->getWpts();
    foreach(CWpt* wpt, wpts)
    {
        wpt->setComment(toPlainText(diary->tblWpt->cellAt(cnt, 5)));
        cnt++;
    }



    CGeoDB::self().setProjectDiaryData(diary->keyProjectGeoDB, *diary);

    CTabWidget * tab = theMainWindow->getCanvasTab();
    if(tab)
    {
        int idx = tab->indexOf(this);
        tab->setTabText(idx, tr("Diary - %1").arg(diary->getName()));
    }
    modified = false;
}
