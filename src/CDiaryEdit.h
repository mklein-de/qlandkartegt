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
class CTrack;

class CDiaryEdit : public QWidget, private Ui::IDiaryEdit
{
    Q_OBJECT;
    public:
        CDiaryEdit(CDiary& diary, QWidget * parent);
        virtual ~CDiaryEdit();

        void collectData();
        void setTabTitle();

    public slots:
        void slotReload();
        void slotReload(bool fromDB);

    private slots:
        void slotSave();
        void slotPrintPreview();
        void setWindowModified();
        void setWindowModified(bool yes);
        void slotClipboardDataChanged();
        void slotTextBold();
        void slotTextUnderline();
        void slotTextItalic();
        void slotTextColor();
        void slotTextStyle(int styleIndex);

        void slotCurrentCharFormatChanged(const QTextCharFormat &format);
        void slotCursorPositionChanged();

        void slotIntReload();


    protected:
        void resizeEvent(QResizeEvent * e);
        void closeEvent(QCloseEvent * event);

    private:
        friend class CDiaryEditLock;

        void draw(QTextDocument& doc);
        void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
        void fontChanged(const QFont &f);
        void colorChanged(const QColor &c);
        void getTrackProfile(CTrack * trk, QImage& image);

        enum eTblCol{eSym, eInfo, eComment, eMax};

        int isInternalEdit;

        CDiary& diary;



        QAction * actionUndo;
        QAction * actionRedo;
        QAction * actionCut;
        QAction * actionCopy;
        QAction * actionPaste;

        QAction * actionTextBold;
        QAction * actionTextUnderline;
        QAction * actionTextItalic;
        QAction * actionTextColor;


};

#endif //CDIARYEDIT_H

