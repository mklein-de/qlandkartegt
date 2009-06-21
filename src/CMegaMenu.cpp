/**********************************************************************************************
 Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CDiaryDB.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CRouteDB.h"
#include "CTrackToolWidget.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IDevice.h"
#include "CDlgCreateWorldBasemap.h"
#include "CMenus.h"
#include "CActions.h"
#ifdef PLOT_3D
#include "CMap3DWidget.h"
#endif

#include <QtGui>

CMegaMenu * CMegaMenu::m_self = 0;

/// Left hand side multi-function menu
CMegaMenu::CMegaMenu(CCanvas * canvas)
: QLabel(canvas)
, canvas(canvas)
, currentItemIndex(-1)
, mouseDown(false)
{
    m_self = this;
    setScaledContents(true);
    setMouseTracking(true);

    actionGroup = theMainWindow->getActionGroupProvider();
    actions     = actionGroup->getActions();

}


CMegaMenu::~CMegaMenu()
{

}


void CMegaMenu::switchState()
{
    QList<QAction *> acts = QWidget::actions();
    QAction * act;
    foreach(act,acts) {
        removeAction(act);
    }
    actionGroup->addActionsToWidget(this);

    title = tr("%1 ...").arg(objectName().remove('&'));

}


void CMegaMenu::switchByKeyWord(const QString& key)
{
    if (!isEnabled())
        return;

    if (key == "Main") {
        actions->funcSwitchToMain();
    } else if (key == "Waypoints")
    {
        actions->funcSwitchToWpt();
        actions->funcMoveArea();
    } else if (key == "Search")
    {
        actions->funcSwitchToMain();
        actions->funcMoveArea();
    } else if (key == "Maps")
    {
        actions->funcSwitchToMap();
        actions->funcMoveArea();
    } else if (key == "Tracks")
    {
        actions->funcSwitchToTrack();
        actions->funcMoveArea();
    } else if (key == "LiveLog")
    {
        actions->funcSwitchToLiveLog();
        actions->funcMoveArea();
    } else if (key == "Overlay")
    {
        actions->funcSwitchToOverlay();
        actions->funcMoveArea();
    } else if (key == "Routes")
    {
        actions->funcSwitchToRoute();
        actions->funcMoveArea();
    }

}


/*!
    Initialize \a option with the values from this menu and information from \a action. This method
    is useful for subclasses when they need a QStyleOptionMenuItem, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom() QMenuBar::initStyleOption()
*/
void CMegaMenu::initStyleOption(QStyleOptionMenuItem *option, const QAction *action, bool isCurrent) const
{
    if (!option || !action)
        return;

    option->initFrom(this);
    option->palette = palette();
    option->palette.setBrush(QPalette::Normal, QPalette::Button, Qt::NoBrush);
    option->palette.setBrush(QPalette::Inactive, QPalette::Button, Qt::NoBrush);
    option->state = QStyle::State_None;

    if (window()->isActiveWindow()){
        option->state |= QStyle::State_Active;
    }
    if (isEnabled() && action->isEnabled() && (!action->menu() || action->menu()->isEnabled())){
        option->state |= QStyle::State_Enabled;
    }
    else{
        option->palette.setCurrentColorGroup(QPalette::Disabled);
    }

    option->font = action->font();

    if (isCurrent && !action->isSeparator()) {
        option->state |= QStyle::State_Selected | (mouseDown ? QStyle::State_Sunken : QStyle::State_None);
    }

    //     option->menuHasCheckableItems = d->hasCheckableItems;
    if (!action->isCheckable()) {
        option->checkType = QStyleOptionMenuItem::NotCheckable;
    }
    else {
        option->checkType = (action->actionGroup() && action->actionGroup()->isExclusive())
            ? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
        option->checked = action->isChecked();
    }
    if (action->menu())
        option->menuItemType = QStyleOptionMenuItem::SubMenu;
    else if (action->isSeparator())
        option->menuItemType = QStyleOptionMenuItem::Separator;

    else
        option->menuItemType = QStyleOptionMenuItem::Normal;
    if (action->isIconVisibleInMenu())
        option->icon = action->icon();
    QString textAndAccel = action->text();

    if (textAndAccel.indexOf(QLatin1Char('\t')) == -1) {
        QKeySequence seq = action->shortcut();
        if (!seq.isEmpty())
            textAndAccel += QLatin1Char('\t') + QString(seq);
    }

    option->text            = textAndAccel;

    QFontMetrics fm(option->font);
    option->tabWidth        = fm.width("ESC");
    option->maxIconWidth    = 16;
    option->menuRect        = rect();
}


void CMegaMenu::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);

    QPainter p(this);

    QFont f = font();
    f.setBold(true);
    p.setFont(f);

    p.setClipRegion(rectTitle);
    p.drawText(rectTitle, Qt::AlignCenter, title);

    p.setFont(font());

    int idx = 0;
    QList<QAction *> acts = QWidget::actions();
    QAction * act;
    foreach(act,acts) {
        p.setClipRegion(rectF[idx]);

        QStyleOptionMenuItem opt;
        initStyleOption(&opt, act, currentItemIndex == idx);

        opt.rect = rectF[idx];

        style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);

        ++idx;
    }
}


void CMegaMenu::resizeEvent(QResizeEvent * e)
{
    QFont f = font();
    f.setBold(true);
    QFontMetrics fm(f);

    int yoff    = 0;
    int w       = e->size().width();
    int h       = fm.height();

    rectTitle = QRect(0,yoff, w, h);
    for(int i=0; i < 11; ++i) {
        yoff += h;
        rectF[i] = QRect(0,yoff, w, h);
    }

    yoff += h;
    setMinimumHeight(yoff);
}


void CMegaMenu::leaveEvent ( QEvent * event )
{
    currentItemIndex = -1;
    mouseDown = false;
    update();
}


void CMegaMenu::mousePressEvent(QMouseEvent * e)
{
    mouseDown = true;
    update();
}


void CMegaMenu::mouseReleaseEvent(QMouseEvent * e)
{
    QList<QAction*> acts = QWidget::actions();

    QPoint pos = e->pos();
    currentItemIndex = -1;
    for(int i = 0; i < 11; ++i) {
        if(rectF[i].contains(pos)) {
            acts[i]->trigger();
            break;
        }
    }

    mouseDown = false;
    update();
}


void CMegaMenu::mouseMoveEvent(QMouseEvent * e)
{
    QPoint pos = e->pos();
    currentItemIndex = -1;
    for(int i = 0; i < 11; ++i) {
        if(rectF[i].contains(pos)) {
            currentItemIndex = i;
            update();
            return;
        }
    }
}
