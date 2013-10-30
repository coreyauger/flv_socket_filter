//-----------------------------------------------------------------------------
//
//	Author : Corey Auger
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

bool		_hadHeaders = false;
bool		canWrite = false;
char *		pFlvHeader;
long		flvHeaderSize = 0;
char *		pFlvMeta;
long		flvMetaSize = 0;
char *		pFlvAac1;
long		flvAacSize1 = 0;
int			grabVPacket = 0;
const int   NUM_VID_PACETS = 10;
char *		pFlvVid1[NUM_VID_PACETS];
long		flvVidSize1[NUM_VID_PACETS];

WSADATA _wsaData;
std::list<SOCKET> _remoteSocketList;
SOCKET _remoteSocket = INVALID_SOCKET;

#pragma warning(disable: 4018)		// signed/usigned mismatch
#pragma warning(disable: 4267)		// '=' : conversion from 'size_t' to '....'

const unsigned short _PORT = 22822;

// Quick and dirty debug..
void Log( const char* msg ){
#ifdef _DEBUG
	//static FILE *f = fopen("C:\\_sock.txt","a");
	//fprintf(f,"%s",msg);
	//fflush(f);
#endif
}

#define PRINTERROR(s)	\
		fprintf(stderr,"\n%: %d\n", s, WSAGetLastError())


CSocketSendFilter::CSocketSendFilter(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseFilter(NAME("SocketSend"), pUnk, (CCritSec *) this, CLSID_SocketSendFilter, phr)
{
    ASSERT(phr);	
	m_hostPort = _PORT;
    m_pInputPin = NULL;

    // Create the single input pin
    m_pInputPin = new CSocketSendInputPin(this,phr,L"in");
    if(m_pInputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

} // (Constructor)


//
// Destructor
//
CSocketSendFilter::~CSocketSendFilter()
{
	WSACleanup();
    // Delete the contained interfaces
    ASSERT(m_pInputPin);
	delete(m_pInputPin);
} // (Destructor)

//
// CreateInstance
//
// This goes in the factory template table to create new instances
//
CUnknown * WINAPI CSocketSendFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	WORD wVersionRequested = MAKEWORD(1,1);
	// Initialize WinSock and check the version	
	int nRet = WSAStartup(wVersionRequested, &_wsaData);
	if (_wsaData.wVersion != wVersionRequested){	
		Log("\n Wrong version\n");		
		return NULL;
	}

    return new CSocketSendFilter(pUnk, phr);

} // CreateInstance

STDMETHODIMP CSocketSendFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
	} else
	if (riid == IID_ISocketSendFilter) {
		return GetInterface((ISocketSendFilter*)this, ppv);
	} else
		return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CSocketSendFilter::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

//	*(pPages->pElems) = CLSID_SocketSendFilterPropertyPage;
    return NOERROR;
}

//
// GetPinCount
//
// Return the number of input pins we support
//
int CSocketSendFilter::GetPinCount()
{
    return 1;
} // GetPinCount


//
// GetPin
//
// Return our single input pin - not addrefed
//
CBasePin *CSocketSendFilter::GetPin(int n)
{
    // We only support one input pin and it is numbered zero

    ASSERT(n == 0);
    if(n != 0)
    {
        return NULL;
    }

    return m_pInputPin;

} // GetPin


//
// Stop
//
// Switch the filter into stopped mode.
//
STDMETHODIMP CSocketSendFilter::Stop()
{
    CAutoLock lock(this);

	return CBaseFilter::Stop();
} // Stop


//
// Pause
//
// Override Pause to stop the window streaming
//
STDMETHODIMP CSocketSendFilter::Pause()
{
    CAutoLock lock(this);

    // tell the pin to go inactive and change state
    return CBaseFilter::Pause();

} // Pause


//
// Run
//
// Overriden to start the window streaming
//
STDMETHODIMP CSocketSendFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(this);

	return CBaseFilter::Run(tStart);
} // Run

//
// Constructor
//
CSocketSendInputPin::CSocketSendInputPin(CSocketSendFilter *pFilter,
                               HRESULT *phr,
                               LPCWSTR pPinName) :
    CBaseInputPin(NAME("in"), pFilter, pFilter, phr, pPinName),
	IStream(),
    ISocketSendInputPin()
{
	frame = 0;
    m_pFilter = pFilter;
} // (Constructor)


//
// Destructor does nothing
//
CSocketSendInputPin::~CSocketSendInputPin()
{
} // (Destructor)


STDMETHODIMP CSocketSendInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_ISocketSendInputPin)
	{
		return GetInterface(static_cast<ISocketSendInputPin*>(this), ppv);
	}
	else if(riid == IID_IStream)
	{
		return GetInterface(static_cast<IStream*>(this), ppv);
	}
	else
	{
		return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
	}
}


//
// CheckMediaType
//
// Check that we can support a given proposed type
//
HRESULT CSocketSendInputPin::CheckMediaType(const CMediaType *pmt)
{
    return S_OK; // We like all types :-)
}

//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and allows us to reset the connection media type to be invalid so that
// we can always use that to determine whether we are connected or not. We
// leave the format block alone as it will be reallocated if we get another
// connection or alternatively be deleted if the filter is finally released
//
HRESULT CSocketSendInputPin::BreakConnect()
{
    // Check we have a valid connection
	if(m_mt.IsValid() == FALSE)
    {
        // Don't return an error here, because it could lead to 
        // ASSERT failures when rendering media files in GraphEdit.
        return S_FALSE;
    }

	/// CA - winsock cleanup
	for( std::list<SOCKET>::iterator it = _remoteSocketList.begin(); it != _remoteSocketList.end(); ++it){
		closesocket(*it);
	}
	_remoteSocketList.empty();

    m_pFilter->Stop();

   
	canWrite = false;
	Log("canWrite false\r\n");
	if(flvHeaderSize){Log("delete pFlvHeader\r\n");	 delete[] pFlvHeader; flvHeaderSize=0; }
	if(flvMetaSize){Log("delete pFlvMeta\r\n"); delete[] pFlvMeta; flvMetaSize = 0; }
	if(flvAacSize1){Log("delete pFlvAac1\r\n"); delete[] pFlvAac1; flvAacSize1 = 0; }
	if(grabVPacket){
		Log("delete pFlvVid1\r\n"); 
		for( int i=0; i<grabVPacket; i++){
			delete[] pFlvVid1[i]; 
			flvVidSize1[i] = 0; 
		}
		grabVPacket = 0;
	}	

	// Reset the CLSIDs of the connected media type
    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);
    return CBaseInputPin::BreakConnect();

} // BreakConnect


void ThreadProc(void *param)
{
	Log("\nThreadProc");		

	// Create a TCP/IP stream socket to "listen" with	
	SOCKET	listenSocket;
	listenSocket = socket(AF_INET,			// Address family
						  SOCK_STREAM,		// Socket type
						  IPPROTO_TCP);		// Protocol
	if (listenSocket == INVALID_SOCKET){
		PRINTERROR("socket()");
		//return -1;
		return;
	}

	// Fill in the address structure
	SOCKADDR_IN saServer;		

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
	saServer.sin_port = htons(_PORT);		// TODO: CA - do not hard code the port 
	
	// bind the name to the socket	
	int nRet = bind(listenSocket,	// Socket 
		(LPSOCKADDR)&saServer,	// Our address
		sizeof(struct sockaddr));// Size of address structure
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("bind()");
		closesocket(listenSocket);
		//return nRet;
		return;
	}
	
	// Set the socket to listen
	Log("\nlisten()");
	nRet = listen(listenSocket,	// Bound socket
		SOMAXCONN);	// Number of connection request queue
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("listen()");
		closesocket(listenSocket);
		//return nRet;
		return;
	}
	for(;;){
		_remoteSocket = accept(listenSocket,  // Listening socket
			NULL,	// Optional client address
			NULL);
		if (_remoteSocket == INVALID_SOCKET)
		{
			char err[256];
			sprintf(err, "%d\n",WSAGetLastError());
			//PRINTERROR("accept()");
			closesocket(listenSocket);	
			return;
		}
		Log("\naccept()\r\n");


		while( !_hadHeaders ){
			Sleep(100);
		}

		// CA - this appears to make a big difference on our latency.. further testing is required however.	
		int flag = 1;
		int result = setsockopt(_remoteSocket,        // socket affected 
									 IPPROTO_TCP,     // set option at TCP level 
									 TCP_NODELAY,     // name of option 
									 (char *) &flag,  // the cast is historicalcruft 
									 sizeof(int));    // length of option value 
	
		// (CA) - first thing that we want to do is make sure that if this is a flash socket (in the case of our local player)
		// that we send back a policy file
		int iTimeout = 500;		// only block for .5s
		result = setsockopt( _remoteSocket,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        /*
                        reinterpret_cast<char*>(&tv),
                        sizeof(timeval) );
                        */
                        (const char *)&iTimeout,
                        sizeof(iTimeout) );
		const int BUFFER_SIZE = 256;
		char buf[BUFFER_SIZE] = {'\0'};
		recv(_remoteSocket,buf, BUFFER_SIZE, 0);

		Log("buf: ");
		Log(buf);
		Log("\r\n");
		
		
		int iStillToSend = 0;
		int iSendStatus = 0;

		if( strncmp(buf, "<policy-file-request/>", 22 ) == 0 ){
			// As long we need to send bytes...
			Log("Sending policy file\n");
			char* policy = "<?xml version=""1.0""?><!DOCTYPE cross-domain-policy SYSTEM \"http://adobe.com/xml/dtds/cross-domain-policy.dtd\"><cross-domain-policy><site-control permitted-cross-domain-policies=\"master-only\"/><allow-access-from domain=\"*\" to-ports=\"22820-22830\" /></cross-domain-policy>\0";
			iStillToSend = strlen( policy ) + 1;	// include NULL byte
			while(iStillToSend > 0){
				iSendStatus = send(_remoteSocket, policy, iStillToSend, 0); // Socket is of type
		
				// Error
				if(iSendStatus < 0){
					//return WSAGetLastError();
				}else{
					// Update buffer and counter
					iStillToSend -= iSendStatus;
					policy += iSendStatus;
				}
		
			}
			Log("Policy File sent Closing Socket\n");
			closesocket(_remoteSocket);	// close this socket flash should reconnect now
			continue;	// begin listening again... next request should be the real deal now..
		}else{
			Log("Skipping policy file\n");
		}


		int iRC = 0;
		
		char* pCursor = pFlvHeader;
		iStillToSend = flvHeaderSize;
		

		sprintf(buf, "\nheader (%d)", flvHeaderSize);
		Log(buf);

		// As long we need to send bytes...
		while(iStillToSend > 0){
			iSendStatus = send(_remoteSocket, pCursor, iStillToSend, 0); // Socket is of type
		
			// Error
			if(iSendStatus < 0){
				//return WSAGetLastError();
			}else{
				// Update buffer and counter
				iStillToSend -= iSendStatus;
				pCursor += iSendStatus;
			}
		
		}
	
		iStillToSend = flvMetaSize;
		pCursor = pFlvMeta;
		sprintf(buf, "\nmeta (%d)", flvMetaSize);
		Log(buf);

		while(iStillToSend > 0){
			iSendStatus = send(_remoteSocket, pCursor, iStillToSend, 0); // Socket is of type
		
			// Error
			if(iSendStatus < 0){
				//return WSAGetLastError();
			}else{
				// Update buffer and counter
				iStillToSend -= iSendStatus;
				pCursor += iSendStatus;
			}
		}

		iStillToSend = flvAacSize1;
		pCursor = pFlvAac1;
		sprintf(buf, "\naac (%d)", flvAacSize1);
		Log(buf);
		while(iStillToSend > 0){
			iSendStatus = send(_remoteSocket, pCursor, iStillToSend, 0); // Socket is of type
		
			// Error
			if(iSendStatus < 0){
				//return WSAGetLastError();
			}else{
				// Update buffer and counter
				iStillToSend -= iSendStatus;
				pCursor += iSendStatus;
			}
		}	

		
		for( int i=0; i<grabVPacket; i++){
			// send a bunch of video packets...		
			iStillToSend = flvVidSize1[i];
			pCursor = pFlvVid1[i];
			sprintf(buf, "\nvid (%d)", iStillToSend);
			Log(buf);
			while(iStillToSend > 0){
				iSendStatus = send(_remoteSocket, pCursor, iStillToSend, 0); // Socket is of type		
				// Error
				if(iSendStatus < 0){
					//return WSAGetLastError();
				}else{
					// Update buffer and counter
					iStillToSend -= iSendStatus;
					pCursor += iSendStatus;
				}
			}
		}


		//canWrite = true;
		_remoteSocketList.push_back(_remoteSocket);
		canWrite = true;
	}
    _endthread();

}

//
// Active
//
// Implements the remaining IMemInputPin virtual methods
//
HRESULT CSocketSendInputPin::Active(void)
{
	Log("socket_write.ax::CSocketSendInputPin::Active called.\n");
    

	// CA - TODO: when a stream is stop and started the FLV Mux does not resend the meta data and
	// AAC/VID headers.  So for now I am not going to reset the stream... this is fine as long as
	// to parameters of the stream do not change...
	
//	canWrite = false; 
//	if(flvHeaderSize){ printf("delete header\n"); delete[] pFlvHeader; flvHeaderSize=0; }
//	if(flvMetaSize){ printf("delete meta\n"); delete[] pFlvMeta; flvHeaderSize = 0; }
//	if(flvAacSize1){ printf("delete aac\n"); delete[] pFlvAac1; flvAacSize1 = 0; }
//	if(flvVidSize1){ printf("delete vid\n"); delete[] pFlvVid1; flvVidSize1 = 0; }	

	Log("begin thread\n");
	// Wait for an incoming request	
	 _beginthread(ThreadProc,0, NULL);	
	
    filepos = 0;
    seekMade = false;    

	return S_OK;
} // Active

//
// Inactive
//
// Implements the remaining IMemInputPin virtual methods
//
HRESULT CSocketSendInputPin::Inactive(void)
{
    HRESULT result = NOERROR;

	Log("socket_send.ax::CSocketSendInputPin::Inactive called.\n");
	for( std::list<SOCKET>::iterator it = _remoteSocketList.begin(); it != _remoteSocketList.end(); ++it){
		closesocket(*it);
	}
	_remoteSocketList.empty();
	Log("CLOSE SOCKET\n\n\n");

    return result;
} // Active


HRESULT CSocketSendInputPin::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    // CA - we never should get a read request .. we are a WRITER
	return NOERROR;
}

static int _soc_frame = 0;

// buf: buffer to write out
// size: length of buffer to write out in bytes
// written: number of bytes written
HRESULT CSocketSendInputPin::Write(const void *buf, ULONG size, ULONG *written)
{		
	char *pBuffer = static_cast<char *>( const_cast<void *>(buf) );
	char packetType = pBuffer[4];		// skip 4 byte previoud tag size


//if( packetType != 8 && packetType != 9 ){
//	Log("Unknown packet type\n");
//}


	if( !canWrite ){
		// Header size 9 bytes
		// 4 byte past size value 0)
		// 11 byte meta size
		// ...

		if( !flvHeaderSize ){
			flvHeaderSize = size;
			pFlvHeader = new char[flvHeaderSize];
			memcpy(pFlvHeader, pBuffer, flvHeaderSize);
			*written = size;
			Log("Have header\r\n");
		}else if( !flvMetaSize ){
			flvMetaSize = size;
			pFlvMeta = new char[flvMetaSize];
			memcpy(pFlvMeta, pBuffer, flvMetaSize);
			*written = size;
			Log("Have meta\r\n");
		}else{	
			if( !flvAacSize1 && packetType == 0x8){		
				flvAacSize1 = size;
				pFlvAac1 = new char[flvAacSize1];
				memcpy(pFlvAac1, pBuffer, flvAacSize1);
				*written = size;
				Log("Have aac\r\n");
			} 
			if( !flvVidSize1[grabVPacket] && packetType == 0x9){
				flvVidSize1[grabVPacket] = size;
				pFlvVid1[grabVPacket] = new char[size];
				memcpy(pFlvVid1[grabVPacket], pBuffer, size);
				_soc_frame++;
				*written = size;	
				Log("Have avc ");
				if( grabVPacket < NUM_VID_PACETS-1 ){
					Log(" . \r\n");
					grabVPacket++;
				}else{
					_hadHeaders = true;
				}
			}
		}
		return S_OK;
	}

	for( std::list<SOCKET>::iterator it = _remoteSocketList.begin(); it != _remoteSocketList.end(); ++it){
		pBuffer = static_cast<char *>( const_cast<void *>(buf) );
		int iRC = 0;
		int iSendStatus = 0;
		int iStillToSend = size;

		// As long we need to send bytes...
		while(iStillToSend > 0){		
			// Send some bytes
			iSendStatus = send(*it, pBuffer, iStillToSend, 0); // Socket is of type
		
			// Error
			if(iSendStatus < 0){
				Log("SOCKET SEND ERROR%d\n");
				std::list<SOCKET>::iterator tmp = it;
				it++;
				_remoteSocketList.remove(*tmp);
				if( _remoteSocketList.size() == 0 ){
					return WSAGetLastError();
				}
			}else{
				// Update buffer and counter
				iStillToSend -= iSendStatus;
				pBuffer += iSendStatus;
			}
		}
	}
	*written = size;
	filepos += *written;	
	return S_OK;
}

HRESULT CSocketSendInputPin::Seek(LARGE_INTEGER offset,DWORD origin,ULARGE_INTEGER *c)
{
    if ( offset.QuadPart != 0 )
        seekMade = true;

    // cannot seek on a live stream
//    if ( rtmpConnector.IsLiveStream() )
    {
        c = NULL;
        return E_FAIL;
    }
}

HRESULT CSocketSendInputPin::SetSize(ULARGE_INTEGER a)
{    
	return S_OK;
}

HRESULT CSocketSendInputPin::CopyTo(IStream *a,ULARGE_INTEGER b,ULARGE_INTEGER *c,ULARGE_INTEGER *d)
{
	printf("CSocketSendInputPin::CopyTo not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::Commit(DWORD a)
{
	printf("CSocketSendInputPin::Commit not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::Revert(void)
{
	printf("CSocketSendInputPin::Revert not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::LockRegion(ULARGE_INTEGER a,ULARGE_INTEGER b,DWORD c)
{
	printf("CSocketSendInputPin::LockRegion not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::UnlockRegion(ULARGE_INTEGER a,ULARGE_INTEGER b,DWORD c)
{
	printf("CSocketSendInputPin::UnlockRegion not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::Stat(STATSTG *a,DWORD b)
{
	printf("CSocketSendInputPin::Stat not implemented.\n");
	return NOERROR;
}

HRESULT CSocketSendInputPin::Clone(IStream **a)
{
	printf("CSocketSendInputPin::Clone not implemented.\n");
	return NOERROR;
}


