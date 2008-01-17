/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CMegaMenu.cpp

  Module:

  Description:

  Created:     12/04/2007

  (C) 2007


**********************************************************************************************/

#include "CMegaMenu.h"
#include "CCanvas.h"

#include <QtGui>
/// Enhanced QLabel used by CMegaMenu
class CLabel : public QLabel
{
    public:
        CLabel(QWidget * parent): QLabel(parent)
        {
            setMouseTracking(true);
        }
        ~CLabel(){}

        void mouseMoveEvent(QMouseEvent * e)
        {
            setBackgroundRole(QPalette::Highlight);
            setForegroundRole(QPalette::HighlightedText);
        }

        void leaveEvent(QEvent *)
        {
            setBackgroundRole(QPalette::Window);
            setForegroundRole(QPalette::WindowText);
        }

        void paintEvent(QPaintEvent * e)
        {

            QPainter p;
            p.begin(this);
            if(backgroundRole() == QPalette::Highlight){
                p.setBrush(QBrush( QColor(66,121,206,150) ));
            }
            else{
                p.setBrush(QBrush( QColor(66,121,206,0) ));
            }
            p.setPen(Qt::NoPen);
            p.drawRect(rect());
            p.end();

            QLabel::paintEvent(e);
        }
};


const CMegaMenu::func_key_state_t CMegaMenu::fsMain[] = {
     {0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconMap16x16",QObject::tr("Map ..."),&CMegaMenu::funcSwitchToMap,tr("Manage maps.")}
    ,{":/icons/iconWaypoint16x16",QObject::tr("Waypoint ..."),&CMegaMenu::funcSwitchToWpt,tr("Manage waypoints.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
};

const CMegaMenu::func_key_state_t CMegaMenu::fsMap[] = {
     {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map.\nPress down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconSelMap16x16",QObject::tr("Select Map"),&CMegaMenu::funcSelectArea,tr("Select area of map for upload.\nSelect area by pressing down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
};

const CMegaMenu::func_key_state_t CMegaMenu::fsWpt[] = {
     {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map.\nPress down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
};


/// Left hand side multi-function menu
CMegaMenu::CMegaMenu(CCanvas * canvas)
    : QLabel(canvas)
    , canvas(canvas)
{
    setScaledContents(true);

    int i;
    QVBoxLayout * mainLayout = new QVBoxLayout(this);

    QHBoxLayout * titleLayout = new QHBoxLayout();
    mainLayout->addLayout(titleLayout);

    menuTitle = new QLabel(this);
    menuTitle->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(menuTitle);

    layout = new QGridLayout();
    mainLayout->addLayout(layout);

    keyEsc   = new QLabel("Esc",this);
    layout->addWidget(keyEsc,0,0);
    keyF1   = new QLabel("F1",this);
    layout->addWidget(keyF1,1,0);
    keyF2 = new QLabel("F2",this);
    layout->addWidget(keyF2,2,0);
    keyF3 = new QLabel("F3",this);
    layout->addWidget(keyF3,3,0);
    keyF4 = new QLabel("F4",this);
    layout->addWidget(keyF4,4,0);
    keyF5 = new QLabel("F5",this);
    layout->addWidget(keyF5,5,0);
    keyF6 = new QLabel("F6",this);
    layout->addWidget(keyF6,6,0);
    keyF7 = new QLabel("F7",this);
    layout->addWidget(keyF7,7,0);
    keyF8 = new QLabel("F8",this);
    layout->addWidget(keyF8,8,0);
    keyF9 = new QLabel("F9",this);
    layout->addWidget(keyF9,9,0);
    keyF10 = new QLabel("F10",this);
    layout->addWidget(keyF10,10,0);

    for(i=0; i<11; ++i){
        icons[i] = new QLabel(this);
        layout->addWidget(icons[i],i,1);
    }

    for(i=0; i<11; ++i){
        names[i] = new CLabel(this);
        names[i]->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
        layout->addWidget(names[i],i,2);
    }

    funcSwitchToMain();
}

CMegaMenu::~CMegaMenu()
{

}

void CMegaMenu::switchState(const func_key_state_t statedef[11])
{
    unsigned i;



    for(i=0; i<11; ++i){
        if(statedef[i].icon){
            icons[i]->setPixmap(QPixmap(statedef[i].icon));
        }
        else{
            icons[i]->setPixmap(QPixmap());
        }
        names[i]->setText(statedef[i].name);
        names[i]->setToolTip(statedef[i].tooltip);
    }

    current = statedef;
}


void CMegaMenu::keyPressEvent(QKeyEvent * e)
{

    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)){
        unsigned i = e->key() - Qt::Key_F1 + 1;
        if(current[i].func){
            (this->*current[i].func)();

        }
        return e->accept();
    }
    else if(e->key() == Qt::Key_Escape){
        if(current[0].func){
            (this->*current[0].func)();
        }
        return e->accept();
    }
    else if(e->key() == Qt::Key_Plus){
        canvas->zoom(true, canvas->geometry().center());
        return e->accept();
    }
    else if(e->key() == Qt::Key_Minus){
        canvas->zoom(false, canvas->geometry().center());
        return e->accept();
    }
    else if(e->key() == Qt::Key_Left){
        canvas->move(CCanvas::eMoveLeft);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Right){
        canvas->move(CCanvas::eMoveRight);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Up){
        canvas->move(CCanvas::eMoveUp);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Down){
        canvas->move(CCanvas::eMoveDown);
        return e->accept();
    }
}

void CMegaMenu::mousePressEvent(QMouseEvent * e)
{
    unsigned i;

    if(e->button() != Qt::LeftButton) return;

    for(i=0; i<11; ++i){
        if(names[i]->geometry().contains(e->pos())){
            if(current[i].func){
                (this->*current[i].func)();
            }
            return;
        }
    }
}


void CMegaMenu::funcSwitchToMain()
{

    menuTitle->setText(tr("<b>Main ...</b>"));
    setPixmap(QPixmap(":/icons/backGlobe128x128"));
    switchState(fsMain);
    funcMoveArea();
}

void CMegaMenu::funcSwitchToMap()
{
    menuTitle->setText(tr("<b>Maps ...</b>"));
    setPixmap(QPixmap(":/icons/backMap128x128"));
    switchState(fsMap);
//     gpResources->mapdb().gainFocus();
    funcMoveArea();
}

void CMegaMenu::funcSwitchToWpt()
{
    menuTitle->setText(tr("<b>Waypoints ...</b>"));
    setPixmap(QPixmap(":/icons/backWaypoint128x128"));
    switchState(fsWpt);
//     gpResources->mapdb().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcMoveArea()
{
    canvas->setMouseMode(CCanvas::eMouseMoveArea);
}

void CMegaMenu::funcSelectArea()
{
    canvas->setMouseMode(CCanvas::eMouseSelectArea);
}
