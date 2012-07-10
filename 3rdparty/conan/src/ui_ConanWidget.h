/********************************************************************************
** Form generated from reading UI file 'ConanWidget.ui'
**
** Created: Thu 31. Mar 20:33:19 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONANWIDGET_H
#define UI_CONANWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QTabWidget>
#include <QtGui/QTableView>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConanWidget
{
public:
    QAction *actionBack;
    QAction *actionForward;
    QAction *actionFind;
    QAction *actionRefresh;
    QAction *actionFocusFind;
    QAction *actionDiscover;
    QAction *actionFindMethod;
    QAction *actionSpySignal;
    QAction *actionAboutConan;
    QAction *actionBug;
    QAction *actionDeleteSpies;
    QAction *actionSelectAllSpies;
    QAction *actionExport;
    QAction *actionRemoveRootObject;
    QAction *actionDisconnect;
    QAction *actionDisconnectAll;
    QAction *actionRemoveAllRootObjects;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *objectTab;
    QVBoxLayout *verticalLayout;
    QSplitter *mainSplitter;
    QWidget *layoutWidget;
    QVBoxLayout *leftVerticalLayout;
    QHBoxLayout *leftToolBarLayout;
    QToolButton *backToolButton;
    QToolButton *forwardToolButton;
    QLineEdit *findLineEdit;
    QToolButton *findToolButton;
    QToolButton *refreshToolButton;
    QToolButton *discoverToolButton;
    QToolButton *bugToolButton;
    QToolButton *exportToolButton;
    QSpacerItem *toolBarSpacer;
    QToolButton *aboutToolButton;
    QTreeView *objectTree;
    QToolBox *toolBox;
    QWidget *inheritancePage;
    QVBoxLayout *verticalLayout_4;
    QListView *inheritanceList;
    QWidget *classInfoPage;
    QVBoxLayout *verticalLayout_5;
    QTableView *classInfoTable;
    QWidget *layoutWidget1;
    QVBoxLayout *rightVerticalLayout;
    QHBoxLayout *rightToolbarLayout;
    QSpacerItem *horizontalConnectionSpacer;
    QCheckBox *hideInactiveCheckBox;
    QCheckBox *hideInheritedCheckBox;
    QSplitter *signalSlotSplitter;
    QGroupBox *signalGroupBox;
    QGridLayout *gridLayout_1;
    QTreeView *signalTree;
    QGroupBox *slotGroupBox;
    QGridLayout *gridLayout_2;
    QTreeView *slotTree;
    QWidget *signalTab;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *loggerOptionsGroupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *optionsHorizontalLayout;
    QCheckBox *timestampCheckBox;
    QCheckBox *objectCheckBox;
    QCheckBox *addressCheckBox;
    QCheckBox *signatureCcheckBox;
    QCheckBox *emitCountCheckBox;
    QCheckBox *argumentsCheckBox;
    QFrame *line;
    QCheckBox *prettyFormattingCheckBox;
    QFrame *line_2;
    QLabel *separatorLabel;
    QLineEdit *separatorLineEdit;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *exampleLayout;
    QLabel *signalLogLabel;
    QGroupBox *spiesGroupBox;
    QHBoxLayout *horizontalLayout_2;
    QTableView *signalSpiesTableView;

    void setupUi(QWidget *ConanWidget)
    {
        if (ConanWidget->objectName().isEmpty())
            ConanWidget->setObjectName(QString::fromUtf8("ConanWidget"));
        ConanWidget->resize(800, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/conan/connect"), QSize(), QIcon::Normal, QIcon::Off);
        ConanWidget->setWindowIcon(icon);
        actionBack = new QAction(ConanWidget);
        actionBack->setObjectName(QString::fromUtf8("actionBack"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/conan/prev"), QSize(), QIcon::Normal, QIcon::Off);
        actionBack->setIcon(icon1);
        actionForward = new QAction(ConanWidget);
        actionForward->setObjectName(QString::fromUtf8("actionForward"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/conan/next"), QSize(), QIcon::Normal, QIcon::Off);
        actionForward->setIcon(icon2);
        actionFind = new QAction(ConanWidget);
        actionFind->setObjectName(QString::fromUtf8("actionFind"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/conan/find"), QSize(), QIcon::Normal, QIcon::Off);
        actionFind->setIcon(icon3);
        actionRefresh = new QAction(ConanWidget);
        actionRefresh->setObjectName(QString::fromUtf8("actionRefresh"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/conan/refresh"), QSize(), QIcon::Normal, QIcon::Off);
        actionRefresh->setIcon(icon4);
        actionFocusFind = new QAction(ConanWidget);
        actionFocusFind->setObjectName(QString::fromUtf8("actionFocusFind"));
        actionDiscover = new QAction(ConanWidget);
        actionDiscover->setObjectName(QString::fromUtf8("actionDiscover"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/conan/transmit"), QSize(), QIcon::Normal, QIcon::Off);
        actionDiscover->setIcon(icon5);
        actionFindMethod = new QAction(ConanWidget);
        actionFindMethod->setObjectName(QString::fromUtf8("actionFindMethod"));
        actionFindMethod->setIcon(icon3);
        actionSpySignal = new QAction(ConanWidget);
        actionSpySignal->setObjectName(QString::fromUtf8("actionSpySignal"));
        actionSpySignal->setCheckable(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/conan/spy"), QSize(), QIcon::Normal, QIcon::Off);
        actionSpySignal->setIcon(icon6);
        actionAboutConan = new QAction(ConanWidget);
        actionAboutConan->setObjectName(QString::fromUtf8("actionAboutConan"));
        actionAboutConan->setIcon(icon);
        actionBug = new QAction(ConanWidget);
        actionBug->setObjectName(QString::fromUtf8("actionBug"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/conan/bug"), QSize(), QIcon::Normal, QIcon::Off);
        actionBug->setIcon(icon7);
        actionDeleteSpies = new QAction(ConanWidget);
        actionDeleteSpies->setObjectName(QString::fromUtf8("actionDeleteSpies"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/conan/delete"), QSize(), QIcon::Normal, QIcon::Off);
        actionDeleteSpies->setIcon(icon8);
        actionSelectAllSpies = new QAction(ConanWidget);
        actionSelectAllSpies->setObjectName(QString::fromUtf8("actionSelectAllSpies"));
        actionExport = new QAction(ConanWidget);
        actionExport->setObjectName(QString::fromUtf8("actionExport"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/conan/export"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport->setIcon(icon9);
        actionRemoveRootObject = new QAction(ConanWidget);
        actionRemoveRootObject->setObjectName(QString::fromUtf8("actionRemoveRootObject"));
        actionRemoveRootObject->setIcon(icon8);
        actionDisconnect = new QAction(ConanWidget);
        actionDisconnect->setObjectName(QString::fromUtf8("actionDisconnect"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/conan/disconnect"), QSize(), QIcon::Normal, QIcon::Off);
        actionDisconnect->setIcon(icon10);
        actionDisconnectAll = new QAction(ConanWidget);
        actionDisconnectAll->setObjectName(QString::fromUtf8("actionDisconnectAll"));
        actionDisconnectAll->setIcon(icon10);
        actionRemoveAllRootObjects = new QAction(ConanWidget);
        actionRemoveAllRootObjects->setObjectName(QString::fromUtf8("actionRemoveAllRootObjects"));
        actionRemoveAllRootObjects->setIcon(icon8);
        horizontalLayout = new QHBoxLayout(ConanWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tabWidget = new QTabWidget(ConanWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setAutoFillBackground(false);
        tabWidget->setStyleSheet(QString::fromUtf8(" QTabWidget::pane { /* The tab widget frame */\n"
"	border-bottom: 2px solid;\n"
"	border-bottom-color: rgb(92, 92, 92);\n"
"	bottom: -0.15em\n"
"}\n"
"\n"
" QTabWidget::tab-bar {\n"
"     subcontrol-position: center bottom;\n"
"	\n"
" }\n"
"\n"
""));
        tabWidget->setTabPosition(QTabWidget::South);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tabWidget->setUsesScrollButtons(true);
        objectTab = new QWidget();
        objectTab->setObjectName(QString::fromUtf8("objectTab"));
        verticalLayout = new QVBoxLayout(objectTab);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        mainSplitter = new QSplitter(objectTab);
        mainSplitter->setObjectName(QString::fromUtf8("mainSplitter"));
        mainSplitter->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(mainSplitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        leftVerticalLayout = new QVBoxLayout(layoutWidget);
        leftVerticalLayout->setObjectName(QString::fromUtf8("leftVerticalLayout"));
        leftVerticalLayout->setContentsMargins(0, 0, 0, 0);
        leftToolBarLayout = new QHBoxLayout();
        leftToolBarLayout->setObjectName(QString::fromUtf8("leftToolBarLayout"));
        backToolButton = new QToolButton(layoutWidget);
        backToolButton->setObjectName(QString::fromUtf8("backToolButton"));
        backToolButton->setIcon(icon1);

        leftToolBarLayout->addWidget(backToolButton);

        forwardToolButton = new QToolButton(layoutWidget);
        forwardToolButton->setObjectName(QString::fromUtf8("forwardToolButton"));
        forwardToolButton->setIcon(icon2);

        leftToolBarLayout->addWidget(forwardToolButton);

        findLineEdit = new QLineEdit(layoutWidget);
        findLineEdit->setObjectName(QString::fromUtf8("findLineEdit"));

        leftToolBarLayout->addWidget(findLineEdit);

        findToolButton = new QToolButton(layoutWidget);
        findToolButton->setObjectName(QString::fromUtf8("findToolButton"));
        findToolButton->setIcon(icon3);

        leftToolBarLayout->addWidget(findToolButton);

        refreshToolButton = new QToolButton(layoutWidget);
        refreshToolButton->setObjectName(QString::fromUtf8("refreshToolButton"));
        refreshToolButton->setIcon(icon4);

        leftToolBarLayout->addWidget(refreshToolButton);

        discoverToolButton = new QToolButton(layoutWidget);
        discoverToolButton->setObjectName(QString::fromUtf8("discoverToolButton"));
        discoverToolButton->setIcon(icon5);

        leftToolBarLayout->addWidget(discoverToolButton);

        bugToolButton = new QToolButton(layoutWidget);
        bugToolButton->setObjectName(QString::fromUtf8("bugToolButton"));
        bugToolButton->setIcon(icon7);

        leftToolBarLayout->addWidget(bugToolButton);

        exportToolButton = new QToolButton(layoutWidget);
        exportToolButton->setObjectName(QString::fromUtf8("exportToolButton"));
        exportToolButton->setIcon(icon9);

        leftToolBarLayout->addWidget(exportToolButton);

        toolBarSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        leftToolBarLayout->addItem(toolBarSpacer);

        aboutToolButton = new QToolButton(layoutWidget);
        aboutToolButton->setObjectName(QString::fromUtf8("aboutToolButton"));
        aboutToolButton->setIcon(icon);

        leftToolBarLayout->addWidget(aboutToolButton);


        leftVerticalLayout->addLayout(leftToolBarLayout);

        objectTree = new QTreeView(layoutWidget);
        objectTree->setObjectName(QString::fromUtf8("objectTree"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(objectTree->sizePolicy().hasHeightForWidth());
        objectTree->setSizePolicy(sizePolicy);
        objectTree->setContextMenuPolicy(Qt::CustomContextMenu);
        objectTree->setProperty("showDropIndicator", QVariant(false));
        objectTree->setUniformRowHeights(true);
        objectTree->setAllColumnsShowFocus(true);

        leftVerticalLayout->addWidget(objectTree);

        toolBox = new QToolBox(layoutWidget);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(toolBox->sizePolicy().hasHeightForWidth());
        toolBox->setSizePolicy(sizePolicy1);
        inheritancePage = new QWidget();
        inheritancePage->setObjectName(QString::fromUtf8("inheritancePage"));
        inheritancePage->setGeometry(QRect(0, 0, 425, 146));
        verticalLayout_4 = new QVBoxLayout(inheritancePage);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        inheritanceList = new QListView(inheritancePage);
        inheritanceList->setObjectName(QString::fromUtf8("inheritanceList"));
        sizePolicy1.setHeightForWidth(inheritanceList->sizePolicy().hasHeightForWidth());
        inheritanceList->setSizePolicy(sizePolicy1);
        inheritanceList->setMinimumSize(QSize(0, 128));
        inheritanceList->setMaximumSize(QSize(16777215, 128));
        inheritanceList->setBaseSize(QSize(0, 128));
        inheritanceList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        inheritanceList->setProperty("showDropIndicator", QVariant(false));

        verticalLayout_4->addWidget(inheritanceList);

        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/conan/hierarchy"), QSize(), QIcon::Normal, QIcon::Off);
        toolBox->addItem(inheritancePage, icon11, QString::fromUtf8("Inheritance"));
        classInfoPage = new QWidget();
        classInfoPage->setObjectName(QString::fromUtf8("classInfoPage"));
        classInfoPage->setGeometry(QRect(0, 0, 425, 146));
        verticalLayout_5 = new QVBoxLayout(classInfoPage);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        classInfoTable = new QTableView(classInfoPage);
        classInfoTable->setObjectName(QString::fromUtf8("classInfoTable"));
        sizePolicy1.setHeightForWidth(classInfoTable->sizePolicy().hasHeightForWidth());
        classInfoTable->setSizePolicy(sizePolicy1);
        classInfoTable->setMinimumSize(QSize(0, 128));
        classInfoTable->setMaximumSize(QSize(16777215, 128));
        classInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        classInfoTable->setTabKeyNavigation(false);
        classInfoTable->setProperty("showDropIndicator", QVariant(false));
        classInfoTable->setDragDropOverwriteMode(false);
        classInfoTable->setShowGrid(false);

        verticalLayout_5->addWidget(classInfoTable);

        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons/conan/info"), QSize(), QIcon::Normal, QIcon::Off);
        toolBox->addItem(classInfoPage, icon12, QString::fromUtf8("Class information"));

        leftVerticalLayout->addWidget(toolBox);

        mainSplitter->addWidget(layoutWidget);
        layoutWidget1 = new QWidget(mainSplitter);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        rightVerticalLayout = new QVBoxLayout(layoutWidget1);
        rightVerticalLayout->setObjectName(QString::fromUtf8("rightVerticalLayout"));
        rightVerticalLayout->setContentsMargins(0, 0, 0, 0);
        rightToolbarLayout = new QHBoxLayout();
        rightToolbarLayout->setObjectName(QString::fromUtf8("rightToolbarLayout"));
        horizontalConnectionSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        rightToolbarLayout->addItem(horizontalConnectionSpacer);

        hideInactiveCheckBox = new QCheckBox(layoutWidget1);
        hideInactiveCheckBox->setObjectName(QString::fromUtf8("hideInactiveCheckBox"));

        rightToolbarLayout->addWidget(hideInactiveCheckBox);

        hideInheritedCheckBox = new QCheckBox(layoutWidget1);
        hideInheritedCheckBox->setObjectName(QString::fromUtf8("hideInheritedCheckBox"));

        rightToolbarLayout->addWidget(hideInheritedCheckBox);


        rightVerticalLayout->addLayout(rightToolbarLayout);

        signalSlotSplitter = new QSplitter(layoutWidget1);
        signalSlotSplitter->setObjectName(QString::fromUtf8("signalSlotSplitter"));
        signalSlotSplitter->setOrientation(Qt::Vertical);
        signalGroupBox = new QGroupBox(signalSlotSplitter);
        signalGroupBox->setObjectName(QString::fromUtf8("signalGroupBox"));
        signalGroupBox->setFlat(true);
        gridLayout_1 = new QGridLayout(signalGroupBox);
        gridLayout_1->setObjectName(QString::fromUtf8("gridLayout_1"));
        signalTree = new QTreeView(signalGroupBox);
        signalTree->setObjectName(QString::fromUtf8("signalTree"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(signalTree->sizePolicy().hasHeightForWidth());
        signalTree->setSizePolicy(sizePolicy2);
        signalTree->setContextMenuPolicy(Qt::CustomContextMenu);
        signalTree->setProperty("showDropIndicator", QVariant(false));
        signalTree->setUniformRowHeights(false);
        signalTree->setAllColumnsShowFocus(true);

        gridLayout_1->addWidget(signalTree, 0, 0, 1, 2);

        signalSlotSplitter->addWidget(signalGroupBox);
        slotGroupBox = new QGroupBox(signalSlotSplitter);
        slotGroupBox->setObjectName(QString::fromUtf8("slotGroupBox"));
        slotGroupBox->setFlat(true);
        gridLayout_2 = new QGridLayout(slotGroupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        slotTree = new QTreeView(slotGroupBox);
        slotTree->setObjectName(QString::fromUtf8("slotTree"));
        sizePolicy2.setHeightForWidth(slotTree->sizePolicy().hasHeightForWidth());
        slotTree->setSizePolicy(sizePolicy2);
        slotTree->setContextMenuPolicy(Qt::CustomContextMenu);
        slotTree->setProperty("showDropIndicator", QVariant(false));
        slotTree->setUniformRowHeights(false);
        slotTree->setAllColumnsShowFocus(true);

        gridLayout_2->addWidget(slotTree, 0, 0, 1, 2);

        signalSlotSplitter->addWidget(slotGroupBox);

        rightVerticalLayout->addWidget(signalSlotSplitter);

        mainSplitter->addWidget(layoutWidget1);

        verticalLayout->addWidget(mainSplitter);

        tabWidget->addTab(objectTab, icon11, QString());
        signalTab = new QWidget();
        signalTab->setObjectName(QString::fromUtf8("signalTab"));
        verticalLayout_3 = new QVBoxLayout(signalTab);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        loggerOptionsGroupBox = new QGroupBox(signalTab);
        loggerOptionsGroupBox->setObjectName(QString::fromUtf8("loggerOptionsGroupBox"));
        loggerOptionsGroupBox->setFlat(true);
        verticalLayout_2 = new QVBoxLayout(loggerOptionsGroupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        optionsHorizontalLayout = new QHBoxLayout();
        optionsHorizontalLayout->setObjectName(QString::fromUtf8("optionsHorizontalLayout"));
        timestampCheckBox = new QCheckBox(loggerOptionsGroupBox);
        timestampCheckBox->setObjectName(QString::fromUtf8("timestampCheckBox"));

        optionsHorizontalLayout->addWidget(timestampCheckBox);

        objectCheckBox = new QCheckBox(loggerOptionsGroupBox);
        objectCheckBox->setObjectName(QString::fromUtf8("objectCheckBox"));
        objectCheckBox->setChecked(true);

        optionsHorizontalLayout->addWidget(objectCheckBox);

        addressCheckBox = new QCheckBox(loggerOptionsGroupBox);
        addressCheckBox->setObjectName(QString::fromUtf8("addressCheckBox"));

        optionsHorizontalLayout->addWidget(addressCheckBox);

        signatureCcheckBox = new QCheckBox(loggerOptionsGroupBox);
        signatureCcheckBox->setObjectName(QString::fromUtf8("signatureCcheckBox"));
        signatureCcheckBox->setChecked(true);

        optionsHorizontalLayout->addWidget(signatureCcheckBox);

        emitCountCheckBox = new QCheckBox(loggerOptionsGroupBox);
        emitCountCheckBox->setObjectName(QString::fromUtf8("emitCountCheckBox"));

        optionsHorizontalLayout->addWidget(emitCountCheckBox);

        argumentsCheckBox = new QCheckBox(loggerOptionsGroupBox);
        argumentsCheckBox->setObjectName(QString::fromUtf8("argumentsCheckBox"));
        argumentsCheckBox->setChecked(true);

        optionsHorizontalLayout->addWidget(argumentsCheckBox);

        line = new QFrame(loggerOptionsGroupBox);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        optionsHorizontalLayout->addWidget(line);

        prettyFormattingCheckBox = new QCheckBox(loggerOptionsGroupBox);
        prettyFormattingCheckBox->setObjectName(QString::fromUtf8("prettyFormattingCheckBox"));

        optionsHorizontalLayout->addWidget(prettyFormattingCheckBox);

        line_2 = new QFrame(loggerOptionsGroupBox);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        optionsHorizontalLayout->addWidget(line_2);

        separatorLabel = new QLabel(loggerOptionsGroupBox);
        separatorLabel->setObjectName(QString::fromUtf8("separatorLabel"));

        optionsHorizontalLayout->addWidget(separatorLabel);

        separatorLineEdit = new QLineEdit(loggerOptionsGroupBox);
        separatorLineEdit->setObjectName(QString::fromUtf8("separatorLineEdit"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(separatorLineEdit->sizePolicy().hasHeightForWidth());
        separatorLineEdit->setSizePolicy(sizePolicy3);
        separatorLineEdit->setMinimumSize(QSize(24, 0));
        separatorLineEdit->setMaximumSize(QSize(24, 16777215));
        separatorLineEdit->setMaxLength(1);
        separatorLineEdit->setFrame(true);

        optionsHorizontalLayout->addWidget(separatorLineEdit);

        horizontalSpacer = new QSpacerItem(108, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        optionsHorizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(optionsHorizontalLayout);

        exampleLayout = new QHBoxLayout();
        exampleLayout->setObjectName(QString::fromUtf8("exampleLayout"));
        exampleLayout->setContentsMargins(-1, 16, -1, 16);
        signalLogLabel = new QLabel(loggerOptionsGroupBox);
        signalLogLabel->setObjectName(QString::fromUtf8("signalLogLabel"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(signalLogLabel->sizePolicy().hasHeightForWidth());
        signalLogLabel->setSizePolicy(sizePolicy4);
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        signalLogLabel->setFont(font);

        exampleLayout->addWidget(signalLogLabel);


        verticalLayout_2->addLayout(exampleLayout);


        verticalLayout_3->addWidget(loggerOptionsGroupBox);

        spiesGroupBox = new QGroupBox(signalTab);
        spiesGroupBox->setObjectName(QString::fromUtf8("spiesGroupBox"));
        spiesGroupBox->setFlat(true);
        horizontalLayout_2 = new QHBoxLayout(spiesGroupBox);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        signalSpiesTableView = new QTableView(spiesGroupBox);
        signalSpiesTableView->setObjectName(QString::fromUtf8("signalSpiesTableView"));
        signalSpiesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
        signalSpiesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        signalSpiesTableView->setProperty("showDropIndicator", QVariant(false));
        signalSpiesTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        signalSpiesTableView->setSortingEnabled(true);

        horizontalLayout_2->addWidget(signalSpiesTableView);


        verticalLayout_3->addWidget(spiesGroupBox);

        tabWidget->addTab(signalTab, icon6, QString());

        horizontalLayout->addWidget(tabWidget);

        QWidget::setTabOrder(backToolButton, forwardToolButton);
        QWidget::setTabOrder(forwardToolButton, findLineEdit);
        QWidget::setTabOrder(findLineEdit, findToolButton);
        QWidget::setTabOrder(findToolButton, refreshToolButton);
        QWidget::setTabOrder(refreshToolButton, discoverToolButton);
        QWidget::setTabOrder(discoverToolButton, bugToolButton);
        QWidget::setTabOrder(bugToolButton, aboutToolButton);
        QWidget::setTabOrder(aboutToolButton, hideInactiveCheckBox);
        QWidget::setTabOrder(hideInactiveCheckBox, hideInheritedCheckBox);
        QWidget::setTabOrder(hideInheritedCheckBox, objectTree);
        QWidget::setTabOrder(objectTree, signalTree);
        QWidget::setTabOrder(signalTree, slotTree);
        QWidget::setTabOrder(slotTree, timestampCheckBox);
        QWidget::setTabOrder(timestampCheckBox, objectCheckBox);
        QWidget::setTabOrder(objectCheckBox, addressCheckBox);
        QWidget::setTabOrder(addressCheckBox, signatureCcheckBox);
        QWidget::setTabOrder(signatureCcheckBox, emitCountCheckBox);
        QWidget::setTabOrder(emitCountCheckBox, argumentsCheckBox);
        QWidget::setTabOrder(argumentsCheckBox, prettyFormattingCheckBox);
        QWidget::setTabOrder(prettyFormattingCheckBox, separatorLineEdit);
        QWidget::setTabOrder(separatorLineEdit, signalSpiesTableView);
        QWidget::setTabOrder(signalSpiesTableView, tabWidget);

        retranslateUi(ConanWidget);

        tabWidget->setCurrentIndex(0);
        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ConanWidget);
    } // setupUi

    void retranslateUi(QWidget *ConanWidget)
    {
        ConanWidget->setWindowTitle(QApplication::translate("ConanWidget", "Conan %1 - Connection Analyzer for Qt %2", 0, QApplication::UnicodeUTF8));
        actionBack->setText(QApplication::translate("ConanWidget", "Back", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionBack->setToolTip(QApplication::translate("ConanWidget", "Back (Alt+Left)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionBack->setShortcut(QApplication::translate("ConanWidget", "Alt+Left", 0, QApplication::UnicodeUTF8));
        actionForward->setText(QApplication::translate("ConanWidget", "Forward", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionForward->setToolTip(QApplication::translate("ConanWidget", "Forward (Alt+Right)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionForward->setShortcut(QApplication::translate("ConanWidget", "Alt+Right", 0, QApplication::UnicodeUTF8));
        actionFind->setText(QApplication::translate("ConanWidget", "Find next", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionFind->setToolTip(QApplication::translate("ConanWidget", "Find next (F3)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionFind->setShortcut(QApplication::translate("ConanWidget", "F3", 0, QApplication::UnicodeUTF8));
        actionRefresh->setText(QApplication::translate("ConanWidget", "Refresh objects", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionRefresh->setToolTip(QApplication::translate("ConanWidget", "Refresh objects (F5)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionRefresh->setShortcut(QApplication::translate("ConanWidget", "F5", 0, QApplication::UnicodeUTF8));
        actionFocusFind->setText(QApplication::translate("ConanWidget", "FocusFind", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionFocusFind->setToolTip(QApplication::translate("ConanWidget", "Focus find edit (Ctrl+F)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionFocusFind->setShortcut(QApplication::translate("ConanWidget", "Ctrl+F", 0, QApplication::UnicodeUTF8));
        actionDiscover->setText(QApplication::translate("ConanWidget", "Discover objects", 0, QApplication::UnicodeUTF8));
        actionFindMethod->setText(QApplication::translate("ConanWidget", "Find object", 0, QApplication::UnicodeUTF8));
        actionFindMethod->setIconText(QApplication::translate("ConanWidget", "Find OB", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionFindMethod->setToolTip(QApplication::translate("ConanWidget", "Find method", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSpySignal->setText(QApplication::translate("ConanWidget", "Spy signal", 0, QApplication::UnicodeUTF8));
        actionAboutConan->setText(QApplication::translate("ConanWidget", "About Conan", 0, QApplication::UnicodeUTF8));
        actionBug->setText(QApplication::translate("ConanWidget", "Find duplicate connections", 0, QApplication::UnicodeUTF8));
        actionDeleteSpies->setText(QApplication::translate("ConanWidget", "Delete", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionDeleteSpies->setToolTip(QApplication::translate("ConanWidget", "Delete (Del)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionDeleteSpies->setShortcut(QApplication::translate("ConanWidget", "Del", 0, QApplication::UnicodeUTF8));
        actionSelectAllSpies->setText(QApplication::translate("ConanWidget", "Select all", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSelectAllSpies->setToolTip(QApplication::translate("ConanWidget", "Select all signal spies (Ctrl+A)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSelectAllSpies->setShortcut(QApplication::translate("ConanWidget", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionExport->setText(QApplication::translate("ConanWidget", "Export to XML", 0, QApplication::UnicodeUTF8));
        actionRemoveRootObject->setText(QApplication::translate("ConanWidget", "Remove root object", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionRemoveRootObject->setToolTip(QApplication::translate("ConanWidget", "Remove root object", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionDisconnect->setText(QApplication::translate("ConanWidget", "Disconnect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionDisconnect->setToolTip(QApplication::translate("ConanWidget", "Disconnect method", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionDisconnectAll->setText(QApplication::translate("ConanWidget", "Disconnect all", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionDisconnectAll->setToolTip(QApplication::translate("ConanWidget", "Disconnect all methods", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionRemoveAllRootObjects->setText(QApplication::translate("ConanWidget", "Remove all root objects", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionRemoveAllRootObjects->setToolTip(QApplication::translate("ConanWidget", "Remove all root objects", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        backToolButton->setText(QApplication::translate("ConanWidget", "Back", 0, QApplication::UnicodeUTF8));
        forwardToolButton->setText(QApplication::translate("ConanWidget", "Forward", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        findLineEdit->setToolTip(QApplication::translate("ConanWidget", "Find object (Ctrl+F)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        findToolButton->setText(QApplication::translate("ConanWidget", "Find", 0, QApplication::UnicodeUTF8));
        refreshToolButton->setText(QApplication::translate("ConanWidget", "Refresh", 0, QApplication::UnicodeUTF8));
        discoverToolButton->setText(QApplication::translate("ConanWidget", "Discover widgets", 0, QApplication::UnicodeUTF8));
        bugToolButton->setText(QApplication::translate("ConanWidget", "Find duplicate connections", 0, QApplication::UnicodeUTF8));
        exportToolButton->setText(QApplication::translate("ConanWidget", "Export to XML", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        aboutToolButton->setToolTip(QApplication::translate("ConanWidget", "About CONAN", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        aboutToolButton->setText(QApplication::translate("ConanWidget", "...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        objectTree->setToolTip(QApplication::translate("ConanWidget", "Shows the object hierarchies of all root objects that have been added to Conan", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        inheritanceList->setToolTip(QApplication::translate("ConanWidget", "Shows the inheritance hierarchy from the current selected object to QObject", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolBox->setItemText(toolBox->indexOf(inheritancePage), QApplication::translate("ConanWidget", "Inheritance", 0, QApplication::UnicodeUTF8));
        toolBox->setItemToolTip(toolBox->indexOf(inheritancePage), QApplication::translate("ConanWidget", "Shows the inheritance hierarchy from the current selected object to QObject", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        classInfoTable->setToolTip(QApplication::translate("ConanWidget", "Shows the class information (Q_CLASSINFO) associated with current selected object", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolBox->setItemText(toolBox->indexOf(classInfoPage), QApplication::translate("ConanWidget", "Class information", 0, QApplication::UnicodeUTF8));
        toolBox->setItemToolTip(toolBox->indexOf(classInfoPage), QApplication::translate("ConanWidget", "Shows the class information (Q_CLASSINFO) associated with current selected object", 0, QApplication::UnicodeUTF8));
        hideInactiveCheckBox->setText(QApplication::translate("ConanWidget", "Hide Inactive Methods", 0, QApplication::UnicodeUTF8));
        hideInheritedCheckBox->setText(QApplication::translate("ConanWidget", "Hide Inherited Methods", 0, QApplication::UnicodeUTF8));
        signalGroupBox->setTitle(QApplication::translate("ConanWidget", "Signals", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        signalTree->setToolTip(QApplication::translate("ConanWidget", "Lists all signals of the current selected object, including their connections", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        slotGroupBox->setTitle(QApplication::translate("ConanWidget", "Slots", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        slotTree->setToolTip(QApplication::translate("ConanWidget", "Lists all slots of the current selected object, including their connections ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tabWidget->setTabText(tabWidget->indexOf(objectTab), QApplication::translate("ConanWidget", "Objects hierarchy", 0, QApplication::UnicodeUTF8));
        loggerOptionsGroupBox->setTitle(QApplication::translate("ConanWidget", "Logging options", 0, QApplication::UnicodeUTF8));
        timestampCheckBox->setText(QApplication::translate("ConanWidget", "Timestamp", 0, QApplication::UnicodeUTF8));
        objectCheckBox->setText(QApplication::translate("ConanWidget", "Object", 0, QApplication::UnicodeUTF8));
        addressCheckBox->setText(QApplication::translate("ConanWidget", "Address", 0, QApplication::UnicodeUTF8));
        signatureCcheckBox->setText(QApplication::translate("ConanWidget", "Signature", 0, QApplication::UnicodeUTF8));
        emitCountCheckBox->setText(QApplication::translate("ConanWidget", "Emit count", 0, QApplication::UnicodeUTF8));
        argumentsCheckBox->setText(QApplication::translate("ConanWidget", "Arguments", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        prettyFormattingCheckBox->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        prettyFormattingCheckBox->setText(QApplication::translate("ConanWidget", "Pretty formatting", 0, QApplication::UnicodeUTF8));
        separatorLabel->setText(QApplication::translate("ConanWidget", "Separator:", 0, QApplication::UnicodeUTF8));
        separatorLineEdit->setText(QApplication::translate("ConanWidget", ",", 0, QApplication::UnicodeUTF8));
        signalLogLabel->setText(QApplication::translate("ConanWidget", "signal log", 0, QApplication::UnicodeUTF8));
        spiesGroupBox->setTitle(QApplication::translate("ConanWidget", "Active signal spies", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        signalSpiesTableView->setToolTip(QApplication::translate("ConanWidget", "Lists all active signal spies", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tabWidget->setTabText(tabWidget->indexOf(signalTab), QApplication::translate("ConanWidget", "Signal spies", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ConanWidget: public Ui_ConanWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONANWIDGET_H
