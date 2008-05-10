/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CDeviceGarmin.h"
#include "CMainWindow.h"
#include "CWptDB.h"
#include "CWpt.h"

#undef IDEVICE_H
#include <garmin/IDevice.h>

#include <QtGui>

#define XSTR(x) STR(x)
#define STR(x) #x

struct garmin_icon_t
{
    uint16_t id;
    const char * name;
};
garmin_icon_t GarminIcons[];


/**
  @param progress the progress as integer from 0..100, if -1 no progress bar needed.
  @param ok if this pointer is 0 no ok button needed, if non zero set to 1 if ok button pressed
  @param cancel if this pointer is 0 no cancel button needed, if non zero set to 1 if cancel button pressed
  @param title dialog title as C string
  @param msg dialog message C string to display
  @param self void pointer as provided while registering the callback
*/
void GUICallback(int progress, int * ok, int * cancel, const char * title, const char * msg, void * self)
{
    CDeviceGarmin * parent = static_cast<CDeviceGarmin *>(self);
    CDeviceGarmin::dlgdata_t& dd = parent->dlgData;

    if(progress != -1) {
        quint32 togo, hour, min, sec;
        QString message;

        if(dd.dlgProgress == 0) {
            dd.canceled     = false;
            dd.dlgProgress  = new QProgressDialog(QString(title),0,0,100,theMainWindow, Qt::WindowStaysOnTopHint);
            dd.timeProgress.start();
            if(cancel) {
                QPushButton * butCancel = new QPushButton(QObject::tr("Cancel"),dd.dlgProgress);
                parent->connect(butCancel, SIGNAL(clicked()), parent, SLOT(slotCancel()));
                dd.dlgProgress->setCancelButton(butCancel);
            }
        }

        if(title) dd.dlgProgress->setWindowTitle(QString(title));

        togo = (quint32)((100.0 * (double)dd.timeProgress.elapsed() / (double)progress) + 0.5);
        togo = (quint32)((double)(togo - dd.timeProgress.elapsed()) / 1000.0 + 0.5);

        hour = (togo / 3600);
        min  = (togo - hour * 3600) / 60;
        sec  = (togo - hour * 3600 - min * 60);

        message.sprintf(QObject::tr("\n\nEstimated finish: %02i:%02i:%02i [hh:mm:ss]").toUtf8(),hour,min,sec);

        dd.dlgProgress->setLabelText(QString(msg) + message);
        dd.dlgProgress->setValue(progress);

        if(progress == 100 && dd.dlgProgress) {
            delete dd.dlgProgress;
            dd.dlgProgress = 0;
        }

        if(cancel) {
            *cancel = dd.canceled;
        }

        qApp->processEvents();

    }
    else {
        if(ok && cancel) {
            QMessageBox::StandardButtons res = QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel);
            *ok     = res == QMessageBox::Ok;
            *cancel = res == QMessageBox::Cancel;
        }
        else if(ok && !cancel) {
            QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Ok,QMessageBox::Ok);
            *ok     = true;
        }
        else if(!ok && cancel) {
            QMessageBox::question(theMainWindow,QString(title),QString(msg),QMessageBox::Cancel,QMessageBox::Cancel);
            *cancel     = true;
        }
        else if(!ok && !cancel) {
            //kiozen - that doesn't work nicely
            //             QMessageBox * dlg = new QMessageBox(&parent->main);
            //             dlg->setWindowTitle(QString(title));
            //             dlg->setText(QString(msg));
            //             dlg->setStandardButtons(QMessageBox::NoButton);
            //             dlg->setIcon(QMessageBox::Information);
            //             dlg->show();
            //             qApp->processEvents(QEventLoop::AllEvents, 1000);
            //             sleep(3); // sleep for 3 seconds
            //             delete dlg;
        }
    }
}


CDeviceGarmin::CDeviceGarmin(const QString& devkey, const QString& port, QObject * parent)
    : IDevice(devkey, parent)
    , port(port)
{
    qDebug() << "CDeviceGarmin::CDeviceGarmin()";

}

CDeviceGarmin::~CDeviceGarmin()
{
    qDebug() << "~CDeviceGarmin::CDeviceGarmin()";
}

Garmin::IDevice * CDeviceGarmin::getDevice()
{
    Garmin::IDevice * (*func)(const char*) = 0;
    Garmin::IDevice * dev = 0;

    QString libname     = QString("%1/lib%2" XSTR(SOEXT)).arg(XSTR(QL_LIBDIR)).arg(devkey);
    QString funcname    = QString("init%1").arg(devkey);

    func = (Garmin::IDevice * (*)(const char*))QLibrary::resolve(libname,funcname.toAscii());

    if(func == 0) {
        QMessageBox::warning(0,tr("Error ..."),tr("Failed to load driver."),QMessageBox::Ok,QMessageBox::NoButton);
        return 0;
    }

    dev = func(INTERFACE_VERSION);
    if(dev == 0) {
        QMessageBox::warning(0,tr("Error ..."),tr("Driver version mismatch."),QMessageBox::Ok,QMessageBox::NoButton);
        func = 0;
    }

    if(dev){
        dev->setPort(port.toLatin1());
        dev->setGuiCallback(GUICallback,this);
    }


    return dev;
}

void CDeviceGarmin::uploadWpts(const QList<CWpt*>& wpts)
{
    qDebug() << "CDeviceGarmin::uploadWpts()";
    Garmin::IDevice * dev = getDevice();
    if(dev == 0) return;

}

void CDeviceGarmin::downloadWpts(QList<CWpt*>& wpts)
{
    qDebug() << "CDeviceGarmin::downloadWpts()";
    Garmin::IDevice * dev = getDevice();
    if(dev == 0) return;

    std::list<Garmin::Wpt_t> garwpts;
    try{
        dev->downloadWaypoints(garwpts);
    }
    catch(int e) {
        QMessageBox::warning(0,tr("Device Link Error"),dev->getLastError().c_str(),QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    std::list<Garmin::Wpt_t>::const_iterator garwpt = garwpts.begin();
    while(garwpt != garwpts.end()) {
        CWpt * wpt = new CWpt(&CWptDB::self());

        wpt->name       = garwpt->ident.c_str();
        wpt->comment    = garwpt->comment.c_str();
        wpt->lon        = garwpt->lon;
        wpt->lat        = garwpt->lat;
        wpt->ele        = garwpt->alt;
        wpt->prx        = garwpt->dist;

//         uint16_t smbl;
//         double   lat;
//         double   lon;
//         float    alt;
//         float    dpth;
//         float    dist;
//         uint32_t time;
//         std::string ident;
//         std::string comment;

        wpts << wpt;
        ++garwpt;
    }
}


garmin_icon_t GarminIcons[] =
{
    /*    mps    pcx    desc */
    {  16384, "Airport" },
    {   8204, "Amusement Park" },
    {    169, "Ball Park" },
    {      6, "Bank" },
    {     13, "Bar" },
    {   8244, "Beach" },
    {      1, "Bell" },
    {    150, "Boat Ramp" },
    {   8205, "Bowling" },
    {   8233, "Bridge" },
    {   8234, "Building" },
    {    151, "Campground" },
    {    170, "Car" },
    {   8206, "Car Rental" },
    {   8207, "Car Repair" },
    {   8235, "Cemetery" },
    {   8236, "Church" },
    {    179, "Circle with X" },
    {   8203, "City (Capitol)" },
    {   8200, "City (Large)" },
    {   8199, "City (Medium)" },
    {   8198, "City (Small)" },
    {   8198, "Small City" },
    {   8237, "Civil" },
    {   8262, "Contact, Afro" },
    {   8272, "Contact, Alien" },
    {   8258, "Contact, Ball Cap" },
    {   8259, "Contact, Big Ears" },
    {   8271, "Contact, Biker" },
    {   8273, "Contact, Bug" },
    {   8274, "Contact, Cat" },
    {   8275, "Contact, Dog" },
    {   8263, "Contact, Dreadlocks" },
    {   8264, "Contact, Female1" },
    {   8265, "Contact, Female2" },
    {   8266, "Contact, Female3" },
    {   8261, "Contact, Goatee" },
    {   8268, "Contact, Kung-Fu" },
    {   8276, "Contact, Pig" },
    {   8270, "Contact, Pirate" },
    {   8267, "Contact, Ranger" },
    {   8257, "Contact, Smiley" },
    {   8260, "Contact, Spike" },
    {   8269, "Contact, Sumo" },
    {    165, "Controlled Area" },
    {   8220, "Convenience Store" },
    {   8238, "Crossing" },
    {    164, "Dam" },
    {    166, "Danger Area" },
    {   8218, "Department Store" },
    {      4, "Diver Down Flag 1" },
    {      5, "Diver Down Flag 2" },
    {    154, "Drinking Water" },
    {    177, "Exit" },
    {   8208, "Fast Food" },
    {      7, "Fishing Area" },
    {   8209, "Fitness Center" },
    {    178, "Flag" },
    {   8245, "Forest" },
    {      8, "Gas Station" },
    {   8255, "Geocache" },
    {   8256, "Geocache Found" },
    {   8239, "Ghost Town" },
    {  16393, "Glider Area" },
    {   8197, "Golf Course" },
    {      2, "Green Diamond" },
    {     15, "Green Square" },
    {  16388, "Heliport" },
    {      9, "Horn" },
    {    171, "Hunting Area" },
    {    157, "Information" },
    {   8240, "Levee" },
    {     12, "Light" },
    {   8221, "Live Theater" },
    {    173, "Lodging" },
    {    173, "Hotel" },
    {     21, "Man Overboard" },
    {      0, "Marina" },
    {    156, "Medical Facility" },
    {   8195, "Mile Marker" },
    {   8241, "Military" },
    {    174, "Mine" },
    {   8210, "Movie Theater" },
    {   8211, "Museum" },
    {     22, "Navaid, Amber" },
    {     23, "Navaid, Black" },
    {     24, "Navaid, Blue" },
    {     25, "Navaid, Green" },
    {     26, "Navaid, Green/Red" },
    {     27, "Navaid, Green/White" },
    {     28, "Navaid, Orange" },
    {     29, "Navaid, Red" },
    {     30, "Navaid, Red/Green" },
    {     31, "Navaid, Red/White" },
    {     32, "Navaid, Violet" },
    {     33, "Navaid, White" },
    {     34, "Navaid, White/Green" },
    {     35, "Navaid, White/Red" },
    {   8242, "Oil Field" },
    {  16395, "Parachute Area" },
    {    159, "Park" },
    {    158, "Parking Area" },
    {   8212, "Pharmacy" },
    {    160, "Picnic Area" },
    {   8213, "Pizza" },
    {   8214, "Post Office" },
    {  16389, "Private Field" },
    {     37, "Radio Beacon" },
    {      3, "Red Diamond" },
    {     16, "Red Square" },
    {     10, "Residence" },
    {     10, "House" },
    {     11, "Restaurant" },
    {    167, "Restricted Area" },
    {    152, "Restroom" },
    {   8215, "RV Park" },
    {   8226, "Scales" },
    {    161, "Scenic Area" },
    {   8216, "School" },
    {  16402, "Seaplane Base" },
    {     19, "Shipwreck" },
    {    172, "Shopping Center" },
    {  16392, "Short Tower" },
    {    153, "Shower" },
    {    162, "Skiing Area" },
    {     14, "Skull and Crossbones" },
    {  16390, "Soft Field" },
    {   8217, "Stadium" },
    {   8246, "Summit" },
    {    163, "Swimming Area" },
    {  16391, "Tall Tower" },
    {    155, "Telephone" },
    {   8227, "Toll Booth" },
    {   8196, "TracBack Point" },
    {    175, "Trail Head" },
    {    176, "Truck Stop" },
    {   8243, "Tunnel" },
    {  16394, "Ultralight Area" },
    {   8282, "Water Hydrant" },
    {     18, "Waypoint" },
    {     17, "White Buoy" },
    {     36, "White Dot" },
    {   8219, "Zoo" },

    /* Custom icons.   The spec reserves 7680-8191 for the custom
     * icons on the C units, Quest, 27xx, 276, 296,  and other units.
     * Note that firmware problems on the earlier unit result in these
     * being mangled, so be sure you're on a version from at least
     * late 2005.
     * {    -2,  7680, "Custom 0" },
     * ....
     * {    -2,  8192, "Custom 511" },
     */
    {   7680, "Custom 1" },
    {   7681, "Custom 2" },
    {   7682, "Custom 3" },
    {   7683, "Custom 4" },
    {   7684, "Custom 5" },
    {   7685, "Custom 6" },
    {   7686, "Custom 7" },
    {   7687, "Custom 8" },
    {   7688, "Custom 9" },
    {   7689, "Custom 10" },
    {   7690, "Custom 11" },
    {   7691, "Custom 12" },
    {   7692, "Custom 13" },
    {   7693, "Custom 14" },
    {   7694, "Custom 15" },
    {   7695, "Custom 16" },
    {   7696, "Custom 17" },
    {   7697, "Custom 18" },
    {   7698, "Custom 19" },
    {   7699, "Custom 20" },
    {   7700, "Custom 21" },
    {   7701, "Custom 22" },
    {   7702, "Custom 23" },
    {   7703, "Custom 24" },

    {   8227, "Micro-Cache" },
    {    161, "Virtual cache" },
    {   8217, "Multi-Cache" },
    {    157, "Unknown Cache"    },
    {                            /* Icon for "Flag" */
        wptDefault,   178, "Locationless (Reverse) Cache"
    },
    {                            /* Icon for "Post Office" */
        wptDefault,  8214, "Post Office"
    },
    {                            /* Icon for "Event" */
        wptDefault,   160, "Event Cache"
    },
    {                            /* Icon for "Live Theatre" */
        wptDefault,  8221, "Webcam Cache"
    },

    /* MapSource V6.x */

    { ":/icons/wpt/flag_pin_red15x15.png",  8286, "Flag, Red" },
    { ":/icons/wpt/flag_pin_blue15x15.png",  8284, "Flag, Blue" },
    { ":/icons/wpt/flag_pin_green15x15.png",  8285, "Flag, Green" },
    { ":/icons/wpt/pin_red15x15.png",  8289, "Pin, Red" },
    { ":/icons/wpt/pin_blue15x15.png",  8287, "Pin, Blue" },
    { ":/icons/wpt/pin_green15x15.png",  8288, "Pin, Green" },
    { ":/icons/wpt/box_red15x15.png",  8292, "Block, Red" },
    { ":/icons/wpt/box_blue15x15.png",  8290, "Block, Blue" },
    { ":/icons/wpt/box_green15x15.png",  8291, "Block, Green" },
    { wptDefault,  8293, "Bike Trail" },
    { wptDefault,   181, "Fishing Hot Spot Facility" },
    { wptDefault,  8249, "Police Station"},
    { wptDefault,  8251, "Ski Resort" },
    { wptDefault,  8252, "Ice Skating" },
    { wptDefault,  8253, "Wrecker" },
    { wptDefault,   184, "Anchor Prohibited" },
    { wptDefault,   185, "Beacon" },
    { wptDefault,   186, "Coast Guard" },
    { wptDefault,   187, "Reef" },
    { wptDefault,   188, "Weed Bed" },
    { wptDefault,   189, "Dropoff" },
    { wptDefault,   190, "Dock" },

    /* New in Garmin protocol spec from June 2006.  Extracted from
     * spec and fed through some horrible awk to add ones we didn't
     * have before but normalized for consistency. */
    { wptDefault,  8359, "Asian Food" },
    { wptDefault,  8296, "Blue Circle" },
    { wptDefault,  8299, "Blue Diamond" },
    { wptDefault,  8317, "Blue Letter A" },
    { wptDefault,  8318, "Blue Letter B" },
    { ":/icons/wpt/letter_c_blue15x15.png",  8319, "Blue Letter C" },
    { wptDefault,  8320, "Blue Letter D" },
    { wptDefault,  8341, "Blue Number 0" },
    { wptDefault,  8342, "Blue Number 1" },
    { wptDefault,  8343, "Blue Number 2" },
    { wptDefault,  8344, "Blue Number 3" },
    { wptDefault,  8345, "Blue Number 4" },
    { wptDefault,  8346, "Blue Number 5" },
    { wptDefault,  8347, "Blue Number 6" },
    { wptDefault,  8348, "Blue Number 7" },
    { wptDefault,  8349, "Blue Number 8" },
    { wptDefault,  8350, "Blue Number 9" },
    { wptDefault,  8302, "Blue Oval" },
    { wptDefault,  8305, "Blue Rectangle" },
    { wptDefault,  8308, "Blue Square" },
    { wptDefault,  8351, "Blue Triangle" },
    { wptDefault,  8254, "Border Crossing (Port Of Entry)" },
    { wptDefault,   182, "Bottom Conditions" },
    { wptDefault,  8360, "Deli" },
    { wptDefault,  8228, "Elevation point" },
    { wptDefault,  8229, "Exit without services" },
    { wptDefault, 16398, "First approach fix" },
    { wptDefault,  8250, "Gambling/casino" },
    { wptDefault,  8232, "Geographic place name, land" },
    { wptDefault,  8230, "Geographic place name, Man-made" },
    { wptDefault,  8231, "Geographic place name, water" },
    { wptDefault,  8295, "Green circle" },
    { wptDefault,  8313, "Green Letter A" },
    { wptDefault,  8315, "Green Letter B" },
    { wptDefault,  8314, "Green Letter C" },
    { wptDefault,  8316, "Green Letter D" },
    { wptDefault,  8331, "Green Number 0" },
    { wptDefault,  8332, "Green Number 1" },
    { wptDefault,  8333, "Green Number 2" },
    { wptDefault,  8334, "Green Number 3" },
    { wptDefault,  8335, "Green Number 4" },
    { wptDefault,  8336, "Green Number 5" },
    { wptDefault,  8337, "Green Number 6" },
    { wptDefault,  8338, "Green Number 7" },
    { wptDefault,  8339, "Green Number 8" },
    { wptDefault,  8340, "Green Number 9" },
    { wptDefault,  8301, "Green Oval" },
    { wptDefault,  8304, "Green Rectangle" },
    { wptDefault,  8352, "Green Triangle" },
    { wptDefault, 16385, "Intersection" },
    { wptDefault,  8201, "Intl freeway hwy" },
    { wptDefault,  8202, "Intl national hwy" },
    { wptDefault,  8361, "Italian food" },
    { wptDefault,  8248, "Large exit without services" },
    { wptDefault,  8247, "Large Ramp intersection" },
    { wptDefault, 16399, "Localizer Outer Marker" },
    { wptDefault, 16400, "Missed approach point" },
    { wptDefault, 16386, "Non-directional beacon" },
    { wptDefault,   168, "Null" },
    { wptDefault,   180, "Open 24 Hours" },
    { wptDefault,  8222, "Ramp intersection" },
    { wptDefault,  8294, "Red circle" },
    { wptDefault,  8309, "Red Letter A" },
    { wptDefault,  8310, "Red Letter B" },
    { wptDefault,  8311, "Red Letter C" },
    { wptDefault,  8312, "Red Letter D" },
    { wptDefault,  8321, "Red Number 0" },
    { wptDefault,  8322, "Red Number 1" },
    { wptDefault,  8323, "Red Number 2" },
    { wptDefault,  8324, "Red Number 3" },
    { wptDefault,  8325, "Red Number 4" },
    { wptDefault,  8326, "Red Number 5" },
    { wptDefault,  8327, "Red Number 6" },
    { wptDefault,  8328, "Red Number 7" },
    { wptDefault,  8329, "Red Number 8" },
    { wptDefault,  8330, "Red Number 9" },
    { wptDefault,  8300, "Red Oval" },
    { wptDefault,  8303, "Red Rectangle" },
    { wptDefault,  8353, "Red Triangle" },
    { wptDefault,  8362, "Seafood" },
    { wptDefault,  8194, "State Hwy" },
    { wptDefault,  8363, "Steak" },
    { wptDefault,  8223, "Street Intersection" },
    { wptDefault, 16401, "TACAN" },
    { wptDefault,   183, "Tide/Current PRediction Station" },
    { wptDefault,   191, "U Marina" },
    { wptDefault,  8193, "US hwy" },
    { wptDefault,   193, "U stump" },
    { wptDefault, 16387, "VHF Omni-range" },
    { wptDefault, 16397, "VOR-DME" },
    { wptDefault, 16396, "VOR/TACAN" },
    { 0,    -1, 0 },
};
