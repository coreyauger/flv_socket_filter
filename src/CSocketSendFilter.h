//------------------------------------------------------------------------------
//
// File: CVideoRTMPSendFilter.h
//
// Desc: Send video stream data to RTMFP server.
//
//------------------------------------------------------------------------------
#pragma once

interface DECLSPEC_NOVTABLE ISocketSendFilter : public IUnknown
{
//	STDMETHOD(GetRTMPSendInfo)(RTMP_Send_Info *info);
//	STDMETHOD(PutRTMPSendInfo)(const RTMP_Send_Info *info);
};

interface DECLSPEC_NOVTABLE ISocketSendInputPin : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { return -1; }
	virtual ULONG STDMETHODCALLTYPE AddRef() { return -1; }
	virtual ULONG STDMETHODCALLTYPE Release() { return -1; }
};

// Class supporting the VideoRTMPSend input pin



class CSocketSendInputPin : 
    public CBaseInputPin, 
    public IStream,
    public ISocketSendInputPin
    /*, public IMemInputPin,*/
{
    friend class CSocketSendFilter;

public:
	DECLARE_IUNKNOWN;

private:
    

	long formatLength;
	BYTE *format;
	long frame;
	__int64 filepos;

    bool seekMade;

    CSocketSendFilter *m_pFilter;         // The filter that owns us
//    CRTMPConnector rtmpConnector;

public:

    CSocketSendInputPin(CSocketSendFilter *pTextOutFilter,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CSocketSendInputPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    // Lets us know where a connection ends
    HRESULT BreakConnect();

    // Check that we can support this input type
    HRESULT CheckMediaType(const CMediaType *pmt);

	HRESULT Active(void);
	HRESULT Inactive(void);
 
	// IStream
	STDMETHODIMP Read(void *,ULONG,ULONG *);
	STDMETHODIMP Write(const void *,ULONG,ULONG *);
	STDMETHODIMP Seek(LARGE_INTEGER,DWORD,ULARGE_INTEGER *);
	STDMETHODIMP SetSize(ULARGE_INTEGER);
	STDMETHODIMP CopyTo(IStream *,ULARGE_INTEGER,ULARGE_INTEGER *,ULARGE_INTEGER *);
	STDMETHODIMP Commit(DWORD);
	STDMETHODIMP Revert(void);
	STDMETHODIMP LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD);
	STDMETHODIMP UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD);
	STDMETHODIMP Stat(STATSTG *,DWORD);
	STDMETHODIMP Clone(IStream **);

}; // CVideoRTMPSendInputPin


// This is the COM object that represents the VideoRTMPSendFilter

class CSocketSendFilter : 
    public CBaseFilter, 
    public CCritSec, 
    public ISocketSendFilter,
	public ISpecifyPropertyPages
{

public:
    // Implements the IBaseFilter and IMediaFilter interfaces

    // constructors/destructor
    CSocketSendFilter(LPUNKNOWN pUnk,HRESULT *phr);
    virtual ~CSocketSendFilter();

    // This goes in the factory template table to create new instances
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);

    DECLARE_IUNKNOWN

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	// ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

    // CBaseFilter interface

    // Return the pins that we support
    int GetPinCount();
    CBasePin *GetPin(int n);

private:

    // The nested classes may access our private state
    friend class CSocketSendInputPin;

    CSocketSendInputPin *m_pInputPin;   // Handles pin interfaces

    __int16 m_hostPort;

}; // CVideoRTMPSendFilter

