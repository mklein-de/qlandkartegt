/**********************************************************************************************
    Copyright (C) 2006, 2007 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CGPX_H
#define CGPX_H

#include <QObject>
#include <QMap>
#include <QColor>
#include <QString>
#include <QtXml/QDomDocument>

#include "Dictionary.h"

/// handle geo data from GPX files
class CGpx : public QObject, public QDomDocument
{
    Q_OBJECT;

    public:
        // Those are standard GPX/XML namespaces
        static const QString gpx_ns;
        static const QString xsi_ns;

        // Those are the URIs of the GPX extensions we support
        static const QString gpxx_ns;
        static const QString gpxtpx_ns;
        static const QString rmc_ns;

    public:
        CGpx(QObject * parent);
        virtual ~CGpx();

        void load(const QString& filename);
        void save(const QString& filename);

        static QMap<QString,QDomElement> mapChildElements(const QDomNode& parent);
	
        const Dictionary<QString, QColor>& getColorMap() const;
        const Dictionary<QString, int>& getTrackColorMap() const;

    protected:
        void writeMetadata();

        Dictionary<QString, QColor> colorMap;
        Dictionary<QString, int> trackColorMap;
};
#endif                           //CGPX_H
