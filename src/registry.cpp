#include "stdafx.h"


#pragma warning(disable: 4996)

//-----------------------------------------------------------------------------
//
//	Registry Information
//
//-----------------------------------------------------------------------------

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Stream,           // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins  =
{
    L"in",                   // Pin string name
    FALSE,                      // Is it rendered
    FALSE,                      // Is it an output
    FALSE,                      // Allowed zero pins
    FALSE,                      // Allowed many
    &CLSID_NULL,                // Connects to filter
    L"outpin",                  // Connects to pin
    1,                          // Number of pins types
    &sudPinTypes				// Pin information
};

const AMOVIESETUP_FILTER sudSocketSend =
{
    &CLSID_SocketSendFilter,     // Filter CLSID
    L"FLV Socket Writer",         // String name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};



// List of class IDs and creator functions for class factory

CFactoryTemplate g_Templates[]=	{
	{ L"FLV Socket Writer", &CLSID_SocketSendFilter, CSocketSendFilter::CreateInstance, NULL, &sudSocketSend }
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//-----------------------------------------------------------------------------
//
//	DLL Entry Points
//
//-----------------------------------------------------------------------------

STDAPI DllRegisterServer() 
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}
