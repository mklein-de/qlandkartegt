#include "CMapOSMType.h"

#include <QtGui>

CMapOSMType::CMapOSMType()
{
    this->title=QString("");
    this->path=QString("");
    this->builtin=false;
}

CMapOSMType::CMapOSMType(QString title, QString path)
{
    this->title=title;
    this->path=path;
    this->builtin=false;
}

CMapOSMType::CMapOSMType(QString title, QString path, QString key)
{
    this->title=title;
    this->path=path;
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


