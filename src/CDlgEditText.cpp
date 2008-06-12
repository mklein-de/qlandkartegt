/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CDlgEditText.cpp

  Module:

  Description:

  Created:     06/12/2008

  (C) 2008


**********************************************************************************************/

#include "CDlgEditText.h"
#include "CDiaryEditWidget.h"

#include <QtGui>

CDlgEditText::CDlgEditText(QString& content, QWidget * parent)
: QDialog(parent)
, content(content)
{
    setObjectName(QString::fromUtf8("IDlgEditText"));
    resize(600, 400);
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    textedit = new CDiaryEditWidget(content,this, true);

    vboxLayout->addWidget(textedit);
    vboxLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CDlgEditText::~CDlgEditText()
{

}

void CDlgEditText::accept()
{
    qDebug() << "CDlgEditText::accept()";
    content = textedit->getHtml();
    QDialog::accept();
}
