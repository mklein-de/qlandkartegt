//TODO: Includes & Abhängigkeiten
#ifndef CGPXEXTENSION_H
#define CGPXEXTENSION_H

#include <QMap>
#include <QDomElement>
#include <QDomNode>
#include <QString>
#include <QSet>
#include <QList>


//TODO: C: extensions auslesen und verarbeiten
class CGpxExtPt
{
 public:    
    
	 CGpxExtPt(){};								// der Default-Konstruktor
	 ~CGpxExtPt(){};								// der Destruktor
 	
    void setValues(const QDomNode& parent);		//Methode um die extensions aus dem xml file in eine values map zu packen
	int getSize();		
	QString getName (const QString& val);					//abfrage über die anzahl der extension pro pt
	QString getValue (const QString& name);		//ausgabe des wertes der extension
	
	QMap<QString, QString> values;				//deklaration der QMap values, die die extensions enthält

private:                     
	
	
	

};

class CGpxExtTr
{
public:
	void addKey2List(const QDomNode& parent);
		QList<QString> list;					// Deklaration einer liste in die die QMap namen sollen	
		int lsize;
		QSet<QString> set;	// Deklaration eines set in die die QMap namen sollen

private:



};
#endif
