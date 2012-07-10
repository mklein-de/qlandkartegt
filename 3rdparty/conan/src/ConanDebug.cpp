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
    \brief Contains Contains Conan debug function definitions
*/

#include "ConanDebug.h"
#include "ConnectionModel.h"
#include "ObjectUtility.h"
#include <QtCore/QObject>
#include <QtCore/QtDebug>


namespace conan {

    bool debug::gEnabled = false;

    /*!
        \brief Dumps information about signal connections for the given object to the debug output.
        \param[in] inObject The object
    */
    void debug::Dump (QObject* inObject) {
        if (gEnabled && inObject)
            inObject->dumpObjectInfo ();
    }

    /*!
        \brief Dumps the given connection data to the debug output.
        \param[in] inData The connection data
    */
    void debug::Dump (const ConnectionData& inData) {
        if (!gEnabled)
            return;

        qDebug ()   << ObjectUtility::QualifiedName (inData.mMethod.mClass, inData.mMethod.mName)
                    << inData.mMethod.mAddress
                    << inData.mMethod.mSignature;

        QString prefix = inData.mMethod.mMethodType == QMetaMethod::Signal ? "-->" : "<--";

        foreach (const MethodData& method, inData.mConnections)
        {
            qDebug ()   << prefix
                        << ObjectUtility::QualifiedName (method.mClass, method.mName)
                        << method.mAddress
                        << method.mSignature;
        }
    }

} // namespace conan
