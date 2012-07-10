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
    \brief Contains QObject helper function definitions
*/

#include "ObjectUtility.h"
#include <QtCore/QObject>


namespace conan {

    /*!
        \brief Determines the mutual parent of two given objects.
        \param[in] inObject1    The first object
        \param[in] inObject2    The second object
        \return                 The mutual parent if it exists; otherwise 0
    */
    const QObject* ObjectUtility::FindMutualParent (const QObject* inObject1, const QObject* inObject2) {
        if (inObject1 && inObject2) {
            while (inObject1->parent ()) {
                inObject1 = inObject1->parent ();
                const QObject* tmp = inObject2;
                while (inObject2->parent ()) {
                    inObject2 = inObject2->parent ();
                    if (inObject1 == inObject2) {
                        return inObject1;
                    }
                }
                inObject2 = tmp;
            }
        }
        return 0;
    }

    /*!
        \brief Determines the top level parent of the given object.
        \param[in] inObject        The object
        \return                    The top level parent if it exists; otherwise 0
    */
    const QObject* ObjectUtility::TopLevelParent (const QObject* inObject) {
        if (inObject) {
            while (inObject->parent ()) {
                inObject = inObject->parent ();
            }
        }
        return inObject;
    }

    /*!
        \brief Determines the name of the given object.
        \param[in] inObject        The object
        \return                    The name of the object if it exists; otherwise 'unnamed'
    */
    QString ObjectUtility::Name (const QObject* inObject) {
        if (inObject) {
            QString name = inObject->objectName ();
            if (!name.isEmpty ()) {
                return name;
            }
        }
        return "unnamed";
    }

    /*!
        \brief Determines the class name of the given object.
        \param[in] inObject        The object
        \return                    The class name of the object if it exists; otherwise an empty string

    */
    QString ObjectUtility::Class (const QObject* inObject) {
        if (inObject && inObject->metaObject ()) {
            return inObject->metaObject ()->className ();
        }
        return QString ();
    }

    /*!
        \brief Determines the qualified name of the given object: [class name] :: [object name]
        \param[in] inObject        The object
        \return                    The qualified name of the object if it exists; otherwise an empty string
    */
    QString ObjectUtility::QualifiedName (const QObject* inObject) {
        return QualifiedName (Class (inObject), Name (inObject));
    }

    /*!
        \brief Builds a qualified name for an object from its given class name and object name: [class name] :: [object name]
        \param[in] inClass      The object's class name
        \param[in] inName       The object's name
        \return                 The qualified name: [\p inClass] :: [\p inName]
    */
    QString ObjectUtility::QualifiedName (const QString& inClass, const QString& inName) {
        return QString ("%1 :: %2").
            arg (inClass).
            arg (inName);
    }

    /*!
        \brief Determines the address of the given object.
        \param[in] inObject        The object
        \return                    The address of the object if it exists; otherwise 0x00000000
    */
    QString ObjectUtility::Address (const QObject* inObject) {
        return QString ("0x%1").arg (reinterpret_cast <quintptr> (inObject), sizeof(quintptr)*2, 16, QChar('0'));
    }

} // namespace conan
