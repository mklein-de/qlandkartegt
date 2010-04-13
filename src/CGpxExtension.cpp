// Includes
#include "CGpxExtension.h"



//TODO: Methode zum auslesen der extension aus dem gpx file
void CGpxExtPt::setValues(const QDomNode& parent)
{
  QDomNode child = parent.firstChild();
    while (!child.isNull())
    {
     if (child.isElement()) {values.insert(child.nodeName(), child.toElement().text());}
        child = child.nextSibling();
    }	
}



//TODO: Methode zur ermittlung der anzahl der extentions pro pt
int CGpxExtPt::getSize()
{
	return values.size();
}

QString CGpxExtPt::getName(const QString& val)
{
	return values.key(val);
}


//TODO: Methode um den Wert der Extension X zu erhalten
QString CGpxExtPt::getValue (const QString& name)
{
	return values.value(name);
}
	

//TODO: Methode um die Namen der Extensions zu listen
void CGpxExtTr::addKey2List(const QDomNode& parent)
{
  QDomNode child = parent.firstChild();

    while (!child.isNull())
    {
        if (child.isElement())
        {
				set.insert(child.nodeName());		
        }
		child = child.nextSibling();

    }
}

