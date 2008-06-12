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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#ifndef CDLGEDITTEXT_H
#define CDLGEDITTEXT_H

#include <QDialog>

class CDiaryEditWidget;
class QVBoxLayout;
class QDialogButtonBox;

class CDlgEditText : public QDialog
{
    Q_OBJECT;
    public:
        CDlgEditText(QString& content, QWidget * parent);
        virtual ~CDlgEditText();
    public slots:
        void accept();

    private:
        QVBoxLayout * vboxLayout;
        CDiaryEditWidget * textedit;
        QDialogButtonBox * buttonBox;

        QString& content;
};

#endif //CDLGEDITTEXT_H

