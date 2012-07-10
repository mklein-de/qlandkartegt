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
    \brief Contains ConanWidget definition
*/


#ifndef _WAITCURSOR__08_10_00__19_21_59__H_
#define _WAITCURSOR__08_10_00__19_21_59__H_


#include "ConanDefines.h"
#include <QtGui/QApplication>


namespace conan {
    /*!
        \brief Helper class for changing the cursor to a wait cursor.
    */
    class CONAN_LOCAL WaitCursor {
    public:
        WaitCursor () : mEnabled (false) {
            Enable ();
        }
        ~WaitCursor () {
            Disable ();
        }
        void Disable () {
            if (mEnabled) {
                mEnabled = false;
                QApplication::restoreOverrideCursor ();
            }
        }
        void Enable () {
            if (!mEnabled) {
                mEnabled = true;
                QApplication::setOverrideCursor (Qt::WaitCursor);
            }
        }
    private:
        bool mEnabled;
    };
} // namespace conan


#endif //_WAITCURSOR__08_10_00__19_21_59__H_