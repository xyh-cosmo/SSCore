// SSSerial.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// This class contains routines for low-level serial (RS-232) communication
// on MacOS, Windows, and Linux. Currently not supported on iOS or Android.

#include "SSSerial.hpp"

#ifdef _WINDOWS

// SSSerial::listPorts() obtains a list of names of available COM or LPT ports.
// Vectors of found port name and DOS path strings are returned in (names) and (paths)
// The function returns the number of ports actually detected.
// Based on sample code by P.J. Naughter (pjn@indigo.ie).

int SSSerial::listPorts ( vector<string> &names, vector<string> &paths )
{
    const char *pszPortType = "COM"; // or "LPT" to list parallel ports
	int		    nNumPorts = 0;

    // Use the QueryDosDevice API to look for all devices of the form COMx.
	// This is a better solution as it means that no ports have to be opened at all.

    TCHAR szDevices[65535] = { 0 }, *pszCurrentDevice = NULL;
	DWORD i, dwChars = QueryDosDevice ( NULL, szDevices, 65535 );

	for ( i = 0; i < dwChars && szDevices[i] != '\0'; i++ )
	{
		// Get the current device name.

		pszCurrentDevice = &szDevices[i];

		// If it starts with "COM" or "LPT" then add it to the array which will be returned

		if ( strncmp ( pszCurrentDevice, pszPortType, 3 ) == 0 )
        {
			names.push_back ( string ( pszCurrentDevice ) );
            paths.push_back ( "\\\\.\\" + string ( pszCurrentDevice ) );
            nNumPorts++;
        }

		// Go to next NULL character

		while ( szDevices[i] != '\0' )
			i++;
	}

	return nNumPorts;
}

// SSSerial::openPort() opens a serial port with the specified path.
// Returns true if successful or false on failure.

bool SSSerial::openPort ( const string &path )
{
	char	szPort[16] = { 0 };
	HANDLE	hCom;

	// First try to open the serial port with a name such as "\\.\COM1", etc.
	// On failure, try to open just "COM1".

	strcpy_s ( szPort, path.c_str() );
	hCom = CreateFile ( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	if ( hCom == INVALID_HANDLE_VALUE )
	{
		strcpy_s ( szPort, path.c_str() + 4 );
		hCom = CreateFile ( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	}

	if ( hCom == INVALID_HANDLE_VALUE )
		return false;

	if ( SetupComm ( hCom, 1024, 1024 ) == FALSE )
	{
		CloseHandle ( hCom );
		return false;
	}

    _port = hCom;
	return true;
}

// SSSerial::portOpen() returns true if this port is currently open,
// or false if the port is closed (or other error).

bool SSSerial::portOpen ( void )
{
    return _port != kError ? true : false;
}

// SSSerial::closePort()

bool SSSerial::closePort ( void )
{
	return ( CloseHandle ( _port ) );
}

// SSSerial::readPort()

int SSSerial::readPort ( void *lpvBuffer, size_t lNumBytes )
{
	unsigned long dwNumBytesRead = 0;

    if ( ReadFile ( _port, lpvBuffer, lNumBytes, &dwNumBytesRead, NULL ) )
        return dwNumBytesRead;
    else
        return -1;
}

// SSSerial::writePort()

int SSSerial::writePort ( void *lpvBuffer, size_t lNumBytes )
{
	unsigned long	dwNumBytesWritten = 0;

    if ( WriteFile ( _port, lpvBuffer, lNumBytes, &dwNumBytesWritten, NULL ) )
        return dwNumBytesWritten;
    else
        return -1;
}

// SSSerial::setPortConfig()

bool SSSerial::setPortConfig ( Baud eBaudRate, Parity eParity, DataBits eDataBits, StopBits eStopBits )
{
	DCB		dcb;

	if ( GetCommState ( _port, &dcb ) == FALSE )
		return false;

	dcb.BaudRate = eBaudRate;

	if ( eParity == Parity::kEven )
		dcb.Parity = EVENPARITY;
	else if ( eParity == Parity::kOdd )
		dcb.Parity = ODDPARITY;
	else if ( eParity == Parity::kNone )
		dcb.Parity = NOPARITY;

	dcb.ByteSize = eDataBits;

	if ( eStopBits == StopBits::k1 )
		dcb.StopBits = ONESTOPBIT;
	else if ( eStopBits == StopBits::k15 )
		dcb.StopBits = ONE5STOPBITS;
	else if ( eStopBits == StopBits::k2 )
		dcb.StopBits = TWOSTOPBITS;

	if ( SetCommState ( _port, &dcb ) == FALSE )
		return false;

	return true;
}

// SSSerial::getPortConfig()

bool SSSerial::getPortConfig ( Baud &peBaudRate, Parity &peParity, DataBits &peDataBits, StopBits &peStopBits )
{
	DCB	dcb;

    if ( GetCommState ( _port, &dcb ) < 0 )
    	return false;

	if ( dcb.BaudRate == 300 )
        peBaudRate = Baud::k300;
	else if ( dcb.BaudRate == 600 )
        peBaudRate = Baud::k600;
	else if ( dcb.BaudRate == 1200 )
        peBaudRate = Baud::k1200;
	else if ( dcb.BaudRate == 2400 )
        peBaudRate = Baud::k2400;
	else if ( dcb.BaudRate == 4800 )
        peBaudRate = Baud::k4800;
	else if ( dcb.BaudRate == 9600 )
        peBaudRate = Baud::k9600;
	else if ( dcb.BaudRate == 14400 )
        peBaudRate = Baud::k14400;
	else if ( dcb.BaudRate == 19200 )
        peBaudRate = Baud::k19200;
	else if ( dcb.BaudRate == 38400 )
        peBaudRate = Baud::k38400;
	else if ( dcb.BaudRate == 57600 )
        peBaudRate = Baud::k57600;
	else if ( dcb.BaudRate == 115200 )
        peBaudRate = Baud::k115200;
	else if ( dcb.BaudRate == 230400 )
        peBaudRate = Baud::k230400;
	else if ( dcb.BaudRate == 460800 )
        peBaudRate = Baud::k460800;
	else if ( dcb.BaudRate == 921600 )
        peBaudRate = Baud::k921600;

	if ( dcb.Parity == EVENPARITY )
		peParity = Parity::kEven;
	else if ( dcb.Parity == ODDPARITY )
		peParity = Parity::kOdd;
	else if ( dcb.Parity == NOPARITY )
		peParity = Parity::kNone;

	if ( dcb.ByteSize == 5 )
   		peDataBits = DataBits::k5;
    else if ( dcb.ByteSize == 6 )
   		peDataBits = DataBits::k6;
    else if ( dcb.ByteSize == 7 )
   		peDataBits = DataBits::k7;
    else if ( dcb.ByteSize == 8 )
   		peDataBits = DataBits::k8;

	if ( dcb.StopBits == ONESTOPBIT )
		peStopBits = StopBits::k1;
	else if (dcb.StopBits == ONE5STOPBITS)
		peStopBits = StopBits::k15;
	else if (dcb.StopBits == TWOSTOPBITS)
		peStopBits = StopBits::k2;

	return true;
}

// SSSerial::inputBytes

int SSSerial::inputBytes ( void )
{
	COMSTAT			comstat;
	unsigned long	dwError;

	if ( ClearCommError ( _port, &dwError, &comstat ) == FALSE )
	 	return -1;

	return comstat.cbInQue;
}

// SSSerial::outputBytes()

int SSSerial::outputBytes ( void )
{
	COMSTAT			comstat;
	unsigned long	dwError;

	if ( ClearCommError ( _port, &dwError, &comstat ) == FALSE )
	 	return -1;

	return comstat.cbOutQue;
}

#else

#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#if defined (__APPLE__)
#include <TargetConditionals.h>
#endif

#if TARGET_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#endif

#if TARGET_OS_OSX

// Finds all available serial ports using IOKit, and returns
// the number of resources available.  Based on Apple Developer sample code.
// (names): vector of strings which will recieve the "friendly names" of the serial ports.
// (paths): vector of strings which will recieve the BSD paths to the serial port device "files".
// The function returns the number of serial ports actually found.

int SSSerial::listPorts ( vector<string> &names, vector<string> &paths )
{
    int                     n;
    kern_return_t           kernResult;
    mach_port_t             masterPort;
    CFMutableDictionaryRef  classesToMatch;
    io_iterator_t           SerialPortOSXIterator;
    io_object_t             serialService;

    // Get the mach port used to initiate communication with the IOKit.
    
    kernResult = IOMasterPort ( MACH_PORT_NULL, &masterPort ) ;
    if ( kernResult != KERN_SUCCESS )
        return ( 0 );

    // Create a matching dictionary that specifies an IOService class match.
    // Serial devices are instances of class IOSerialBSDClient.
    
    classesToMatch = IOServiceMatching ( kIOSerialBSDServiceValue );
    if ( classesToMatch == NULL )
        return ( 0 );
     
    // Set the value of the key in the dictionary. Each serial device object has a property
    // with key kIOSerialBSDTypeKey and a value that is one of kIOSerialBSDAllTypes,
    // kIOSerialBSDModemType, or kIOSerialBSDRS232Type.
          
    CFDictionarySetValue ( classesToMatch, CFSTR ( kIOSerialBSDTypeKey ), CFSTR ( kIOSerialBSDAllTypes ) );

    // Look up registered IOService objects that match a matching dictionary.
    // This is the preferred method of finding IOService objects currently registered by IOKit.
         
    kernResult = IOServiceGetMatchingServices ( masterPort, classesToMatch, &SerialPortOSXIterator );
    if ( kernResult != KERN_SUCCESS )
        return ( 0 );
    
    // Given an iterator across the serial devices returned, get their names and BSD paths.
    
    n = 0;
    while ( ( serialService = IOIteratorNext ( SerialPortOSXIterator ) ) )
    {
        CFStringRef serialNameAsCFString;
        CFStringRef bsdPathAsCFString;

        // Create a instantaneous snapshot of a registry entry property.
        
        serialNameAsCFString = (CFStringRef) IORegistryEntryCreateCFProperty ( serialService, CFSTR ( kIOTTYDeviceKey ), kCFAllocatorDefault, 0 );
        if ( serialNameAsCFString )
        {
            char name[256] = { 0 };
            CFStringGetCString ( serialNameAsCFString, name, 256, kCFStringEncodingASCII );
            CFRelease ( serialNameAsCFString );
            names.push_back ( string ( name ) );
        }
        
        bsdPathAsCFString = (CFStringRef) IORegistryEntryCreateCFProperty ( serialService, CFSTR ( kIOCalloutDeviceKey ), kCFAllocatorDefault, 0 );
        if ( bsdPathAsCFString )
        {
            char path[256] = { 0 };
            CFStringGetCString ( bsdPathAsCFString, path, 256, kCFStringEncodingASCII );
            CFRelease ( bsdPathAsCFString );
            paths.push_back ( string ( path ) );
        }
        
        n++;
    }
    
    // Release the object handle previously returned by IOKitLib.
    // Return the number of serial devices found.
    
    IOObjectRelease (SerialPortOSXIterator);
    return ( n );
}

#else   // Generic Linux implementation

#include <stdlib.h>
#include <dirent.h>

int SSSerial::listPorts ( vector<string> &names, vector<string> &paths )
{
    int n, nPorts = 0;
    struct dirent **namelist = NULL;
    const char *sysdir = "/dev/";
    char devicedir[256] = { 0 };
    
    // Scan through /dev and look for all tty devices

    n = scandir ( sysdir, &namelist, NULL, NULL );
    if ( n < 0 ) {
        return ( 0 );
    }

    for ( int i = 0; i < n; i++ )
    {
        if ( strncmp ( namelist[i]->d_name, "tty", 3 ) == 0 )
        {
            // Construct absolute file path
            
            strncpy ( devicedir, sysdir, sizeof ( devicedir ) );
            strncat ( devicedir, namelist[i]->d_name, sizeof ( devicedir ) - strlen ( devicedir ) - 1 );
            
            // Open the device
            
            int fd = open ( devicedir, O_RDWR | O_NOCTTY | O_NONBLOCK );
            if ( fd < 0 )
                continue;

            // Attempt to get get modem line state.  If we success, it's a real serial port.
            
            int bits = 0;
            if ( ioctl ( fd, TIOCMGET, &bits ) >= 0 )
            {
                names.push_back ( string ( namelist[i]->d_name ) );
                paths.push_back ( devicedir );
                nPorts++;
            }
            
            close ( fd );
        }
        
        free ( namelist[i] );
    }
    
    free ( namelist );
    return ( nPorts );
}

#endif // TARGET_OS_OSX

// SSSerial::openPort() opens a serial port with the specifid path.
// Returns true if successful or false on failure.

bool SSSerial::openPort ( const string &path )
{
    int            fileDescriptor = -1;
    struct termios options;
    
    fileDescriptor = open ( path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK );
    
    // Turn of all status flags.
    
    if ( fcntl ( fileDescriptor, F_SETFL, 0 ) == -1 )
        goto error;
    
    // Get the current serial options into a termios stucture.
    
    if ( tcgetattr ( fileDescriptor, &options ) == -1 )
        goto error;

    // Set raw input and output so the serial driver does not perform
    // any processing on the data that is sent to it.  Ignore modem
    // status lines; enable reciever; hang up on close; set 8 data bits,
    // 1 stop bit, and no parity; set one second timeout.  These options
    // are documented in the man page for termios!
         
    cfmakeraw ( &options );
    
    options.c_cflag = HUPCL | CLOCAL | CREAD | CS8;
    options.c_cc[ VMIN ] = 0;
    options.c_cc[ VTIME ] = 10;

    cfsetspeed ( &options, B9600 );
    
    // Set the options.  Generate an error code on failure.
    
    if ( tcsetattr ( fileDescriptor, TCSANOW, &options ) == -1 )
        goto error;

    // If we succeeded, save the file descriptor to the opened serial port.
    
    _port = fileDescriptor;
    return true;
    
    // On failure, close the serial device driver file descriptor
    // (if it's been opened) and return an error code.
         
error:

    if ( fileDescriptor != -1 )
        close ( fileDescriptor );
    return false;
}

// SSSerial::portOpen() returns true if this port is currently open,
// or false if the port is closed (or other error).

bool SSSerial::portOpen ( void )
{
    return _port != kError ? true : false;
}

// SSSerial::closePort() closes this serial port.
// Returns true if successful or false on failure.

bool SSSerial::closePort ( void )
{
    bool success = close ( _port ) == 0;
    _port = kError;
    return success;
}

// SSSerial::readPort() reads up to (size) bytes from serial port into (data) buffer.
// Returns number of bytes read from serial port, or -1 on error.

int SSSerial::readPort ( void *data, size_t size )
{
    return (int) read ( _port, data, size );
}

// SSSerial::writePort() writes up to (size) bytes from (data) buffer into serial port.
// Returns number of bytes written to serial port, or -1 on error.

int SSSerial::writePort ( void *data, size_t size )
{
    return (int) write ( _port, data, size );
}

// SSSerial::inputBytes() returns the number of bytes available to read from the serial port,
// or -1 on error.

int SSSerial::inputBytes ( void )
{
    int bytes = 0;
    
    if ( ioctl ( _port, FIONREAD, &bytes ) == -1 )
        return -1;
    
    return bytes;
}

// SSSerial::outputBytes() returns the number of bytes remaining to be written to serial port,
// or -1 on error.

int SSSerial::outputBytes ( void )
{
    int bytes = 0;
    
    if ( ioctl ( _port, TIOCOUTQ, &bytes ) == -1 )
        return -1;
    
    return bytes;
}

// SSSerial::getPortConfig() obtains this serial port's baud rate, parity, data bits, and stop bits.
// Returns true if successful or false on failure (e.g. if the port is not opened.)

bool SSSerial::getPortConfig ( Baud &baud, Parity &parity, DataBits &dataBits, StopBits &stopBits )
{
    struct termios options;
    
    // Get the current serial port control options into a termios struct.
    // Return an error code on failure.
         
    if ( tcgetattr ( _port, &options ) == -1 )
        return false;
    
    // Get the baud rate from the termios structure. Note that we're only
    // using the input speed here; we'll assume this is the same as the output.
    
    speed_t speed = cfgetispeed ( &options );
    if ( speed == B300 )
        baud = Baud::k300;
    else if ( speed == B600 )
        baud = Baud::k600;
    else if ( speed == B1200 )
        baud = Baud::k1200;
    else if ( speed == B2400 )
        baud = Baud::k2400;
    else if ( speed == B4800 )
        baud = Baud::k4800;
    else if ( speed == B9600 )
        baud = Baud::k9600;
    else if ( speed == B14400 )
        baud = Baud::k14400;
    else if ( speed == B19200 )
        baud = Baud::k19200;
    else if ( speed == B38400 )
        baud = Baud::k38400;
    else if ( speed == B57600 )
        baud = Baud::k57600;
    else if ( speed == B115200 )
        baud = Baud::k115200;
    else if ( speed == B230400 )
        baud = Baud::k230400;
    else if ( speed == 460800 )
        baud = Baud::k460800;
    else if ( speed == 921600 )
        baud = Baud::k921600;

    // Determine the parity options from the parity bit flags in the termios structure.
    
    if ( options.c_cflag & PARENB )
    {
        if ( options.c_cflag & PARODD )
            parity = Parity::kOdd;
        else
            parity = Parity::kEven;
    }
    else
    {
        parity = Parity::kNone;
    }
        
    // Same for the data bit flags.
    
    if ( ( options.c_cflag & CSIZE ) == CS5 )
        dataBits = DataBits::k5;
    else if ( ( options.c_cflag & CSIZE ) == CS6 )
        dataBits = DataBits::k6;
    else if ( ( options.c_cflag & CSIZE ) == CS7 )
        dataBits = DataBits::k7;
    else if ( ( options.c_cflag & CSIZE ) == CS8 )
        dataBits = DataBits::k8;
    
    // Now for the stop bit flags. Note that the 1.5 stop bit combination
    // is not supported under MacOS.
    
    if ( options.c_cflag & CSTOPB )
        stopBits = StopBits::k2;
    else
        stopBits = StopBits::k1;
            
    // Return a successful result code.
    
    return true;
}

// SSSerial::setPortConfig() changes this serial port's baud rate, parity, data bits, and stop bits.
// Returns true if successful or false on failure (e.g. if the port is not opened.)

bool SSSerial::setPortConfig ( Baud baud, Parity parity, DataBits dataBits, StopBits stopBits )
{
    struct termios options;
    
    // Get the current serial port control options into a termios struct.
    // Return an error code on failure.
         
    if ( tcgetattr ( _port, &options ) == -1 )
        return false;
    
    // Set the baud rate.
       
    cfsetspeed ( &options, baud );
    
    // Set the partity bit flags in the termios structure by first turning them
    // all off, then turning on those which match the parity specified.
         
    if ( parity == Parity::kNone )
        options.c_cflag = ( options.c_cflag & ~( PARENB | PARODD ) );
    else if ( parity == Parity::kEven )
        options.c_cflag = ( options.c_cflag & ~( PARENB | PARODD ) ) | PARENB;
    else if ( parity == Parity::kOdd )
        options.c_cflag = ( options.c_cflag & ~( PARENB | PARODD ) ) | PARENB | PARODD;
        
    // Same for the data bit flags.
    
    if ( dataBits == DataBits::k5 )
        options.c_cflag = ( options.c_cflag & ~CSIZE ) | CS5;
    else if ( dataBits == DataBits::k6 )
        options.c_cflag = ( options.c_cflag & ~CSIZE ) | CS6;
    else if ( dataBits == DataBits::k7 )
        options.c_cflag = ( options.c_cflag & ~CSIZE ) | CS7;
    else if ( dataBits == DataBits::k8 )
        options.c_cflag = ( options.c_cflag & ~CSIZE ) | CS8;
    
    // Same for the stop bit flags. Note that 1.5 stop bits
    // is not supported under MacOS.
    
    if ( stopBits == StopBits::k1 )
        options.c_cflag = ( options.c_cflag & ~CSTOPB );
    else if ( stopBits == StopBits::k2  )
        options.c_cflag = ( options.c_cflag & ~CSTOPB ) | CSTOPB;

    // Now pass the changed termios options back to the serial driver.
    // Return an error code on failure.
    
    if ( tcsetattr ( _port, TCSANOW, &options ) == -1 )
        return false;
    
    // Return a successful result code if we succeeded.
    
    return true;
}

#endif // ! _WINDOWS
