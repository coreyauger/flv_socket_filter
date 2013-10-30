//-----------------------------------------------------------------------------
//
//	AfterCAD RTMP Writer Properties Page
//
//	Author : Kenney Wong
//
//-----------------------------------------------------------------------------
#pragma once


struct ISocketSendFilter;

//-----------------------------------------------------------------------------
//
//	CVideoRTMPSendPropertyPage class
//
//-----------------------------------------------------------------------------
class CSocketSendPropertyPage : public CBasePropertyPage
{
public:

	// rtmp send info
	ISocketSendFilter		*socketSendFilter;

public:
	CSocketSendPropertyPage(LPUNKNOWN lpUnk, HRESULT *phr);
	virtual ~CSocketSendPropertyPage();
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr);

	// CBasePropertyPage
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();

	// message
	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void GetRtmpServerInfo();
    void PutRtmpServerInfo();
	void UpdateStreamInfo();
};


