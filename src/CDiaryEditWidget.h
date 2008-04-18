/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License; or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful;
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not; write to the Free Software
    Foundation; Inc.; 59 Temple Place - Suite 330; Boston; MA 02111 USA

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
#ifndef CDIARYEDITWIDGET_H
#define CDIARYEDITWIDGET_H

#include <QWidget>
#include "ui_IDiaryEditWidget.h"

class CDiaryEditWidget : public QWidget, private Ui::IDiaryEditWidget
{
    Q_OBJECT;
    public:
        CDiaryEditWidget(QWidget * parent);
        virtual ~CDiaryEditWidget();

    private slots:
        void textBold();
        void textUnderline();
        void textItalic();
        void textFamily(const QString &f);
        void textSize(const QString &p);
        void textStyle(int styleIndex);
        void textColor();
        void textAlign(QAction *a);

        void currentCharFormatChanged(const QTextCharFormat &format);
        void cursorPositionChanged();
        void setWindowModified(bool);
        void clipboardDataChanged();

    private:
        friend class CDiaryDB;
        void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
        void fontChanged(const QFont &f);
        void colorChanged(const QColor &c);
        void alignmentChanged(Qt::Alignment a);

        QAction * actionTextBold;
        QAction * actionTextUnderline;
        QAction * actionTextItalic;
        QAction * actionTextColor;
        QAction * actionAlignLeft;
        QAction * actionAlignCenter;
        QAction * actionAlignRight;
        QAction * actionAlignJustify;

        QAction * actionUndo;
        QAction * actionRedo;
        QAction * actionCut;
        QAction * actionCopy;
        QAction * actionPaste;

};

#endif //CDIARYEDITWIDGET_H

