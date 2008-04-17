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

    private:
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

