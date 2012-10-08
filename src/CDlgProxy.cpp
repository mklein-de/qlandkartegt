/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgProxy.h"
#include "CResources.h"
#include "CMainWindow.h"

#include <QtGui>

CDlgProxy::CDlgProxy(QString &user, QString &pwd, QWidget *parent)
: QDialog(parent)
, user(user)
, pwd(pwd)
{
    setupUi(this);

    QString url;
    quint16 port;
    CResources::self().getHttpProxy(url, port);

    iconLabel->setText(QString());
    iconLabel->setPixmap(theMainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, theMainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(Qt::escape(url));
    introLabel->setText(introMessage);
    introLabel->setWordWrap(true);

}

CDlgProxy::~CDlgProxy()
{

}

void CDlgProxy::accept()
{
    user    = userNameLineEdit->text();
    pwd     = passwordLineEdit->text();
    QDialog::accept();
}
