#include "CMapOSMType.h"

#include <QtGui>

CMapOSMType::CMapOSMType(QString title, QString path)
{
    this->title=title;
    this->path=path;
    this->builtin=false;
}


void CMapOSMType::setBuiltin(QString key)
{
    this->builtin=true;
    this->key=key;
}


void CMapOSMType::setEnabled(bool state)
{
    this->enabled=state;
}


bool CMapOSMType::isBuiltin()
{
    return this->builtin;
}


bool CMapOSMType::isEnabled()
{
    return this->builtin ? this->enabled : true;
}
