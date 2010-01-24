/* -*-mode:c++; c-basic-offset:4; -*- */
/**********************************************************************************************
    Copyright (C) 2010 Albrecht Dre� <albrecht.dress@arcor.de>

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

#include "CHelpButton.h"


CHelpDialog::CHelpDialog(QWidget * parent)
: QDialog(parent)
{
    setupUi(this);
    helpIcn->setPixmap(QPixmap(":/icons/iconHelp48x48"));
    
    setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
}


static CHelpDialog *helpPopup = 0;


CHelpButton::CHelpButton(QWidget * parent)
: QToolButton(parent)
{
    this->setIcon(QPixmap(":/icons/iconHelp16x16"));
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
}


void CHelpButton::setHelp(const QString & title, const QString & contents)
{
    m_title = title;
    m_contents = contents;
}


void CHelpButton::slotClicked()
{
    if (helpPopup == 0)
        helpPopup = new CHelpDialog();

    helpPopup->setWindowTitle(m_title);
    helpPopup->setContents(m_contents);
    helpPopup->show();
    helpPopup->raise();
}
