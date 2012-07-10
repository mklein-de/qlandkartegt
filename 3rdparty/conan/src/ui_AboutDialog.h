/********************************************************************************
** Form generated from reading UI file 'AboutDialog.ui'
**
** Created: Thu 31. Mar 20:33:19 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *textLabel;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(412, 322);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/conan/connect"), QSize(), QIcon::Normal, QIcon::Off);
        AboutDialog->setWindowIcon(icon);
        AboutDialog->setLayoutDirection(Qt::LeftToRight);
        AboutDialog->setModal(true);
        gridLayout = new QGridLayout(AboutDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textLabel = new QLabel(AboutDialog);
        textLabel->setObjectName(QString::fromUtf8("textLabel"));
        textLabel->setOpenExternalLinks(true);
        textLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

        gridLayout->addWidget(textLabel, 0, 0, 1, 3);

        horizontalSpacer = new QSpacerItem(302, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 0, 1, 1);

        okButton = new QPushButton(AboutDialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        gridLayout->addWidget(okButton, 1, 1, 1, 1);


        retranslateUi(AboutDialog);
        QObject::connect(okButton, SIGNAL(clicked()), AboutDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Conan", 0, QApplication::UnicodeUTF8));
        textLabel->setText(QApplication::translate("AboutDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-size:12pt; font-weight:600;\">Conan - Connection Analyzer for Qt</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">run-time introspection of object hierarchies, object</p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"> inheritance, signal/slot connections, and signal emissions</"
                        "p>\n"
"<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">Version - %1; Qt %2</p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">Contact - <a href=\"edekoning@gmail.com\"><span style=\" text-decoration: underline; color:#0000ff;\">edekoning@gmail.com</span></a></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"edekoning@gmail.com\"><span style=\" text-decoration:none; color:#000000;\">Website - </span></a><a href=\"http://sourceforge.net/projects/conanforqt\""
                        ">http://sourceforge.net/projects/conanforqt</a></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"http://sourceforge.net/projects/conanforqt\"><span style=\" text-decoration:none; color:#000000;\">License - GPL (source code); CCL (icons)</span></a></p>\n"
"<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\"></p>\n"
"<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\"><span style=\" font-weight:600;\">Conan was made "
                        "using the following tools:</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">Silk icons made by Mark James available at <a href=\"http://www.famfamfam.com/lab/icons/silk/\"><span style=\" text-decoration: underline; color:#0000ff;\">FamFamFam</span></a></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"http://www.famfamfam.com/lab/icons/silk/\"><span style=\" text-decoration:none; color:#000000;\">Qt Open Source Edition, available from </span></a><a href=\"http://qt.nokia.com/downloads/\">Nokia</a></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"http://trolltech.com/downloads/opensource/\"><span style=\" text-decoration:"
                        "none; color:#000000;\">Visual C++ 2008 Express Edition, available from </span></a><a href=\"http://www.microsoft.com/express/Windows/\">Microsoft</a></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"www.doxygen.org/\">Doxygen</a><a href=\"http://www.microsoft.com/express/vc/\"><span style=\" text-decoration:none; color:#000000;\"> by Dimitri van Heesch for source code documentation</span></a></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; text-decoration: underline; color:#0000ff;\"><a href=\"http://sourceforge.net/\">SourceForge</a><a href=\"http://www.microsoft.com/express/vc/\"><span style=\" text-decoration:none; color:#000000;\">, which hosts this project.</span></a></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; marg"
                        "in-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\"></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt; color:#000000;\">\302\251 Copyright 2008 - 2011 Elmar de Koning, All Rights Reserved</p></body></html>", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("AboutDialog", "Ok", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
