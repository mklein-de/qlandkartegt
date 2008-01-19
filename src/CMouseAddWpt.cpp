/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseAddWpt.cpp

  Module:

  Description:

  Created:     01/19/2008

  (C) 2008


**********************************************************************************************/

#include "CMouseAddWpt.h"
#include "CCanvas.h"

#include <QtGui>

CMouseAddWpt::CMouseAddWpt(CCanvas * canvas)
    : IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorAdd"),0,0);
}

CMouseAddWpt::~CMouseAddWpt()
{

}

void CMouseAddWpt::mouseMoveEvent(QMouseEvent * e)
{

}

void CMouseAddWpt::mousePressEvent(QMouseEvent * e)
{
    canvas->setMouseMode(CCanvas::eMouseMoveArea);

}

void CMouseAddWpt::mouseReleaseEvent(QMouseEvent * e)
{

}
