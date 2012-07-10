/*!
    Conan - Connection Analyzer for Qt
    Copyright (C) 2008 - 2011 Elmar de Koning, edekoning@gmail.com

    This file is part of Conan.

    Conan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Conan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Conan.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
    \file
    \brief Contains QObject helper function declarations
*/


#ifndef _OBJECTUTILITY__05_01_09__11_40_36__H_
#define _OBJECTUTILITY__05_01_09__11_40_36__H_


class QObject;
class QString;


namespace conan {

    //! Contains QObject helper functions
    namespace ObjectUtility {
        const QObject* FindMutualParent (const QObject* inObject1, const QObject* inObject2);
        const QObject* TopLevelParent (const QObject* inObject);

        QString Name (const QObject* inObject);
        QString Class (const QObject* inObject);
        QString QualifiedName (const QObject* inObject);
        QString QualifiedName (const QString& inClass, const QString& inName);
        QString Address (const QObject* inObject);
    }    // namespace ObjectUtility

} // namespace conan


#endif //_OBJECTUTILITY__05_01_09__11_40_36__H_
