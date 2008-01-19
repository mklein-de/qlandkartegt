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
#include <QFont>

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

        /// set time offset to UTC in multiples of hours
        /**
            @param offset the offset in multiple of hours
        */
        void setUTCOffset(int offset, int fract);

        uint32_t getUTCOffset(){return time_offset;}


    signals:
        void sigProxyChanged();

    private:
        friend class CMainWindow;
        friend class CDlgConfig;
        CResources(QObject * parent);

        static CResources * m_self;

        enum bowser_e   {eFirefox = 0
                        ,eKonqueror = 1
                        ,eOther = 2
                        };


        /// use proxy for http requests
        bool m_useHttpProxy;
        /// the  proxy name or address
        QString m_httpProxy;
        /// the proxy port
        quint16 m_httpProxyPort;

        /// font used by the map
        QFont m_mapfont;
        /// true for metric system, false for imperial
        bool m_doMetric;
        /// offset to UTC in multiples of hours
        qint32 m_offsetUTC;
        /// offset to UTC fractional part
        qint32 m_offsetUTCfract;

        /// the installed browser type
        bowser_e m_eBrowser;
        /// command string to start Firefox
        QString cmdFirefox;
        /// command string to start Konqueror
        QString cmdKonqueror;
        /// user defined command string
        QString cmdOther;

        /// this offset is needed to correct time in seconds until Dec. 30th, 1989 12:00 to POSIX standard
        uint32_t time_offset;

};

#endif //CRESOURCES_H

