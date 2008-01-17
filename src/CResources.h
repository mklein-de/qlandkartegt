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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#ifndef CRESOURCES_H
#define CRESOURCES_H

#include <QObject>

/// all global resources
class CResources : public QObject
{
    Q_OBJECT
    public:
        virtual ~CResources();

        static CResources& self(){return *m_self;}


        /// get HTTP proxy settings
        /**
            @param url a string to store the proxy's URL
            @param port a 16bit unsigned integer to store the proxy's port

            @return The method will return true if the proxy is enabled.
        */
        bool getHttpProxy(QString& url, quint16& port);

    private:
        friend class CMainWindow;
        CResources(QObject * parent);

        static CResources * m_self;

        /// use proxy for http requests
        bool m_useHttpProxy;
        /// the  proxy name or address
        QString m_httpProxy;
        /// the proxy port
        quint16 m_httpProxyPort;

};

#endif //CRESOURCES_H

