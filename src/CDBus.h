/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CDBUS_H
#define CDBUS_H

#include <QtDBus>

class CDBus : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "QLandkarteGT")

    public:
        virtual ~CDBus();

    public slots:
        void addGeoData(const QString& filename);
        void loadGeoData(const QString& filename);
        void zoomToRect(const double lon1, const double lat1, const double lon2, const double lat2);

    private:
        friend class CMainWindow;
        CDBus(QObject * parent);
};

#endif //CDBUS_H

