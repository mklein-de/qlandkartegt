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
    \brief Contains AboutDialog declaration
*/


#ifndef _ABOUTDIALOG__08_01_09__20_12_49__H_
#define _ABOUTDIALOG__08_01_09__20_12_49__H_


#include "ConanDefines.h"
#include "ui_AboutDialog.h"
#include <QtGui/QDialog>


namespace conan {

    //! A simple 'about' dialog containing information about Conan
    class CONAN_API AboutDialog : public QDialog
    {
    public:
        AboutDialog (QWidget* inParent=0, Qt::WindowFlags inFlags=0);

    private:
        Ui::AboutDialog mForm;
    };

} // namespace conan


#endif // _ABOUTDIALOG__08_01_09__20_12_49__H_
