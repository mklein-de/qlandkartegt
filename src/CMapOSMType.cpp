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

bool CMapOSMType::isBuiltin()
{
    return this->builtin;
}

