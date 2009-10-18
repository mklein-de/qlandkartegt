/***************************************************************************
 *   Copyright (C) 2008 by Manuel Schrape                                  *
 *   manuel.schrape@gmx.de                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QColor>
#include <QBitArray>

//#include "Parameter_Positions.h"

#ifdef _WIN32_
static const QString OS_PORT = "COM1";
#else
static const QString OS_PORT = "/dev/ttyUSB0";
#endif

//#define _EEEPC_

// Version des Seriellen Protokoll
static const int VERSION_SERIAL_MAJOR = 10;
static const int VERSION_SERIAL_MINOR = 0;

// Basis-Addressen der verschiedenen Hardware
static const int ADDRESS_ALL    = 0;
static const int ADDRESS_FC     = 1;
static const int ADDRESS_NC     = 2;
static const int ADDRESS_MK3MAG = 3;

static const int SETTINGS_ID = 2;

static const int SLEEP = 500000;

#ifdef _EEEPC_
    static const int MeterSize = 125;
    static const int TICKER = 5000;
#else
    static const int MeterSize = 160;
    static const int TICKER = 2000;
#endif

static const QString QA_NAME = "QMK-Groundstation";
static const QString QA_VERSION_NR = "1.0.0";

#ifdef _BETA_
    static const QString QA_VERSION = QA_VERSION_NR + " (BETA)";
    static const QString QA_HWVERSION = "FlightCtrl v0.75a & NaviCtrl v0.15c";
#else
    static const QString QA_VERSION = QA_VERSION_NR;
    static const QString QA_HWVERSION = "FlightCtrl v0.75a & NaviCtrl v0.15c";
#endif

#ifdef Q_OS_LINUX
    static const QString QA_OS = "Linux";
#else
    #ifdef Q_OS_DARWIN
        static const QString QA_OS = "OSX";
    #else
        #ifdef Q_OS_WIN32
            static const QString QA_OS = "Windows";
        #else
            static const QString QA_OS = "n/a";
        #endif
    #endif
#endif


static const QString QA_DATE = "19.07.2009";
static const QString QA_YEAR = "2008-2009";
static const QString QA_AUTHOR = "Manuel Schrape";
static const QString QA_EMAIL  = "manuel.schrape@gmx.de";

static const QString QA_ABOUT =
    "<HTML>"
    "<p><b><font size=8>" + QA_NAME + "</font></b></p>"
    "<br />"
    "Version " + QA_VERSION + " - " + QA_DATE + " on " + QA_OS + ""
    "<br /><br /><b>kompatibel zu " + QA_HWVERSION + "</b>"
    "<br /><br />"
    "(C) " + QA_YEAR + " by " + QA_AUTHOR + " - "
    "<a href=\"mailto:" + QA_EMAIL + "\">" + QA_EMAIL + "</a> <br /><br />"
    "Groundstation-Programm f&uuml;r den Mikrokopter zum <br>Parametrieren und Debug-Werte anzeigen, aufzeichnen & Visualisieren."
    "<br /><br /> Dieses Programm wird unter den Bedingungen der GPL v2 ver&ouml;ffentlicht."
    "</HTML>";

static const QString Def_AnalogNames[] = {"Integral Nick", "Integral Roll", "ACC Nick", "ACC Roll", "Gyro Gier", "Hoehen-Wert", "ACC Z", "GAS", "Kompass-Value", "Spannung", "Empfang", "Ersatzkompass", "Motor Vorne", "Motor Hinten", "Motor Links", "Motor Rechts", "Analog 16", "Distance", "OSD-Bar", "MK3Mag", "Servo", "Nick", "Roll", "Analog 23", "Analog 24",  "Analog 25",  "Analog 26",  "Kalman Max",  "Analog 28",  "Kalman K", "GPS Nick", "GPS Roll"};

static const QRgb Def_Colors[] = {0x00FF0000, 0x0000FF00, 0x00FFFF00, 0x000000FF, 0x00FF00FF, 0x0000FFFF, 0x00FFFFFF, 0x00660000, 0x00006600, 0x00666600, 0x00000066, 0x00660066, 0x000066, 0x00666666, 0x00990000, 0x00009900, 0x00999900, 0x00000099, 0x00990099, 0x00009999, 0x00999999, 0x00CC0000, 0x0000CC00, 0x00CCCC00, 0x000000CC, 0x00CC00CC, 0x0000CCCC, 0x00CCCCCC, 0x0066FF99, 0x009966FF, 0x00FF9966, 0x0099FF66};

static const bool Def_Plot_Show[] = {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0};
static const bool Def_Log[]       = {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0};

static const QString HardwareType[] = {"Default", "FlightCtrl", "NaviCtrl", "MK3Mag"};

static const int MaxTickerEvents = 5;

static const int MaxAnalog    = 32;
static const int MaxPlot      = 50000;

static const int MaxNaviPos   = 2000;

struct sMotor
{
    int Speed[12];
};

struct sMode
{
    int ID;
    int VERSION_MAJOR;
    int VERSION_MINOR;
    int VERSION_PATCH;
    int VERSION_SERIAL_MAJOR;
    int VERSION_SERIAL_MINOR;
    QString Hardware;
    QString Version;
};

struct sRxData
{
    char *Input;
    QString String;
    int Decode[150];
    int DecLen;
};

struct sGPS_Pos
{
    long Longitude;
    long Latitude;
    long Altitude;
};

struct sNaviData
{
    sGPS_Pos Current;
    sGPS_Pos Target;
    sGPS_Pos Home;


    long Longitude;
    long Latitude;
    long Altitude;
};

struct sNaviString
{
    QString Longitude;
    QString Latitude;
    QString Altitude;
};

struct sWayPoint
{
    double Longitude;
    double Latitude;
    double Altitude;
    int Time;
};

#endif
