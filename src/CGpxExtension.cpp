/**********************************************************************************************
	Copyright (C) 2010 Christian Treffs ctreffs@gmail.com

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

// Includes
#include "CGpxExtension.h"

//TODO: Methode zum auslesen der extension aus dem gpx file
void CGpxExtPt::setValues(const QDomNode& parent)
{
    QDomNode child = parent.firstChild();
    while (!child.isNull())
    {
        if (child.isElement()) {values.insert(child.nodeName(), child.toElement().text());}
        child = child.nextSibling();
    }
}


//TODO: Methode zur ermittlung der anzahl der extentions pro pt
int CGpxExtPt::getSize()
{
    return values.size();
}

//TODO: Methode um den Wert der Extension X zu erhalten
QString CGpxExtPt::getValue (const QString& name)
{
    return values.value(name);
}


//TODO: Methode um die Namen der Extensions zu listen
void CGpxExtTr::addKey2List(const QDomNode& parent)
{
    QDomNode child = parent.firstChild();

    while (!child.isNull())
    {
        if (child.isElement())
        {
            set.insert(child.nodeName());
        }
        child = child.nextSibling();

    }
}
