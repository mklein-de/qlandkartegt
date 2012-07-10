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
    \brief Contains AboutDialog definition
*/


#include "AboutDialog.h"
#include "ConanDefines.h"


namespace conan {

    AboutDialog::AboutDialog (QWidget* inParent, Qt::WindowFlags inFlags) :
        QDialog (inParent, inFlags)
    {
        // remove 'what's this' help button
        setWindowFlags (windowFlags () & ~Qt::WindowContextHelpButtonHint);

        mForm.setupUi (this);
        QString msg = mForm.textLabel->text ().arg (CONAN_VERSION_STR).arg (QT_VERSION_STR);
        mForm.textLabel->setText (msg);
    }

} // namespace conan
