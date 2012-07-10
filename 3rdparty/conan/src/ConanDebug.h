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
    \brief Contains Conan debug function declarations
*/


#ifndef _CONANDEBUG__05_01_09__11_22_08__H_
#define _CONANDEBUG__05_01_09__11_22_08__H_


class QObject;


namespace conan {

    struct ConnectionData;

    //! Contains helper functions for debugging Conan
    namespace debug {
        extern bool gEnabled;
        void Dump (QObject* inObject);
        void Dump (const ConnectionData& inData);
    } // namespace debug

} // namespace conan

#endif //_CONANDEBUG__05_01_09__11_22_08__H_
