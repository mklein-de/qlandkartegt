Conan 1.0.2 - Connection analyzer for Qt 4.6.0 - Qt 4.7.2
---------------------------------------------------------

    Conan is a C++ library that provides run-time introspection of object
    hierarchies, object inheritance, signal/slot connections, and signal
    emissions.

    Conan contains a single widget (conan::ConanWidget) that provides the
    following functionality:
    -object hierarchies can be added, discovered, browsed and exported to XML
    -signals/slots, including active connections, can be examined
    -any signal/slot connection can be disconnected
    -duplicate connections can be found
    -signal emissions can be monitored and logged
    See /doc/html/index.html for detailed information.

    Conan has been used to identify multiple duplicate connections in the
    source code of Qt!

    Contact - edekoning@gmail.com
    Website - http://sourceforge.net/projects/conanforqt
    License - GPL - source code -> see ./GPL.txt
              CCL - icons -> see ./icons/CCL.txt

    One note about the GPL license. You can use Conan in your proprietary
    program for analysis purposes. Just make sure that you do not distribute
    Conan as part of your proprietary product. I generally use Conan only in
    my debug builds, which are for internal use only. See
    http://www.gnu.org/licenses/gpl-faq.html for commonly-asked questions
    about the GPL.

    Conan was made using the following tools:
    -Silk icons made by Mark James available at FamFamFam
        http://www.famfamfam.com/lab/icons/silk/
    -Qt SDK Open Source Edition, available from Nokia
        http://qt.nokia.com/downloads/
    -Visual C++ 2008 Express Edition, available from Microsoft
        http://www.microsoft.com/express/vc/
    -Doxygen by Dimitri van Heesch, for source code documentation
        http://www.doxygen.org/
    -SourceForge, which hosts this project
        http://sourceforge.net/

    Other sites hosting Conan include:
    -Qt-Apps.org
        http://qt-apps.org/content/show.php/Conan+-+Connection+analyzer+for+Qt?content=108330
    -Ohloh.net
        http://www.ohloh.net/p/conanforqt
    -FamousWhy.com - Granted an Editor's Pick Award
        http://download.famouswhy.com/conan_for_qt/

    Other projects using Conan include:
    -Qtilities
        Building blocks for Qt applications, allowing rapid application development.
        http://www.qtilities.org/

    Feel free to contact me for suggestions, remarks, bugs, feature requests,
    or anything else Conan related.


Conan demo package
------------------

    The Conan demo package contains an application (./bin folder) that
    demonstrates the use of Conan. It shows the conan::ConanWidget and
    a console window that captures the output of the signal loggers.
    The conan::ConanWidget contains itself as root object, so it can be
    used to examine to internals of Conan.

    For more information about the use of the conan::ConanWidget see the
    generated documentation in ./doc/index.html.

    All sources of the demo application (not of Conan) are located in the
    ./src folder. This includes a Visual Studio 2008 project file (./win32
    folder) and a pro file (./ConanApp.pro) for use with QT Creator and qmake.

    The Conan Demo application has been tested on:
        -Windows XP SP3


Conan source package - installation
-----------------------------------

    Conan provides both a Visual Studio 2008 project file (./win32 folder)
    and a pro file (./conan.pro) for use with QT Creator and qmake. When using
    Visual Studio, make sure the following environment variable has been
    defined:
        QTDIR = the root dir of where QT has been installed to

    The following paths are used to find all Qt headers used by Conan:
        $(QTDIR)/include
        $(QTDIR)/include/Qt
        $(QTDIR)/include/QtCore

    All sources are located in ./src folder, including generated moc, ui and qrc
    files. With Visual Studio these files can be regenerated using the build
    rules as specified in the QTRules.rules file, located in the ./win32 folder.

    Note that Conan uses parts of the private non-documented Qt-API. This is
    needed to be able to retrieve all the connection information. As such there
    is no guarentee that Conan will continue to build after switching to a
    different (non-supported) Qt version.

    Use of the private Qt-API has been restricted to a single header:
        ./src/ConanWidget_p.h
    This header includes the private Qt headers:
        <private/qobject_p.h>
        <private/qmetaobject_p.h>
    Depending on the Qt version used, both files are located in:
        $(QTDIR)/include/Qt/private OR
        $(QTDIR)/include/QtCore/private

    If these files are not present, it means you are using a Qt distribution
    that does not contain the complete Qt source code. In that case, download
    and extract a Qt source only package, and add the following line to the
    ./conan.pro file:
        INCLUDEPATH += /<qt source location>/include/Qt OR
        INCLUDEPATH += /<qt source location>/include/QtCore
    or when using Visual Studio, open the Conan project properties and add the
    following path to 'C/C++ > General > Additional Include Directories':
        <qt source location>/include/Qt OR
        <qt source location>/include/QtCore

    When using Qt Creator make sure the 'Build Directory' is set to the Conan
    root folder. Otherwise uic, moc and rcc will output the wrong directory.


Using Conan
-----------

    After building Conan (see installation), link your project against the
    generated import library Conan(d).lib and make sure your application can
    find the Conan(d).dll. Furthermore, the ./include folder should be part of
    your include search path.

    Make sure QT_NO_DEBUG_OUTPUT is not defined as it will disable signal spy
    logging.

    To create and show the Conan widget in your application, all you need to do
    is add the following lines of code:

    #include <Conan.h>          // checks for proper qt version and includes
                                // the ConanWidget and AboutDialog

    //Q_INIT_RESOURCE (Conan);  // necessary on some platforms when Conan
                                // is build as a static library
    conan::ConanWidget widget;
    widget.AddRootObject (myMainWindow);    // optional
    widget.AddRootObject (someOtherObject); // optional
    widget.show ();

    For more information about the use of the Conan widget see the generated
    code documentation ./doc/html/index.html.

Copyright (C) 2008 - 2011 Elmar de Koning, All Rights Reserved
