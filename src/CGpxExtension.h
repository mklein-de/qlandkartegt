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

//TODO: Includes & Abhngigkeiten
#ifndef CGPXEXTENSION_H
#define CGPXEXTENSION_H

#include <QMap>
#include <QDomElement>
#include <QDomNode>
#include <QString>
#include <QSet>
#include <QList>

//TODO: C: extensions auslesen und verarbeiten
class CGpxExtPt
{
    public:

        CGpxExtPt()              // der Default-Konstruktor
        {
        };
        ~CGpxExtPt()             // der Destruktor
        {
        };

                                 //Methode um die extensions aus dem xml file in eine values map zu packen
        void setValues(const QDomNode& parent);
        int getSize();
                                 //abfrage ber die anzahl der extension pro pt
        QString getName (const QString& val);
                                 //ausgabe des wertes der extension
        QString getValue (const QString& name);

                                 //deklaration der QMap values, die die extensions enthlt
        QMap<QString, QString> values;

    private:

};

class CGpxExtTr
{
    public:
        void addKey2List(const QDomNode& parent);
        QList<QString> list;     // Deklaration einer liste in die die QMap namen sollen
        int lsize;
        QSet<QString> set;       // Deklaration eines set in die die QMap namen sollen

    private:

};
#endif