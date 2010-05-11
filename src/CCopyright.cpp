/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#include "CCopyright.h"
#include "version.h"
#include <gdal.h>
#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif

CCopyright::CCopyright()
{
    setupUi(this);
    verQLandkarte->setText(VER_STR);
    verQt->setText(qVersion());
    verGDAL->setText(GDALVersionInfo("--version"));
    verProj4->setText(QString::number(PJ_VERSION));

    textCopyright->setHtml(tr(""
        "<p>"
        "&#169; 2007 Oliver Eichler (oliver.eichler@gmx.de)"
        "</p>"
        "<p>Thanks for contributing to the project:</p>"
        "<p>"
        "Andrew Vagin<br/>"
        "Fabrice Crohas<br/>"
        "Marc Feld<br/>"
        "Joerg Wunsch<br/>"
        "Albrecht Dre&szlig;<br/>"
        "Christian Ehrlicher<br/>"
        "Christian Treffs<br/>"        
        "</p>"
        "<p>Translation:<br/>"
        "Sarah Neumann (German)<br/>"
        "Fabrice Crohas (French)<br/>"
        "Alessandro Briosi (Italian)<br/>"
        "Mike Markov (Russian)<br/>"
        "Oscar A. (Spanish)<br/>"
        "Harrie Klomp (Dutch)<br/>"
        "</p>"
        "<p>"
        "Icons and eye candy are from the <b>KDE</b> icon set, the <b>Nuvola</b> icon set and "
        "the <b>Oxygen</b> icon set."
        "See <b>http://www.kde.org/</b>,<b>http://www.icon-king.com/</b> and <b>http://www.oxygen-icons.org/</b>. "
        "Waypoint icons are copied from <b>GPSMan</b>. See <b>http://www.ncc.up.pt/gpsman/</b>. Cursor icons "
        "are from the 'Polar Cursor Theme'. See <b>http://www.kde-look.org/content/show.php?content=27913</b>."
        "</p> "
        "<p>"
        "Some of the 2D polygon math is copied from <b>http://local.wasp.uwa.edu.au/~pbourke/geometry/</b>. "
        "The geodesic distance calculation by Thaddeus Vincenty is copied from "
        "<b>http://www.movable-type.co.uk/scripts/LatLongVincenty.html</b>"
        "</p>"
        "<p>"
        "Typ file decoding is done with the help of the source code published at http://ati.land.cz/gps/typedit/"
        "/<p>"
        ));
}


CCopyright::~CCopyright()
{

}
