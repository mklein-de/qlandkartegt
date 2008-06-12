/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CDlgEditText.h

  Module:

  Description:

  Created:     06/12/2008

  (C) 2008


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

